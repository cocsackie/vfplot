#! /bin/sh
#
# usage:
#
#   sh mkpng.sh file.eps
#
# creates file.png, intended for vfplot
# animation frames
#
# J.J. Green 2008
# $Id: mkpng.sh,v 1.1 2008/09/19 21:48:45 jjg Exp $

eps=$1
base=`basename $eps .eps`
num=`echo $base | cut -c 6-14`
png="$base.png"
logo="vfplot 1.0.3"
echo $png
convert -format png -flatten -depth 8 \
    -fill white  -undercolor '#00000080' \
    -font Bitstream-Vera-Sans-Bold -pointsize 10 \
    -gravity northeast -annotate +4+4 " $num " \
    -gravity northwest -annotate +4+4 " $logo " \
    $eps $png
