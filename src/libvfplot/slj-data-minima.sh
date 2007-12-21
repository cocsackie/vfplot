#! /bin/sh

# truncated lennard-jones potential for a 1-dimentional 
# 4-point syetem with one point fixed at 0, one at 1.

# quick gmt determination of the location of a minimum

GRD=slj-data.grd
RNG=-R0/1/0/1
PRJ=-JX5i

for ((n=0; n <= 24 ; n++))
  do

  t=`dc -e "$n 0.01 * n"`
  ft=`printf "%.3f" $t`
  FRM="slj-data-$ft.eps"

  echo -n "$ft "
  ./slj-data $t | GMT xyz2grd -I256+/256+ $RNG -G$GRD
  GMT grdinfo -M $GRD | grep "z_min" | cut -d' ' -f 7,10

done


