/*
  dim2.c
  vfplot adaptive plot, dimension 2
  J.J.Green 2007
  $Id: dim2.c,v 1.8 2007/08/07 22:35:35 jjg Exp jjg $
*/

#include <math.h>
#include <stdlib.h>

#include <vfplot/dim2.h>

#include <vfplot/error.h>
#include <vfplot/evaluate.h>
#include <vfplot/contact.h>
#include <vfplot/lennard.h>

#ifdef TRIANGLE

#define REAL double
#include <triangle.h> 
typedef struct triangulateio triang_t;

#else

#include <vfdela.h> 

#endif

/* particle system */

#define PARTICLE_FIXED  ((unsigned char) (1 << 0))
#define PARTICLE_LOST   ((unsigned char) (1 << 1))

#define SET_FLAG(flag,val)   ((flag) |= (val))
#define RESET_FLAG(flag,val) ((flag) &= ~(val))
#define GET_FLAG(flag,val)   (((flag) & (val)) != 0)

typedef struct
{
  unsigned char flag;
  unsigned int aid;
  m2_t M;
  vector_t v,dv,F;
} particle_t;

/* expand pA so it fits n1+n2 arrows (and put its new size in na) */

static int ensure_alloc(int n1, int n2, arrow_t **pA,int *na)
{
  arrow_t *p;

  if (n1+n2 <= *na) return 0;

  if ((p = realloc(*pA,(n1+n2)*sizeof(arrow_t))) == NULL) return 1;

  *pA = p;
  *na = n1+n2;

  return 0;
} 

static int neighbours(arrow_t*,int,int,int**,int*);

extern int dim2(dim2_opt_t opt,int* nA,arrow_t** pA,int* nN,nbs_t** pN)
{
  /*
    n1 number of dim 0/1 arrows
    n2 number of dim 2 arrows
    na number of arrows allocated
  */

  int n1, n2, na, err; 

  n2 = 0;
  n1 = na = *nA;

  /*
    estimate number we can fit in, the density of the optimal 
    circle packing is pi/sqrt(12), the area of the ellipse is
    pi.a.b - then we account for the ones there already
  */

  int 
    no = bbox_volume(opt.bb)/(sqrt(12)*opt.me.major*opt.me.minor),
    ni = no-n1;

  if (ni<1)
    {
      fprintf(stderr,"bad dim2 estimate, dim1 %i, dim2 %i\n",n1,ni);
      return ERROR_NODATA;
    }
  
  double 
    w  = bbox_width(opt.bb),
    h  = bbox_height(opt.bb),
    x0 = opt.bb.x.min,
    y0 = opt.bb.y.min;

  /* find the grid size */

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

  if (ensure_alloc(n1,ni,pA,&na) != 0) return ERROR_MALLOC;

  /* generate an initial dim2 arrowset, on a regular grid */

  arrow_t* A2 = (*pA) + n1;

  int i;
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

	  arrow_t *A = A2 + n2;

          A->centre = v;

          int err = evaluate(A);

          switch (err)
            {
            case ERROR_OK : n2++ ; break;
            case ERROR_NODATA: break;
            default: return err;
            }
        }
    }

  /* now the main iteration */

  /* setup force model */

  particle_t *p = malloc((n1+n2)*sizeof(particle_t));

  if (!p) return ERROR_MALLOC;

  /* setup dim0/1 ellipses */

  for (i=0 ; i<n1 ; i++)
    {
      ellipse_t E;

      arrow_ellipse((*pA)+i,&E);

      p[i].M = ellipse_mt(E);

      p[i].v = E.centre;

      p[i].dv.x = 0.0;
      p[i].dv.y = 0.0;

      p[i].flag = 0;
    }

  for (i=0 ; i<n1 ; i++) SET_FLAG(p[i].flag,PARTICLE_FIXED);

  int nedge=0,*edge=NULL;

  /* particle cycle */

  int nmain = opt.iter.main;

  for (i=0 ; i<nmain ; i++)
    {
      int j;

      /* 
	 except for the first cycle this releases the 
	 storage for edges and lets neighbours() allocate
	 it (it knows how big it is), so edge is still
	 held by the end of this iteration
      */

      if (edge)
	{
	  free(edge);
	  edge = NULL;
	}

      /* find neigbours */

      if ((err = neighbours(*pA,n1,n2,&edge,&nedge)) != ERROR_OK)
	return err;

      if (nedge<2)
	{
	  fprintf(stderr,"only %i edges\n",nedge);
	  return ERROR_NODATA;
	}

      /* setup dim2 ellipses */

      for (j=n1 ; j<n1+n2 ; j++)
	{
	  ellipse_t E;
	  
	  arrow_ellipse((*pA)+j,&E);
	  
	  p[j].M = ellipse_mt(E);
	  
	  p[j].v  = E.centre;
	  
	  p[j].dv.x = 0.0;
	  p[j].dv.y = 0.0;

	  p[j].flag = 0;
	}

      /* run short euler model */

      double dt  = 0.1;
      double sse;

      for (j=0 ; j<10 ; j++)
	{
	  int k;
	  
	  /* reset forces */
	  
	  for (k=n1 ; k<n1+n2 ; k++)
	    {
	      p[k].F.x = 0.0;
	      p[k].F.y = 0.0;
	    }

	  /* accumulate forces */

	  sse = 0.0;

	  for (k=0 ; k<nedge ; k++)
	    {
	      int idA = edge[2*k], idB = edge[2*k+1];
	      vector_t rAB = vsub(p[idB].v, p[idA].v), uAB = vunit(rAB);

	      double x = contact_mt(rAB,p[idA].M,p[idB].M);

	      if (x<0) continue;

	      double d = sqrt(x);
	      double f = lennard(d);
	      
	      if (! GET_FLAG(p[idA].flag,PARTICLE_FIXED))
		p[idA].F = vadd(p[idA].F,smul(-f,uAB));
	      
	      if (! GET_FLAG(p[idB].flag,PARTICLE_FIXED))
		p[idB].F = vadd(p[idB].F,smul(f,uAB));
	      
	      sse += (d-1)*(d-1);
	    }

	  /* Euler step */

	  for (k=n1 ; k<n1+n2 ; k++)
	    {
	      double M  = 10;
	      double Cd = 1.0;

	      vector_t F = vadd(p[k].F,smul(-Cd,p[k].dv));
	      
	      p[k].dv = vadd(p[k].dv,smul(dt/M,F));
	      p[k].v  = vadd(p[k].v,smul(dt,p[k].dv));
	    }      
	}

      printf("%i %i %f\n",i,nedge,sqrt(sse/nedge));

      /* reevaluate */

      for (j=n1 ; j<n1+n2 ; j++) 
	{
	  (*pA)[j].centre = p[j].v;
	  evaluate((*pA)+j);
	}

      /* kill escapees */
    }

  free(p);

  /* 
     encapulate the network data in array of nbr_t
     for output 
  */

  nbs_t *nbs = malloc(nedge*sizeof(nbs_t));

  if (!nbs) return ERROR_MALLOC;

  for (i=0 ; i<nedge ; i++)
    {
      int id[2],j;

      for (j=0 ; j<2 ; j++)
	{
	  int idj = edge[2*i+j];

	  if ((idj<0) || (idj>=n1+n2))
	    {
	      fprintf(stderr,"edge (%i,%i) id of %i\n",i,j,idj);
	      return ERROR_BUG;
	    }

	  id[j] = idj;
	}

      nbs[i].a.id = id[0];
      nbs[i].b.id = id[1];

      nbs[i].a.v = (*pA)[id[0]].centre;
      nbs[i].b.v = (*pA)[id[1]].centre;
    }

  *nN = nedge;
  *pN = nbs;

  free(edge);

  /* record number of arrow for output */

  (*nA) += n2;

  return ERROR_OK;
}

#ifdef TRIANGLE

/*
  find the triangulation's neighbour-pairs using Shewchuck's 
  Triangle
*/

static int neighbours(arrow_t* A, int n1, int n2,int **e,int *ne)
{
  triang_t ti = {0}, to = {0};

  ti.numberofpoints = n1+n2;

  if ((ti.pointlist = malloc(2*ti.numberofpoints*sizeof(double))) == NULL)
    return ERROR_MALLOC;

  int i;

  for (i=0 ; i<n1+n2 ; i++)
    {
      ti.pointlist[2*i]   = A[i].centre.x;
      ti.pointlist[2*i+1] = A[i].centre.y;
    }

  /*
    Q/V - quiet or verbose
    z   - number from zero
    e   - edges
    E   - no triangles
    N   - no points
    B   - no boundary
  */

  triangulate("QzeENB",&ti,&to,NULL);

  free(ti.pointlist);

  *ne = to.numberofedges;
  *e  = to.edgelist;

  return ERROR_OK;
}

#else

/*
  find neighbours using internal triangulator
*/

static int neighbours(arrow_t* A, int n1, int n2,int **e,int *ne)
{
  fprintf(stderr,"not implemented yet\n");
  return ERROR_BUG;
}

#endif
