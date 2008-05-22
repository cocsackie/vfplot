/*
  paths.h 
  structures for boundary paths of arrows

  J.J.Green 2008
  $Id: paths.c,v 1.3 2008/05/21 22:08:18 jjg Exp jjg $
*/

#include <math.h>

#include <vfplot/paths.h>
#include <vfplot/graph.h>
#include <vfplot/contact.h>
#include <vfplot/macros.h>

static int path_count(gstack_t** path,int* n)
{
  *n += gstack_size(*path);
  return 1;
}

extern int paths_count(gstack_t* paths)
{
  int n=0;
  gstack_foreach(paths,(int (*)(void*,void*))path_count,(void*)(&n));
  return n;  
}

extern int paths_serialise(gstack_t* paths,int* nA,arrow_t** pA)
{
  int n = paths_count(paths);

  if (n>0)
    {
      arrow_t *A = malloc(n*sizeof(arrow_t));
      
      *pA = A;

      gstack_t *path;

      if (!A) return 1;

      while (gstack_pop(paths,&path) == 0)
	{
	  corner_t corner; 

	  while (gstack_pop(path,&corner) == 0)
	    {
	      *A = corner.A;
	      A++;
	    }
	}
    }

  *nA = n;

  return 0;
}

/*
  delete arrows from pathset until there are no intersections,
  here an "intersection" means the contact distance is less than
  Dmin as specified by the user (1.0 by default)
*/

static int path_decimate(gstack_t**,double*);

extern int paths_decimate(gstack_t* paths,double d)
{
  return gstack_foreach(paths,(int(*)(void*,void*))path_decimate,&d);
}

/* 
   gstack iterator, acts on a single path

   - dump the corners into the cns array 

   - create graph of ellipse intersections with nodes
     weighted by the maximum length of their edges.
     thus boundary corner nodes are precious 

   - totally disconnect the graph in a greedy manner
     aimng to keep the weight high

   - push the non-deleted node ellipses back onto 
     the stack

  here "intersect" means the pw-distance D is less 
  than Dmin, and since D = sqrt(x), x = contact(),
  it is the same to check x < xmin = Dmin^2
*/

static int path_decimate(gstack_t** path, double* Dmin)
{
  int i,n = gstack_size(*path);
  double xmin = pow(*Dmin,2);
  
  corner_t cns[n];

  /* 
     empty the stack into a corners array - we do this
     in reverse order so the gstack_push below gives us
     a gstack_t in the same order (needed due to an 
     assumed orientation of the boundaries)
  */

  for (i=0 ; i<n ; i++) gstack_pop(*path,(void*)(cns+(n-1-i)));

  /* cache metric tensor and ellipse centres */

  vector_t e[n];
  m2_t mt[n];

  for (i=0 ; i<n ; i++)
    {
      ellipse_t E;

      arrow_ellipse(&(cns[i].A),&E);

      mt[i] = ellipse_mt(E);
      e[i]  = E.centre;
    }

  /* create ellipse intersection graph */

  graph_t G;

  if (graph_init(n,&G) != 0) return -1; 

  size_t err = 0;

  for (i=0 ; i<n-1 ; i++)
    {
      int j;

      for (j=i+1 ; j<n ; j++)
	{
	  double x = contact_mt(vsub(e[j],e[i]),mt[i],mt[j]);

	  /* 
	     failed contact() results in edge not being
	     included in the graph so possible intesection
	     survives - save the number for a warning (below)
	  */

	  if (x<0)
	    {
	      err++;
	      continue;
	    }

	  /* 
	     weight of each node is the maximum of the
	     contact-distances of the incoming edges
	  */

	  if (x < xmin)
	    {
	      double 
		w1 = graph_get_weight(G,i),
		w2 = graph_get_weight(G,j);

	      graph_set_weight(G,i,MAX(w1,x));
	      graph_set_weight(G,j,MAX(w2,x));

	      if (graph_add_edge(G,i,j) != 0) return -1;
	    }
	}
    }

  if (err)
    fprintf(stderr,"failed contact distance for %i pairs\n",err);

  /* greedy node deletion to obtain non-intersecting subset */

  size_t maxi;

  while (graph_maxedge(G,&maxi) > 0)
    if (graph_del_node(G,maxi) != 0) return -1;

  /* dump back into gstack */

  for (i=0 ; i<n ; i++)
    if ( !graph_node_flag(G,i,NODE_STALE) )
      gstack_push(*path,(void*)(cns+i));

  graph_clean(&G);

  return 1;
}


