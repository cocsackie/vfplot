/*
  dim2.c
  vfplot adaptive plot, dimension 2
  J.J.Green 2007
  $Id: dim2.c,v 1.3 2007/07/22 22:18:11 jjg Exp jjg $
*/

#include <math.h>
#include <stdlib.h>

#include <vfplot/dim2.h>

#include <vfplot/error.h>
#include <vfplot/evaluate.h>

#ifdef TRIANGLE

#define REAL double
#include <triangle.h> 
typedef struct triangulateio triang_t;

#else

#include <vfdela.h> 

#endif

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

extern int dim2(dim2_opt_t opt,int* pn,arrow_t** pA)
{
  /*
    n1 number of dim 0/1 arrows
    n2 number of dim 2 arrows
    na number of arrows allocated
  */

  int n1, n2, na, err; 

  n2 = 0;
  n1 = na = *pn;

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

  int nedge,*edge;

  if ((err = neighbours(*pA,n1,n2,&edge,&nedge)) != ERROR_OK)
    return err;

  for (i=0 ; i<nedge ; i++)
    printf("%i %i\n",
	   edge[2*i],
	   edge[2*i+1]);

  /* run force model */

  (*pn) += n2;

  free(edge);

  return ERROR_OK;
}

#ifdef TRIANGLE

/*
  find the triangulation's neighbour-pairs using Shewchuck's 
  Triangle
*/

static int neighbours(arrow_t* A, int n1, int n2,int **e,int *ne)
{
  triang_t ti,to;

  ti.numberofpoints = n1+n2;

  if ((ti.pointlist = malloc(2*ti.numberofpoints*sizeof(double))) == NULL)
    return ERROR_MALLOC;

  int i;

  for (i=0 ; i<n1+n2 ; i++)
    {
      ti.pointlist[2*i]   = A[i].centre.x;
      ti.pointlist[2*i+1] = A[i].centre.y;
    }

  ti.numberofpointattributes = 0;
  ti.numberofsegments = 0;
  ti.numberofholes    = 0;
  ti.numberofregions  = 0;

  to.edgelist = NULL;

  /*
    Q/V - quiet or verbose
    z   - number from zero
    e   - edges
    N   - no points
    B   - no boundary
  */

  triangulate("VzeNB",&ti,&to,NULL);

  *ne = to.numberofedges;
  *e  = to.edgelist;

  free(ti.pointlist);

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
