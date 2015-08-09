#!/bin/bash

source "$2/accept-setup.sh"

# -V
# version information

cmd="./sag2grd -V > /dev/null"
assert_raises "$cmd" 0

# -h
# show help

cmd="./sag2grd -h > /dev/null"
assert_raises "$cmd" 0

# fixture tests

cmd="./sag2grd -i $TESTFIX/test.sag u.grd v.grd"
assert_raises "$cmd" 0
assert_valid_grd u.grd
assert_valid_grd v.grd
rm -f u.grd v.grd


source accept-teardown.sh
