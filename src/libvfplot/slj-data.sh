#! /bin/sh

# truncated lennard-jones potential for a 1-dimentional 
# 4-point syetem with one point fixed at 0, one at 1.

T=0.5
C=cpt-city/jjg/subtle

GRD=slj-data.grd
CPT=slj-data.cpt
EPS=slj-data.eps
DEF=slj-data.def
RNG=-R0/1/0/1
PRJ=-JX5i

for ((n=5; n <= 99 ; n++))
  do

  t=`dc -e "$n 0.01 * n"`
  ft=`printf "%.3f" $t`
  FRM="slj-data-$ft.eps"

  echo -n "$FRM .."

  ./slj-data $t | GMT xyz2grd -I256+/256+ $RNG -G$GRD
  GMT grd2cpt $GRD -C$C -Z > $CPT
  GMT grdimage +$DEF $GRD -C$CPT $RNG $PRJ > $FRM
  GMT ps2raster -A -E35 -Tg $FRM
  rm $FRM $GRD $CPT

  echo ". done"

done


