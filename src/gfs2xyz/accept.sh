#!/bin/bash

source "$2/accept-setup.sh"

# -V
# version information

cmd="./gfs2xyz2D -V > /dev/null"
assert_raises "$cmd" 0

# -h
# show help

cmd="./gfs2xyz2D -h > /dev/null"
assert_raises "$cmd" 0

# fixture tests

gfs="$TESTFIX/house-001.gfs"

# default column output

dat="column.dat"
cmd="./gfs2xyz2D -o $dat $gfs"
assert_raises "$cmd" 0
rm -f $dat

# specify a scalar

dat="vorticity.dat"
cmd="./gfs2xyz2D --scalar Vorticity -o $dat $gfs"
assert_raises "$cmd" 0
rm -f $dat

# use indices rather than xy values

dat="indices.dat"
cmd="./gfs2xyz2D --index -o $dat $gfs"
assert_raises "$cmd" 0
rm -f $dat

# create a sag file

sag="with-sag-header.sag"
cmd="./gfs2xyz2D --sag -o $sag $gfs"
assert_raises "$cmd" 0
assert_valid_sag $sag
rm -f $sag


source accept-teardown.sh
