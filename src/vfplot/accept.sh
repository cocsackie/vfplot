#!/bin/bash

source "$2/accept-setup.sh"

# common geometric options

geometry="-s0.75 -m4/4/0 -w4i"

# create a hedgehog plot of all standard plots

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

# using a domain file

eps="circular.eps"
dom="$TESTFIX/circular.dom"
cmd="./vfplot -p adaptive -d $dom -i30/5 $geometry -t circular -o $eps"
assert_raises "$cmd" 0
assert_raises "[ -e $eps ]" 0
rm -f $eps

# create a vgs (vfplot graphic state) file

eps="cylinder.eps"
vgs="cylinder.vgs"
cmd="./vfplot -p adaptive -G $vgs -i30/5 $geometry -t cylinder -o $eps"
assert_raises "$cmd" 0
assert_raises "[ -e $vgs ]" 0
assert_raises "$cmd" 0
rm -f $eps $vgs

source accept-teardown.sh
