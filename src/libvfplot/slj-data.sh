#! /bin/sh

# truncated lennard-jones potential for a 1-dimentional 
# 4-point syetem with one point fixed at 0, one at 1.

T=0.2
Z=-0.2/0/0.01
C=cpt-city/jjg/subtle

GRD=slj-data.grd
CPT=slj-data.cpt
EPS=slj-data.eps
DEF=slj-data.def
RNG=-R0/1/0/1
PRJ=-JX5i

./slj-data $T | GMT xyz2grd -I256+/256+ -R0/1/0/1 -G$GRD
GMT makecpt -C$C -T$Z > $CPT
GMT grdimage -B0.1g0.025 +$DEF $GRD -C$CPT $RNG $PRJ > $EPS
GMT grdinfo -M $GRD
GMT ps2raster -A -E35 -Tg $EPS
rm $GRD $CPT


