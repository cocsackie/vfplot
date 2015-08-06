#!/bin/bash

source "$2/accept-setup.sh"

# can we create the test-plots

for plot in cylinder electro2 electro3
do
    eps="$plot.eps"
    cmd="./vfplot -s0.75 -m4/4/0 -w4i -i30/5 -t $plot -o $eps"
    assert_raises "$cmd" 0
    rm -f $eps
done

source accept-teardown.sh
