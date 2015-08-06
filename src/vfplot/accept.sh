#!/bin/bash

source "$2/accept-setup.sh"

# common geometric options

geometry="-s0.75 -m4/4/0 -w4i"

# create a hedgehog plot of all standard fields

for plot in circular cylinder electro2 electro3
do
    eps="$plot.eps"
    cmd="./vfplot -p hedgehog $geometry -t $plot -o $eps"
    assert_raises "$cmd" 0
    assert_raises "[ -e $eps ]" 0
    rm -f $eps
done

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
cmd="./vfplot -p adaptive -d $dom -i30/5 $geometry -t circular -o $eps"
assert_raises "$cmd" 0
assert_raises "[ -e $eps ]" 0
rm -f $eps

# -G, --graphics-state
# create a vgs (vfplot graphic state) file

eps="cylinder.eps"
vgs="cylinder.vgs"
cmd="./vfplot -p adaptive -G $vgs -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_raises "[ -e $vgs ]" 0
assert_raises "$cmd" 0
rm -f $eps $vgs

# --dump-domain
# create a domain file, then run the plot using that domain

eps="cylinder.eps"
dom="cylinder.dom"
cmd="./vfplot -p adaptive --dump-domain $dom -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_raises "[ -e $dom ]" 0
cmd="./vfplot -p adaptive --domain $dom -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_raises "[ -e $eps ]" 0
rm -f $eps $dom

# --dump-vectors
# create a sag file of the vector field

eps="cylinder.eps"
sag="cylinder.sag"
cmd="./vfplot -p adaptive --dump-vectors $sag -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_raises "[ -e $sag ]" 0
assert_raises "[ -e $eps ]" 0
rm -f $eps $sag

# --histogram
# create a sag file of the vector field

eps="cylinder.eps"
hst="cylinder.hst"
cmd="./vfplot -p adaptive --histogram $hst -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_raises "[ -e $hst ]" 0
assert_raises "[ -e $eps ]" 0
rm -f $eps $hst

source accept-teardown.sh
