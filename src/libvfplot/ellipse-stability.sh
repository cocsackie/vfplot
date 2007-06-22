RNG="-R-4/4/0/5.0"
PRJ="-JX5i"
BASE="ellipse-stability"
DAT="$BASE.dat"
GRD="$BASE.grd"
EPS="$BASE.eps"
PNG="$BASE.png"
CPT="$BASE.cpt"

GMT makecpt -Cpolar -T-10/10/0.1 > $CPT
GMT grdimage $GRD $RNG $PRJ -C$CPT -K > $EPS
GMT grdcontour $GRD -A- $RNG $PRJ -C$CPT -O >> $EPS
convert -trim -rotate 90 $EPS $PNG
feh $PNG
rm $CPT
