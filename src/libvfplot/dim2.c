/*
  dim2.c
  vfplot adaptive plot, dimension 2
  J.J.Green 2007
  $Id: dim2.c,v 1.33 2007/10/29 23:48:17 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <vfplot/dim2.h>

#include <vfplot/error.h>
#include <vfplot/evaluate.h>
#include <vfplot/contact.h>
#include <vfplot/lennard.h>
#include <vfplot/rmdup.h>

#include <kdtree.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

/* 
   the maxinimum PW-distance of a dim2 particle
   to a boundary particle, should be less than 1
*/

#define BOUNDARY_NEAR 0.5

#define MAX(a,b) ((a)>(b) ? (a) : (b))
#define MIN(a,b) ((a)<(b) ? (a) : (b))

/* particle system */

#define PARTICLE_FIXED   ((unsigned char) (1 << 0))
#define PARTICLE_STALE   ((unsigned char) (1 << 1))

#define SET_FLAG(flag,val)   ((flag) |= (val))
#define RESET_FLAG(flag,val) ((flag) &= ~(val))
#define GET_FLAG(flag,val)   (((flag) & (val)) != 0)

#define PARTICLE_MASS 3.0

typedef struct
{
  unsigned char flag;
  double mass;
  m2_t M;
  double major;
  vector_t v,dv,F;
} particle_t;

typedef struct
{
  int id;
  double d;
} pw_t;

/* expand p so it fits n1+n2 particles (and put its new size in na) */

static int ensure_alloc(int n1, int n2, particle_t **pp,int *na)
{
  particle_t *p;

  if (n1+n2 <= *na) return 0;

  if ((p = realloc(*pp,(n1+n2)*sizeof(particle_t))) == NULL) return 1;

  *pp = p;
  *na = n1+n2;

  return 0;
} 

static int neighbours(particle_t*,int,int,int**,int*);
static nbs_t* nbs_populate(int,int*,int,particle_t*);

/* compares particles by flag */

static int ptcomp(particle_t *a,particle_t *b)
{
  return 
    GET_FLAG(a->flag,PARTICLE_STALE) - 
    GET_FLAG(b->flag,PARTICLE_STALE);
}

/*
  fade function - we have a light barely-there set
  of boundary ellipses to start, but they revert to
  the regular weight over the fade period
*/

#define FADE_INITIAL 0.05
#define FADE_FINAL   1.00

#define FADE_START   0.50
#define FADE_END     0.80
#define FADE_LEN     (FADE_END - FADE_START)

static double boundary_fade(double a, double b)
{
  double t = a/b;

  if (t<FADE_START) return FADE_INITIAL;
  if (t>FADE_END) return FADE_FINAL;

  double c = (1.0 - cos(M_PI*(t-FADE_START)/FADE_LEN))/2.0;

  return FADE_START + c*FADE_LEN;
}

extern int dim2(dim2_opt_t opt,int* nA,arrow_t** pA,int* nN,nbs_t** pN)
{
  int i;
  vector_t zero = {0.0,0.0};

  /*
    n1 number of dim 0/1 arrows
    n2 number of dim 2 arrows
    na number of arrows allocated
  */

  int n1, n2, na; 

  n2 = 0;
  n1 = na = *nA;

  /* domain dimensions */
  
  double 
    w  = bbox_width(opt.v.bbox),
    h  = bbox_height(opt.v.bbox),
    x0 = opt.v.bbox.x.min,
    y0 = opt.v.bbox.y.min;

  /* 
     the constant C is used to give domain-scale invariant dynamics
  */

  double C = MIN(w,h);

  /*
    estimate number we can fit in, the density of the optimal 
    circle packing is pi/sqrt(12), the area of the ellipse is
    opt.area - then we account for the ones there already

    FIXME - this is too small, eg with electro2
  */

  int 
    no = M_PI*w*h/(sqrt(12)*opt.area), 
    ni = no-n1;

  if (opt.v.verbose)
    printf("  optimal packing estimate %i\n",no);

  ni *= 2;

  if (ni<1)
    {
      fprintf(stderr,"bad dim2 estimate, dim1 %i, dim2 %i\n",n1,ni);
      return ERROR_NODATA;
    }

  /* find the grid size FIXME */

  double R = w/h;
  int    
    nx = (int)floor(sqrt(ni*R)),
    ny = (int)floor(sqrt(ni/R));

  if ((nx<1) || (ny<1))
    {
      fprintf(stderr,"bad initial dim2 grid is %ix%i, strange domain?\n",nx,ny);
      return ERROR_NODATA;
    }

  /* 
     allocate for ni > nx.ny, we will probably be
     adding more arrows later
  */

  particle_t *p = NULL;

  if (ensure_alloc(n1,ni,&p,&na) != 0) return ERROR_MALLOC;

  /* transfer dim 0/1 arrows */

  for (i=0 ; i<n1 ; i++)
    {
      ellipse_t E;

      arrow_ellipse((*pA)+i,&(E));

      p[i].M  = ellipse_mt(E);
      p[i].major = E.major;
      p[i].v  = E.centre;
      p[i].dv = zero;
      p[i].F  = zero;
      p[i].flag = 0;
      SET_FLAG(p[i].flag,PARTICLE_FIXED);
    }

  /* generate an initial dim2 arrowset, on a regular grid */

  double dx = w/(nx+2);
  double dy = h/(ny+2);

  for (i=0 ; i<nx ; i++)
    {
      double x = x0 + (i+1.5)*dx;
      int j;
      
      for (j=0 ; j<ny ; j++)
        {
          double y = y0 + (j+1.5)*dy;
          vector_t v = {x,y};

          if (! domain_inside(v,opt.dom)) continue;

	  arrow_t A; 

          A.centre = v;

          int err = evaluate(&A);

          switch (err)
            {
	      ellipse_t E;

            case ERROR_OK : 
	      arrow_ellipse(&A,&E);
	      p[n1+n2].v = E.centre;
	      p[n1+n2].M = ellipse_mt(E);
	      p[n1+n2].major = E.major;
	      n2++ ; 
	      break;
            case ERROR_NODATA: break;
            default: return err;
            }
        }
    }

  /* initial neighbour mesh */

  int err,nedge=0,*edge=NULL;
	  
  if ((err = neighbours(p,n1,n2,&edge,&nedge)) != ERROR_OK)
    return err;
	  
  if (nedge<2)
    {
      fprintf(stderr,"only %i edges\n",nedge);
      return ERROR_NODATA;
    }

  /* setup dim2 ellipses */

  for (i=n1 ; i<n1+n2 ; i++)
    {
      p[i].dv   = zero;
      p[i].flag = 0;
    }

  /* set the initial mass */

  for (i=1 ; i<n1+n2 ; i++)
    {
      p[i].mass = PARTICLE_MASS;
    }

  /* particle cycle */
  
  if (opt.v.verbose)
    { 
      printf("    n   pt  edge    res  \n");
      printf("   ----------------------\n");
    }
  
  iterations_t iter = opt.v.place.adaptive.iter;

  for (i=0 ; i<iter.main ; i++)
    {
      int j,err;

      for (j=n1 ; j<n1+n2 ; j++)  p[j].flag = 0;

      /* run short euler model */

      double dt = 0.05 * C;
      double sf = 0.0;

      for (j=0 ; j<iter.euler ; j++)
	{
	  int k;

	  if (opt.v.place.adaptive.animate)
	    {
	      int  bufsz = 32;
	      char buf[bufsz];
	      
	      vfp_opt_t v = opt.v;

	      snprintf(buf,bufsz,"anim.%.3i.%.3i.eps",i,j);

	      v.file.output = buf;
	      v.verbose = 0;

	      if (n1+n2 > *nA)
		{
		  arrow_t* A = realloc(*pA,(n1+n2)*sizeof(arrow_t));
		  
		  if (!A) return ERROR_MALLOC;
		  
		  *pA = A;
		}

	      *nA = n1 + n2;
	
	      for (k=n1 ; k<n1+n2 ; k++)
		{
		  (*pA)[k].centre = p[k].v;
		  evaluate((*pA)+k);
		}
	      
	      nbs_t* nbs = nbs_populate(nedge,edge,n1+n2,p);
	      if (!nbs) return ERROR_BUG;

	      if ((err = vfplot_output(opt.dom,*nA,*pA,nedge,nbs,v)) != ERROR_OK)
		{
		  fprintf(stderr,"failed animate write of %i arrows to %s\n",
			  *nA,v.file.output);
		  return err;
		}

	      free(nbs);
	    }

	  /* reset forces */
	  
	  for (k=n1 ; k<n1+n2 ; k++)
	    {
	      p[k].F = zero;
	    }

	  /* accumulate forces */

	  sf = 0.0;

	  for (k=0 ; k<nedge ; k++)
	    {
	      int idA = edge[2*k], idB = edge[2*k+1];
	      vector_t 
		rAB = vsub(p[idB].v, p[idA].v), 
		uAB = vunit(rAB);

	      double x = contact_mt(rAB,p[idA].M,p[idB].M);

	      if (x<0) continue;

	      double d = sqrt(x);
	      double f = lennard(d);
	      
	      f *= p[idA].mass/PARTICLE_MASS;
	      f *= p[idB].mass/PARTICLE_MASS;

	      if (GET_FLAG(p[idA].flag,PARTICLE_FIXED))
		{
		  if (GET_FLAG(p[idB].flag,PARTICLE_FIXED))
		    {
		      /* shouldnt happen */
		    }
		  else
		    {
		      if (d < BOUNDARY_NEAR) 
			SET_FLAG(p[idB].flag,PARTICLE_STALE);
		      p[idB].F = vadd(p[idB].F,smul(f,uAB));
		    }
		}
	      else
		{
		  p[idA].F = vadd(p[idA].F,smul(-f,uAB));

		  if (GET_FLAG(p[idB].flag,PARTICLE_FIXED))
		    {
		      if (d < BOUNDARY_NEAR) 
			SET_FLAG(p[idA].flag,PARTICLE_STALE);
		    }
		  else
		    {
		      p[idB].F = vadd(p[idB].F,smul(f,uAB));
		    }
		}
	      
	      sf += f;
	    }

	  /* Euler step - we can do better than this FIXME */

	  for (k=n1 ; k<n1+n2 ; k++)
	    {
	      double Cd = 4.0;

	      vector_t F = vadd(p[k].F,smul(-Cd,p[k].dv));
	      
	      p[k].dv = vadd(p[k].dv,smul(dt/p[k].mass,F));
	      p[k].v  = vadd(p[k].v,smul(dt,p[k].dv));
	    }

	  /* set the boundary masses */

	  double fade = boundary_fade(i*iter.euler + j,
				      iter.euler*iter.main);

	  for (k=1 ; k<n1 ; k++)
	    {
	      p[k].mass = fade * PARTICLE_MASS;
	    }
	}

      /* mark escapees */

      int nlost = 0;

      for (j=n1 ; j<n1+n2 ; j++)
	{
	  if (! domain_inside(p[j].v,opt.dom))
	    {
	      SET_FLAG(p[j].flag,PARTICLE_STALE);
	      nlost++;
	    }
	}

      /* mark those with overclose neighbours */

      /* re-evaluate */

      for (j=n1 ; j<n1+n2 ; j++) 
	{
	  int err;

	  switch (err = metric_tensor(p[j].v,opt.mt,&(p[j].M)))
	    {
	    case ERROR_OK: break;
	    case ERROR_NODATA: 
	      SET_FLAG(p[j].flag,PARTICLE_STALE);
	      break;
	    default: return err;
	    }
	}

      /* sort to get stale particles at the end */

      qsort(p+n1,n2,sizeof(particle_t),(int (*)(const void*,const void*))ptcomp);

      /* adjust n2 to discard stale particles */

      while (GET_FLAG(p[n1+n2-1].flag,PARTICLE_STALE) && n2) n2--;

      /* create neighbours for the next cycle */

      free(edge); edge = NULL;

      if ((err = neighbours(p,n1,n2,&edge,&nedge)) != ERROR_OK)
	return err;

      /* insertion routine here (ealier version in RCS) */

      if (opt.v.verbose) printf("  %3i %4i %5i %+.6f\n",i,n1+n2,nedge,sf/nedge);
    }

  /* 
     encapulate the network data in array of nbr_t
     for output 
  */

  nbs_t *nbs = nbs_populate(nedge,edge,n1+n2,p); 

  if (!nbs) return ERROR_BUG;

  *nN = nedge;
  *pN = nbs;

  free(edge);

  /* reallocate the output arrow array and transfer data from p */

  if (n1+n2 > *nA)
    {
      arrow_t* A = realloc(*pA,(n1+n2)*sizeof(arrow_t));
      
      if (!A) return ERROR_MALLOC;

      *pA = A;
    }

  for (i=n1 ; i<n1+n2 ; i++)
    {
      (*pA)[i].centre = p[i].v;
      evaluate((*pA)+i);
    }

  *nA = n1+n2;

  free(p);

  return ERROR_OK;
}

/*
  return a nbs array populated from the edge list
*/

static nbs_t* nbs_populate(int nedge, int* edge,int np, particle_t *p)
{
  int i;
  nbs_t *nbs = malloc(nedge*sizeof(nbs_t));

  if (!nbs) return NULL;

  for (i=0 ; i<nedge ; i++)
    {
      int id[2],j;

      for (j=0 ; j<2 ; j++)
	{
	  int idj = edge[2*i+j];

	  if ((idj<0) || (idj>=np))
	    {
	      fprintf(stderr,"edge (%i,%i) id of %i\n",i,j,idj);
	      return NULL;
	    }

	  id[j] = idj;
	}

      nbs[i].a.id = id[0];
      nbs[i].b.id = id[1];

      nbs[i].a.v = p[id[0]].v;
      nbs[i].b.v = p[id[1]].v;
    }

  return nbs;
}

/* function parameters */

#define KD_RNG_INITIAL   4.0
#define KD_EXPAND_MAX    3
#define KD_EXPAND_FACTOR 1.5
#define KD_NBS_MIN       4
#define KD_NBS_MAX       16

typedef struct 
{
  int n[2];
  double d2;
} edged_t;

static int edcmp(const edged_t *ed1, const edged_t *ed2)
{
  return ed1->d2 > ed2->d2;
}

static int ecmp(const int *e1, const int *e2)
{
  int i;

  for (i=0 ; i<2 ; i++)
    {
      if (e1[i] > e2[i]) return  1;
      if (e1[i] < e2[i]) return -1;
    }

  return 0;
}

static int neighbours(particle_t* p, int n1, int n2,int **pe,int *pne)
{
  int i,np=n1+n2,id[np],e[2*np*KD_NBS_MAX],ne=0;
  void *kd = kd_create(2);

  *pe  = NULL;
  *pne = 0;

  for (i=0 ; i<np ; i++) id[i] = i;

  if (!kd) return ERROR_BUG;

  for (i=0 ; i<np ; i++)
    {
      double v[2] = {p[i].v.x,p[i].v.y};

      kd_insert(kd,v,id+i);
    }

  struct {int nx,nn; } stat = {0,0};

  for (i=n1 ; i<np ; i++)
    {
      int j,n;
      double 
	v[2] = {p[i].v.x,p[i].v.y},
	rng  = KD_RNG_INITIAL * p[i].major;
      void* res;

      /* find neighbours */

      if (!(res = kd_nearest_range(kd,v,rng))) return ERROR_BUG;
      n = kd_res_size(res);

      /* expand search radius if required */

      for (j=0 ; (j<KD_EXPAND_MAX) && (n<KD_NBS_MIN) ; j++)
	{
	  kd_res_free(res);
	  rng *= KD_EXPAND_FACTOR; 

	  stat.nx++;

	  if (!(res = kd_nearest_range(kd,v,rng))) return ERROR_BUG;
	  n = kd_res_size(res);
	}

      stat.nn += n;

      /* select nearest if too many */

      if (n>0)
	{	  
	  /* dump results to temporary edge & distance array */

	  int ned=0;
	  edged_t ed[n];

	  while (kd_res_end(res))
	    {
	      int 
		*nid = kd_res_item_data(res),
		j = *nid;

	      kd_res_next(res);
	  
	      if (i == j) continue;

	      ed[ned].n[i>j] = i;
	      ed[ned].n[i<j] = j;

	      ed[ned].d2 = vabs2(vsub(p[i].v,p[j].v));

	      ned++;
	    }
	  
	  /* sort by distance */
 
	  qsort((void*)ed,
		(size_t)ned, 
		sizeof(edged_t),
		(int (*)(const void*, const void*))edcmp);

	  /* transfer at most KD_NBS_MAX to edge struct */

	  ned = MIN(ned,KD_NBS_MAX); 

	  int k;

	  for (k=0 ; k<ned ; k++)
	    {
	      e[2*(ne+k)]   = ed[k].n[0]; 
	      e[2*(ne+k)+1] = ed[k].n[1];
	    }

	  ne += ned;
	}
      else
	{
	  /* 
	     this can happen and means a particle is nowhere
	     near any of the others, often caused by naive
	     initial placement and small ellipse margins.
	     We just ignore it -- it is probably too small to
	     see.
	  */
	}

      kd_res_free(res);
    }

  kd_free(kd);

  if (!ne) return ERROR_NODATA; 

  /* 
     now remove duplicates from e 
  */
 
  qsort((void*)e,
	(size_t)ne, 
	2*sizeof(int),
	(int (*)(const void*, const void*))ecmp);

  ne = rmdup((void*)e,
	     (size_t)ne, 
	     2*sizeof(int),
	     (int (*)(const void*, const void*))ecmp);

  if (!ne) return ERROR_NODATA; 

  /* allocated edge array to return */
  
  size_t aesz = 2*ne*sizeof(int);
  
  int *ae = malloc(aesz);
  
  if (!ae) return ERROR_MALLOC;
  
  memcpy(ae,e,aesz);
  
  *pe  = ae;
  *pne = ne;
  
  return ERROR_OK;
}

