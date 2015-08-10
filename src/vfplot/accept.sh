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

# -p, --placement list
# list available placement strategies

cmd="./vfplot -p list > /dev/null"
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

# -g, --glyphs list
# list available glyphs

cmd="./vfplot -g list > /dev/null"
assert_raises "$cmd" 0

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

# --output-format list
# list available output formats

cmd="./vfplot --output-format list > /dev/null"
assert_raises "$cmd" 0

# --output-format povray
# create povray output

pov="cylinder.pov"
cmd="./vfplot --output-format povray -i30/5 $geometry -t cylinder -o $pov"
assert_raises "$cmd" 0
assert_valid_povray $pov
rm -f $pov

# --break list
# list available breaks

cmd="./vfplot --break list > /dev/null"
assert_raises "$cmd" 0

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

# -F, --format list
# list available input formats

cmd="./vfplot --format list > /dev/null"
assert_raises "$cmd" 0

# -F, --format grd2
# input data from a pair of GMT netcdf (grd) files

eps="cylinder.eps"
grdu="$TESTFIX/cyl-u.grd"
grdv="$TESTFIX/cyl-v.grd"

# explicit format
cmd="./vfplot --format grd2 -i30/5 $geometry -o $eps $grdu $grdv"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps

# detect format
cmd="./vfplot -i30/5 $geometry -o $eps $grdu $grdv"
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

# explicit format
cmd="./vfplot --format sag -i30/5 $geometry -o $eps $sag"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps

# detect format
cmd="./vfplot -i30/5 $geometry -o $eps $sag"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps $sag

# -F, --format gfs
# input data from a gfs file

base="input-from-gfs"
eps="$base.eps"
gfs="$TESTFIX/house-001.gfs"

# explicit format
cmd="./vfplot --format gfs -i5/5 -w6i -m4/4/0.5 -s3e-4 -o $eps $gfs"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps

# autodetect
cmd="./vfplot -i5/5 -w6i -m4/4/0.5 -s3e-4 -o $eps $gfs"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps

# -F, --format mat
# input data from a mat file

base="input-from-mat"
eps="$base.eps"
mat="$TESTFIX/shear.mat"

# explicit format
cmd="./vfplot --format mat -i5/5 -w6i -m4/4/0.5 -s1e-2 -o $eps $mat"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps

# autodetect
cmd="./vfplot -i5/5 -w6i -m4/4/0.5 -s1e-2 -o $eps $mat"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps

./vfplot --format mat -i5/5 -w6i -m4/4/0.5 -s1e-2 -o input-from-mat.eps ../fixtures/shear.mat

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

# --sort list
# list available oderings

cmd="./vfplot --sort list > /dev/null"
assert_raises "$cmd" 0

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

# -o, --output
# when absent, write to stdout

base="output-to-stdout"
eps="$base.eps"
cmd="./vfplot -i30/5 $geometry -t cylinder > $eps"
assert_raises "$cmd" 0
assert_valid_postscript $eps
rm -f $eps

source accept-teardown.sh
