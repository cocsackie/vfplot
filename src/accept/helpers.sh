#!/bin/bash
# this file is sourced, not executed
#
# helper functions for acceptance tests
#
# J.J. Green 2015

# the base asserts, each using assert_raises to check and
# record a property of the file

function assert_file_nonzero {
    assert_raises "[ -s $1 ]" 0
}

function assert_parsed_by_gs {
    cmd="gs -sDEVICE=nullpage -dNOPAUSE -dBATCH $1"
    assert_raises "$cmd" 0
}

# per-format assert_valid function; these are the ones used
# in the acceptance tests

function assert_valid_postscript {
    assert_file_nonzero $1
    assert_parsed_by_gs $1
}

function assert_valid_povray {
    assert_file_nonzero $1
    # FIXME
}

function assert_valid_dom {
    assert_file_nonzero $1
    # FIXME
}

function assert_valid_hst {
    assert_file_nonzero $1
    # FIXME
}

function assert_valid_vgs {
    assert_file_nonzero $1
    # FIXME
}

function assert_valid_sag {
    assert_file_nonzero $1
    # FIXME
}
