#!/bin/bash

source "$2/accept-setup.sh"

# common geometric options

geometry="-s0.75 -m4/4/0 -w4i"

# -h, --help
# print help and succeed

cmd="./vfplot -h > /dev/null"
assert_raises "$cmd" 0

# -V, --version
# print help and succeed

cmd="./vfplot -V > /dev/null"
assert_raises "$cmd" 0

# -p, --placement hedgehog
# create a hedgehog plot of all standard fields

for plot in circular cylinder electro2 electro3
do
    eps="$plot.eps"
    cmd="./vfplot -p hedgehog $geometry -t $plot -o $eps"
    assert_raises "$cmd" 0
    assert_valid_postscript $eps
    rm -f $eps
done

# -p, --placement adaptive
# create an adaptive plot

for plot in cylinder
do
    eps="$plot.eps"
    cmd="./vfplot -p adaptive -i30/5 $geometry -t $plot -o $eps"
    assert_raises "$cmd" 0
    assert_valid_postscript $eps
    rm -f $eps
done

# -d, --domain
# using a domain file

eps="circular.eps"
dom="$TESTFIX/circular.dom"
cmd="./vfplot -d $dom -i30/5 $geometry -t circular -o $eps"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps

# -G, --graphics-state
# create a vgs (vfplot graphic state) file

eps="cylinder.eps"
vgs="cylinder.vgs"
cmd="./vfplot -G $vgs -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_valid_vgs $vgs
assert_raises "$cmd" 0
rm -f $eps $vgs

# --dump-domain
# create a domain file, then run the plot using that domain

eps="cylinder.eps"
dom="cylinder.dom"
cmd="./vfplot --dump-domain $dom -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_valid_dom $dom
cmd="./vfplot --domain $dom -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps $dom

# --dump-vectors
# create a sag file of the vector field

eps="cylinder.eps"
sag="cylinder.sag"
cmd="./vfplot --dump-vectors $sag -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_valid_sag $sag
assert_valid_postscript $eps
rm -f $eps $sag

# --histogram
# create a hst data file for a histogram

eps="cylinder.eps"
hst="cylinder.hst"
cmd="./vfplot --histogram $hst -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_valid_hst $hst
assert_valid_postscript $eps
rm -f $eps $hst

# -g, --glyph
# the various arrow glyphs

for glyph in arrow triangle wedge
do
    eps="cylinder.eps"
    cmd="./vfplot --glyph $glyph -i30/5 $geometry -t cylinder -o $eps"
    assert_raises "$cmd" 0
    assert_valid_postscript $eps
    rm -f $eps
done

# --output-formy povray
# create povray output

pov="cylinder.pov"
cmd="./vfplot --output-format povray -i30/5 $geometry -t cylinder -o $pov"
assert_raises "$cmd" 0
assert_valid_povray $pov
rm -f $pov

# --break
# break processing early

for brk in corners decimate edges grid super midclean postclean none
do
    eps="cylinder.eps"
    cmd="./vfplot --break $brk -i30/5 $geometry -t cylinder -o $eps"
    assert_raises "$cmd" 0
    assert_valid_postscript $eps
    rm -f $eps
done

# -F, --format grd2
# input data from a pair of GMT netcdf (grd) files

eps="cylinder.eps"
grdu="$TESTFIX/cyl-u.grd"
grdv="$TESTFIX/cyl-v.grd"
cmd="./vfplot --format grd2 -i30/5 $geometry -o $eps $grdu $grdv"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps

# -F, --format sag
# input data from a sag file

base="input-from-sag"
eps="$base.eps"
sag="$base.sag"

# sag is a fat format so we generate it rather than have a fixture
cmd="./vfplot --dump-vectors $sag --break corners $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
rm -f $eps
assert_valid_sag $sag

# the test itself
cmd="./vfplot --format sag -i30/5 $geometry -o $eps $sag"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps $sag

# --animate
# create frames for an animation

pov="cylinder.eps"
cmd="./vfplot --animate povray -i5/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps

for i in $(seq 0 4)
do
    for j in $(seq 0 4)
    do
	eps=$(printf "anim.%04i.%04i.eps" $i $j)
	assert_valid_postscript $eps
	rm -f $eps
    done
done

# -P, --pen
# draw glyphs with specified pen

eps="cylinder.eps"
cmd="./vfplot -P  2/125 -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps

# -D, --domain-pen
# draw ellipses with specified pen

eps="cylinder.eps"
cmd="./vfplot --domain-pen 2/125 -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps

# --network
# draw ellipses with specified pen

eps="cylinder.eps"
cmd="./vfplot --network 2/125 -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps

# --ellipse-pen
# draw ellipses with specified pen

eps="cylinder.eps"
cmd="./vfplot -E --ellipse-pen 2/125 -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps

# --ellipse-fill
# draw ellipses with specified pen

eps="cylinder.eps"
cmd="./vfplot -E --ellipse-fill 200 -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps

# --sort
# specify the order in which glyphs are to be printed

for order in longest shortest bendiest straightest
do
    eps="cylinder.eps"
    cmd="./vfplot --sort $order -i30/5 $geometry -t cylinder -o $eps"
    assert_raises "$cmd" 0
    assert_valid_postscript $eps
    rm -f $eps
done

source accept-teardown.sh