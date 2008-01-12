/*
  dim2.c
  vfplot adaptive plot, dimension 2
  J.J.Green 2007
  $Id: dim2.c,v 1.42 2008/01/10 00:30:45 jjg Exp jjg $
*/

#define _ISOC99_SOURCE

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <vfplot/dim2.h>

#include <vfplot/constants.h>
#include <vfplot/error.h>
#include <vfplot/evaluate.h>
#include <vfplot/contact.h>
#include <vfplot/slj.h>
#include <vfplot/rmdup.h>
#include <vfplot/flag.h>
#include <vfplot/macros.h>

#include <kdtree.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

/*
  the schedule defines a series of parameters
  varied over the course of the dynamics run
  (essentially an annealing schedule) and applied
  over a subset of the particles. 

  charge : the charge is multipled by factor
  mass   : likewise
  rt     : potential truncation radius (from slj.h)
  rd     : deletion radius
  dmax   : maximum number deleted 
*/

typedef struct
{
  double charge,mass,rd,rt;
  size_t dmax;
} schedule_t;

/* particle system */

#define PARTICLE_FIXED  FLAG(0)
#define PARTICLE_STALE  FLAG(1)

#define PARTICLE_MASS 3.0

typedef struct
{
  unsigned char flag;
  double charge,mass;
  m2_t M;
  double major;
  vector_t v,dv,F;
} particle_t;

/* 
   use pthreads for force accumulation 

   on small systems and a single processor
   this leads to a fourfold increase in 
   execution time, so a lot of overhed and
   probably only useful in large problems
*/

#define PTHREAD_FORCES

#ifdef PTHREAD_FORCES

#include <pthread.h>

#define NTHREAD 2

typedef struct
{
  int *edge;
  particle_t *p;
  double rd;
  pthread_mutex_t *mutex;
} tglobal_t;

typedef struct
{
  size_t off,size;
  tglobal_t *glob;
} tdata_t; 

static void subdivide(size_t,size_t,size_t*,size_t*);
static void* force_thread(tdata_t*);

#endif

/* temporary pw-distance struct */

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

/* compare pw_ts by d */

static int pwcomp(pw_t *a,pw_t *b)
{
  return a->d > b->d;
}

/*
  for t in [0,1], defines a spline which is constant z0 in [0,t0],
  constant z1 in [t1,1], and a sinusoid inbetween.
*/

static double sinspline(double t, double t0, double z0, double t1, double z1)
{
  if (t<t0) return z0;
  if (t>t1) return z1;

  return z0 + (z1-z0)*(1.0 - cos(M_PI*(t-t0)/(t1-t0)))/2.0;
}

/*
  phases of the schedule
  - start, for charge buildup
  - contain, initial period of extra containment
  - clean, delete overlappers
  - detrunc, de-truncate the potential
*/

#define START_T0 0.0
#define START_T1 0.1

#define CONTAIN_T0 0.0
#define CONTAIN_T1 0.6

#define CLEAN_T0 0.4
#define CLEAN_T1 0.7

#define CLEAN_RADIUS 0.4
#define CLEAN_DELMAX 32

#define DETRUNC_T0 0.6
#define DETRUNC_T1 0.8
#define DETRUNC_RADIUS 0.90

static void boundary_schedule(double t,schedule_t* s)
{
  s->mass   = 1.0;
  s->charge = sinspline(t,CONTAIN_T0,2.0,CONTAIN_T1,1.0);
  s->rd     = 0.0;
  s->rt     = 0.0;
  s->dmax   = 0;
}

static void interior_schedule(double t,schedule_t* s)
{
  s->mass   = 1.0;
  s->charge = sinspline(t, START_T0, 0.0, START_T1, 1.0);
  s->rd     = sinspline(t, CLEAN_T0, 0.0, CLEAN_T1, CLEAN_RADIUS);
  s->rt     = sinspline(t, DETRUNC_T0, DETRUNC_RADIUS,DETRUNC_T1,0.0);
  s->dmax   = sinspline(t, CLEAN_T0, 0.0, CLEAN_T1, CLEAN_DELMAX);
}

static void schedule(double t, schedule_t* sB,schedule_t* sI)
{
  boundary_schedule(t,sB);
  interior_schedule(t,sI);
}

/*
  set mass & charge on a particle p, then mutiplied 
  by C
*/

static void set_mass(particle_t* p, double C)
{
  p->mass = PARTICLE_MASS * C;
}

extern int dim2(dim2_opt_t opt,int* nA,arrow_t** pA,int* nN,nbs_t** pN)
{
  int i,err;
  vector_t zero = {0.0,0.0};
 
  /* initialise schedules */

  schedule_t schedB,schedI;

  schedule(0.0,&schedB,&schedI);

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
    printf("optimal packing estimate %i\n",no);

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

  /* generate an initial dim2 arrowset on a regular grid */

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

          err = evaluate(&A);

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

  int nedge=0,*edge=NULL;
	  
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

  /* set the initial physics */

  for (i=1 ; i<n1 ; i++)
    {
      set_mass(p+i,schedB.mass);
    }

  for (i=n1 ; i<n1+n2 ; i++)
    {
      set_mass(p+i,schedI.mass);
    }

  /* initialise shifted lennard-jones */

  slj_init(1.0, 0.1, 2.0);

  /* particle cycle */
  
  if (opt.v.verbose)
    { 
      printf("  n   pt esc ocl  edge       res\n");
      printf("--------------------------------\n");
    }
  
  iterations_t iter = opt.v.place.adaptive.iter;

  for (i=0 ; i<iter.main ; i++)
    {
      int j;

      for (j=n1 ; j<n1+n2 ; j++)  p[j].flag = 0;

      /* run short euler model */

      double dt = 0.05 * C;
      double sf = 0.0;

      for (j=0 ; j<iter.euler ; j++)
	{
	  int k;
	  double T = ((double)(i*iter.euler + j))/((double)(iter.euler*iter.main));

	  schedule(T,&schedB,&schedI);

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
	  
	  for (k=n1 ; k<n1+n2 ; k++)  p[k].F = zero;

	  /* accumulate forces */

	  sf = 0.0;

#ifdef PTHREAD_FORCES

          size_t  eoff[NTHREAD];
	  size_t  esz[NTHREAD];

	  subdivide(NTHREAD,nedge,eoff,esz);

	  pthread_mutex_t mutex;

	  pthread_mutex_init(&mutex, NULL);

	  tglobal_t tglob;
	  tdata_t tdata[NTHREAD];

	  tglob.edge  = edge;
	  tglob.p     = p;
	  tglob.rd    = schedI.rd;
	  tglob.mutex = &mutex;


	  for (k=0 ; k<NTHREAD ; k++)
	    {
	      tdata[k].off   = eoff[k];
	      tdata[k].size  = esz[k];
	      tdata[k].glob  = &tglob;
	    }

	  pthread_attr_t attr;

	  pthread_attr_init(&attr);
	  pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

	  pthread_t thread[NTHREAD];

	  for (k=0 ; k<NTHREAD ; k++)
	    pthread_create(thread+k,
			   &attr,
			   (void* (*)(void*))force_thread,
			   (void *)(tdata+k));

	  pthread_attr_destroy(&attr);

	  for (k=0 ; k<NTHREAD ; k++) pthread_join(thread[k],NULL);

	  pthread_mutex_destroy(&mutex);

#else

	  for (k=0 ; k<nedge ; k++)
	    {
	      int idA = edge[2*k], idB = edge[2*k+1];
	      vector_t 
		rAB = vsub(p[idB].v, p[idA].v), 
		uAB = vunit(rAB);

	      double x = contact_mt(rAB,p[idA].M,p[idB].M);

	      if (x<0) continue;

	      double d = sqrt(x);
	      double f = -sljd(d);
	      
	      /* horrible hack! FIXME */

	      if (f > 1.0) f = 1.0;

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
		      if (d < schedI.rd) 
			SET_FLAG(p[idB].flag,PARTICLE_STALE);
		      p[idB].F = vadd(p[idB].F,smul(f,uAB));
		    }
		}
	      else
		{
		  p[idA].F = vadd(p[idA].F,smul(-f,uAB));

		  if (GET_FLAG(p[idB].flag,PARTICLE_FIXED))
		    {
		      if (d < schedI.rd) 
			SET_FLAG(p[idA].flag,PARTICLE_STALE);
		    }
		  else
		    {
		      p[idB].F = vadd(p[idB].F,smul(f,uAB));
		    }
		}
	      
	      sf += f;
	    }

#endif

	  /* Euler step - we can do better than this FIXME */

	  for (k=n1 ; k<n1+n2 ; k++)
	    {
	      double Cd = 4.0;

	      vector_t F = vadd(p[k].F,smul(-Cd,p[k].dv));
	      
	      p[k].dv = vadd(p[k].dv,smul(dt/p[k].mass,F));
	      p[k].v  = vadd(p[k].v,smul(dt,p[k].dv));
	    }

	  /* set the boundary masses */

	  for (k=1 ; k<n1 ; k++)
	    {
	      set_mass(p+k,schedB.mass);
	    }
	  
	  for (k=n1 ; k<n1+n2 ; k++)
	    {
	      set_mass(p+k,schedI.mass);
	    }
	}

      /* mark escapees */

      int nesc = 0;

      for (j=n1 ; j<n1+n2 ; j++)
	{
	  if (! domain_inside(p[j].v,opt.dom))
	    {
	      SET_FLAG(p[j].flag,PARTICLE_STALE);
	      nesc++;
	    }
	}

      /* 
	 mark those with overclose neighbours, here we 
	 - for each internal particle find the minimal 
	   pw-distance from amongst its neighbours
	 - sort by this value and take the top 2n
	 - create the intersection graph from this 2n
	   and ensure there are no intersections
	 - take the top n of the remander ad mark those
	   as stale
      */

      int nocl = 0;

      if ((n2>0) && (schedI.dmax>0) && (schedI.rd>0.0))
	{
          pw_t pw[n2];

          for (j=0 ; j<n2 ; j++)
            {
              pw[j].d = INFINITY;
	      pw[j].id = j+n1;
            }

          for (j=0 ; j<nedge ; j++)
            {
              int id[2] = {edge[2*j],edge[2*j+1]};

              vector_t rAB = vsub(p[id[1]].v, p[id[0]].v);
              double x = contact_mt(rAB,p[id[0]].M,p[id[1]].M);

              if (x<0) continue;

              double d = sqrt(x);
              int k;

	      /*
		only going to k=1 means the minimum is attached
		to the smaller id 
	      */

              for (k=0 ; k<1 ; k++)
                {
		  size_t idk = id[k]-n1;
		  double d1 = pw[idk].d;

                  pw[idk].d = MIN(d,d1);
                }
            }
	  
	  qsort(pw,n2,sizeof(pw_t),(int(*)(const void*,const void*))pwcomp);

	  double r;

          for (j=0,r=pw[0].d ; 
	       (r<schedI.rd) && (nocl<schedI.dmax) && (j<n2) ; 
	       r=pw[++j].d)
            {
	      SET_FLAG(p[pw[j].id].flag,PARTICLE_STALE);
	      nocl++;
            }
	} 

      /* re-evaluate */

      for (j=n1 ; j<n1+n2 ; j++) 
	{
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

      if (opt.v.verbose) 
	printf("%3i %4i %3i %3i %5i %+.6f\n",i,n1+n2,nesc,nocl,nedge,sf/nedge);
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
	      int *nid = kd_res_item_data(res);

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

#ifdef PTHREAD_FORCES

/*
  subdivide a range 0..ne into nt subranges specified
  by offset and size. eg 0..20 by 2 -> 0..10, 11..20
*/

static void subdivide(size_t nt,size_t ne,size_t* off,size_t* size)
{
  size_t m = ne/nt;
  int i;

  for (i=0 ; i<nt-1 ; i++)
    {
      off[i] = i*m;
      size[i] = m;
    }

  off[nt-1] = (nt-1)*m;
  size[nt-1] = ne - (nt-1)*m;
}

/*
  here we have mutex lock for every summand - we could 
  probably make local force accumulations and then write 
  those to the t->p in one fell swoop at the end (needing 
  only a single lock) 
*/

static void* force_thread(tdata_t* pt)
{
  int i;
  tdata_t t = *pt;
  tglobal_t g = *(t.glob);

  for (i=0 ; i<t.size ; i++)
    {
      int k = i+t.off;
      int idA = g.edge[2*k], idB = g.edge[2*k+1];
      vector_t 
	rAB = vsub(g.p[idB].v, g.p[idA].v), 
	uAB = vunit(rAB);
      
      double x = contact_mt(rAB, g.p[idA].M, g.p[idB].M);

      if (x<0) continue;

      double d = sqrt(x);
      double f = -sljd(d);
      
      /* horrible hack! FIXME */
      
      if (f > 1.0) f = 1.0;
      
      f *= g.p[idA].mass/PARTICLE_MASS;
      f *= g.p[idB].mass/PARTICLE_MASS;

      pthread_mutex_lock(g.mutex);
      
      if (GET_FLAG(g.p[idA].flag,PARTICLE_FIXED))
	{
	  if (GET_FLAG(g.p[idB].flag,PARTICLE_FIXED))
	    {
	      /* shouldnt happen */
	    }
	  else
	    {
	      if (d < g.rd) 
		SET_FLAG(g.p[idB].flag,PARTICLE_STALE);
	      g.p[idB].F = vadd(g.p[idB].F,smul(f,uAB));
	    }
	}
      else
	{
	  g.p[idA].F = vadd(g.p[idA].F,smul(-f,uAB));
	  
	  if (GET_FLAG(g.p[idB].flag,PARTICLE_FIXED))
	    {
	      if (d < g.rd) 
		SET_FLAG(g.p[idA].flag,PARTICLE_STALE);
	    }
	  else
	    {
	      g.p[idB].F = vadd(g.p[idB].F,smul(f,uAB));
	    }
	}
      
      pthread_mutex_unlock(g.mutex);
    }

  return NULL;
}

#endif
