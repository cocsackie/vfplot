#!/bin/sh
# script to build vfplot sources from a github checkout,
# of course you will need a fairly full set of development
# tools to to this (autoconf, gengetopt, xsltproc,..)

autoconf
autoheader
./configure
make all
make spotless
