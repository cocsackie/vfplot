#!/bin/bash

source "$2/accept-setup.sh"

# common geometric options

geometry="-s0.75 -m4/4/0 -w4i"

# -p, --placement hedgehog
# create a hedgehog plot of all standard fields

for plot in circular cylinder electro2 electro3
do
    eps="$plot.eps"
    cmd="./vfplot -p hedgehog $geometry -t $plot -o $eps"
    assert_raises "$cmd" 0
    assert_raises "[ -e $eps ]" 0
    rm -f $eps
done

# -p, --placement adaptive
# create an adaptive plot

for plot in cylinder
do
    eps="$plot.eps"
    cmd="./vfplot -p adaptive -i30/5 $geometry -t $plot -o $eps"
    assert_raises "$cmd" 0
    assert_raises "[ -e $eps ]" 0
    rm -f $eps
done

# -d, --domain
# using a domain file

eps="circular.eps"
dom="$TESTFIX/circular.dom"
cmd="./vfplot -d $dom -i30/5 $geometry -t circular -o $eps"
assert_raises "$cmd" 0
assert_raises "[ -e $eps ]" 0
rm -f $eps

# -G, --graphics-state
# create a vgs (vfplot graphic state) file

eps="cylinder.eps"
vgs="cylinder.vgs"
cmd="./vfplot -G $vgs -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_raises "[ -e $vgs ]" 0
assert_raises "$cmd" 0
rm -f $eps $vgs

# --dump-domain
# create a domain file, then run the plot using that domain

eps="cylinder.eps"
dom="cylinder.dom"
cmd="./vfplot --dump-domain $dom -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_raises "[ -e $dom ]" 0
cmd="./vfplot --domain $dom -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_raises "[ -e $eps ]" 0
rm -f $eps $dom

# --dump-vectors
# create a sag file of the vector field

eps="cylinder.eps"
sag="cylinder.sag"
cmd="./vfplot --dump-vectors $sag -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_raises "[ -e $sag ]" 0
assert_raises "[ -e $eps ]" 0
rm -f $eps $sag

# --histogram
# create a sag file of the vector field

eps="cylinder.eps"
hst="cylinder.hst"
cmd="./vfplot --histogram $hst -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_raises "[ -e $hst ]" 0
assert_raises "[ -e $eps ]" 0
rm -f $eps $hst

# -g, --glyph
# the various arrow glyphs

for glyph in arrow triangle wedge
do
    eps="cylinder.eps"
    cmd="./vfplot --glyph $glyph -i30/5 $geometry -t cylinder -o $eps"
    assert_raises "$cmd" 0
    assert_raises "[ -e $eps ]" 0
    rm -f $eps
done

# --output-formy povray
# create povray output

pov="cylinder.eps"
cmd="./vfplot --output-format povray -i30/5 $geometry -t cylinder -o $pov"
assert_raises "$cmd" 0
assert_raises "[ -e $pov ]" 0
rm -f $pov

# --break
# break processing early

for brk in corners decimate edges grid super midclean postclean none
do
    eps="cylinder.eps"
    cmd="./vfplot --break $brk -i30/5 $geometry -t cylinder -o $eps"
    assert_raises "$cmd" 0
    assert_raises "[ -e $eps ]" 0
    rm -f $eps
done

# -F, --format grd2
# input data from a pair of GMT netcdf (grd) files

eps="cylinder.eps"
grdu="$TESTFIX/cyl-u.grd"
grdv="$TESTFIX/cyl-v.grd"
cmd="./vfplot --format grd2 -i30/5 $geometry -o $eps $grdu $grdv"
assert_raises "$cmd" 0
assert_raises "[ -e $eps ]" 0
rm -f $eps


source accept-teardown.sh
