# plot data from ellipse_debug
# $Id$

RNG="-R0/10/0/10"
PRJ="-JX5i"
DAT="ellipse-debug.dat"
GRD="ellipse-debug.grd"
EPS="ellipse-debug.eps"
PNG="ellipse-debug.png"
CPT="/usr/share/gmt/cpt/GMT_cpt-city/cb/qual/Accent_03.cpt"

GMT xyz2grd $DAT -G$GRD $RNG -F -I0.05
GMT grdimage $GRD $RNG $PRJ -C$CPT > $EPS
convert -trim -rotate 90 $EPS $PNG
feh $PNG
