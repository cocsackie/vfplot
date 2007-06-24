# GMT plots for ellipse-stability
# $Id$

RNG="-R-5/5/-5/5"
PRJ="-JX5i"
BASE="ellipse-stability"
BASEB="$BASE-bezdet"
BASEZ="$BASE-zeros"
GRDB="$BASEB.grd"
EPSB="$BASEB.eps"
PNGB="$BASEB.png"
CPTB="$BASEB.cpt"
GRDZ="$BASEZ.grd"
EPSZ="$BASEZ.eps"
PNGZ="$BASEZ.png"
CPTZ="$BASEZ.cpt"
CPTPOS="$BASE-pos.cpt"
CPTNEG="$BASE-neg.cpt"

# GMT makecpt -Cpolar -T-20/20/1 > $CPTPOS
# GMT makecpt -Cpolar -T-1/1/0.05 > $CPTNEG

GMT grdimage $GRDB $RNG $PRJ -C$CPTB -K > $EPSB
GMT grdcontour $GRDB -A- $RNG $PRJ -C$CPTB -O >> $EPSB
convert -trim -rotate 90 $EPSB $PNGB
feh $PNGB &

GMT grdimage $GRDZ $RNG $PRJ -C$CPTZ -K > $EPSZ
GMT grdcontour $GRDZ -A- $RNG $PRJ -C$CPTZ -O >> $EPSZ
convert -trim -rotate 90 $EPSZ $PNGZ
feh $PNGZ &


#rm $CPTPOS $CPTNEG
