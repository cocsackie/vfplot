#!/bin/sh
#
# create the

SAG="/tmp/cyl.sag"
EPS="/tmp/cyl.eps"
vfplot --dump-vectors $SAG --break corners -t cylinder -o $EPS
sag2grd -i $SAG cyl-u-128.grd cyl-v-128.grd
GMT grdsample -I64+ cyl-u-128.grd -Gcyl-u.grd
GMT grdsample -I64+ cyl-v-128.grd -Gcyl-v.grd
rm -f $SAG $EPS cyl-u-128.grd cyl-v-128.grd
