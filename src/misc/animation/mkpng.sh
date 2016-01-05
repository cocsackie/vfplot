#! /bin/sh
#
# usage:
#
#   sh mkpng.sh <file>.eps
#
# creates <file>.png, intended for vfplot animation
# output.
#
# J.J. Green 2015

eps=$1
base=`basename $eps .eps`
num=`echo $base | cut -c 6-14`
png="$base.png"
logo="vfplot 1.0.12"
echo $png
convert -format png -flatten -depth 8 \
    -fill white  -undercolor '#00000080' \
    -font Bitstream-Vera-Sans-Bold -pointsize 10 \
    -gravity northeast -annotate +4+4 " $num " \
    -gravity northwest -annotate +4+4 " $logo " \
    $eps $png
