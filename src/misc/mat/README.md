Matlab binary
-------------

The _Octave_ script `shear.m` generates a Matlab binary file `shear.mat`
which was intended as a fixture for acceptance tests; but we find that
tests which pass locally with this file fail on the Travis CI server,
possibly this is a 32/64 bit issue (we are dealing with a binary format)
and since Matlab has now moved to a HDF5-based format for saving matrices
investigating this is low-priority.

The command which failed was

    vfplot --format mat -i5/5 -w6i -m4/4/0.5 -s1e-2 -o input-from-mat.eps shear.mat

with error on Travis being

    xrange is not two-element (8589934593x0)
    failed to read field
    failure plotting : failed to read file
