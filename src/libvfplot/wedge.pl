# /usr/bin/perl -w
#
# test of plotting a circular wedge by postcript
# bezier curves - seems OK up to 70-80 degrees 
#
# $Id: wedge.pl,v 1.2 2012/05/17 12:28:55 jjg Exp $

$R = 400;
$r = 200;
$t = 55;

$RAD = 1/57.29577951308;

sub cosd 
{
    my $t = shift;
    cos($t * $RAD);
}

sub sind 
{
    my $t = shift;
    sin($t * $RAD);
}

sub rot
{
    my ($t,$x,$y) = @_;

    return (int(cosd($t)*$x - sind($t)*$y),
	    int(sind($t)*$x + cosd($t)*$y));
}

$m = ($R+$r)/2;

$L1 = $R * $t * $RAD;
$L2 = $m * $t * $RAD;
$L3 = $r * $t * $RAD;

($x1,$y1) = (int(5*$R/6 + $r/6),int($L1/3));
($x2,$y2) = rot($t,4*$R/6 + 2*$r/6,-$L2/3);
($x3,$y3) = (int($m * cosd($t)),int($m * sind($t)));
($x4,$y4) = rot($t,2*$R/6 + 4*$r/6,-$L2/3);
($x5,$y5) = (int($R/6 + 5*$r/6),int($L3/3));

($X1,$Y1) = rot($t/2,(3*$R+$r)/4,0);
($X2,$Y2) = rot($t/2,($R+3*$r)/4,0);

$ps = <<EOF
%!PS-Adobe-2.0 EPSF-2.0
%%BoundingBox: -10 -10 410 410
%%EndComments

0 setgray

newpath
$R 0 moveto 
$x1 $y1 $x2 $y2 $x3 $y3 curveto
$x4 $y4 $x5 $y5 $r 0 curveto
closepath
gsave 
0.7 setgray
fill
grestore
stroke

0.3 setgray

newpath
$R 0 moveto 
$x1 $y1 lineto
$x2 $y2 lineto
$x3 $y3 lineto
$x4 $y4 lineto
$x5 $y5 lineto
$r 0 lineto
stroke

0 setgray

newpath
$R 0 2 0 360 arc
stroke

newpath
$x1 $y1 2 0 360 arc
stroke

newpath
$x2 $y2 2 0 360 arc
stroke

newpath
$x3 $y3 2 0 360 arc
stroke

newpath
$x4 $y4 2 0 360 arc
stroke

newpath
$x5 $y5 2 0 360 arc
stroke

newpath
$r 0 2 0 360 arc
stroke

newpath
0 0 $R 0 $t arc
0 0 lineto
closepath
stroke

newpath
0 0 $m 0 $t arc
stroke

newpath
0 0 $r 0 $t arc
stroke

newpath 
$X1 $Y1 4 0 360 arc
stroke

newpath 
$X2 $Y2 4 0 360 arc
stroke

showpage
EOF
;

print $ps;
