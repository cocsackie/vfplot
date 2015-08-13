/*
  pwgrid.c
  Copyright (c) J.J. Green 2015
*/

#include <stdio.h>
#include <math.h>

#include <vfplot/contact.h>

#include "pwgrid.h"

static int pwgrid_stream(FILE *st, pwgrid_opt_t opt)
{
  size_t n = opt.n;
  ellipse_t E0 = {
    .major = opt.ellipse[0].major,
    .minor = opt.ellipse[0].minor,
    .theta = 0,
    .centre = VEC(0, 0)
  };
  ellipse_t E1 = {
    .major = opt.ellipse[1].major,
    .minor = opt.ellipse[1].minor,
    .theta = 0
  };

  double
    xmax = 2.0,
    ymax = 2.0;

  for (size_t i = 0 ; i < n ; i++)
    {
      double x = i * xmax /(n - 1);

      X(E1.centre) = x;

      for (size_t j = 0 ; j < n ; j++)
	{
	  double y = j * ymax /(n - 1);
	  Y(E1.centre) = y;

	  double d = contact(E0, E1);

	  if (d >= 0.0)
	    fprintf(st, "%.6f %.6f %.6f\n", x, y, sqrt(d));
	}
    }

  return 0;
}

extern int pwgrid(pwgrid_opt_t opt)
{
  int err;

  if (opt.file)
    {
      FILE *st = fopen(opt.file, "w");
      err = pwgrid_stream(st, opt);
      fclose(st);
    }
  else
    err = pwgrid_stream(stdout, opt);

  return err;
}
