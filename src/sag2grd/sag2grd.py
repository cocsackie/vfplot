#!/usr/bin/env python3

import argparse as ap
import sys
import shutil
import subprocess as sp


def sag2grd_st(args, st):
    line = st.readline().rstrip()
    parts = line.split()
    if len(parts) != 11:
        raise RuntimeError('bad SAG header "%s"' % line)
    magic, ver, gdim, vdim, gxn, gyn, gxmin, gxmax, gymin, gymax, tol = parts

    if magic != '#sag':
        raise RuntimeError('bad magic "%s", not a SAG file' % magic)

    ver = int(ver)
    if ver != 1:
        raise RuntimeError('version %i not supported' % ver)

    gdim = int(gdim)
    if gdim != 2:
        raise RuntimeError('grid dimension %i not supported' % gdim)

    ngrd = len(args.grd)
    vdim = int(vdim)
    if ngrd != vdim:
        raise RuntimeError('SAG has vector dimension %i but %i grd files given'
                           % (vdim, ngrd))

    grdargs = ['-R%s/%s/%s/%s' % (gxmin, gxmax, gymin, gymax),
               '-I%s+/%s+' % (gxn, gyn)]

    info('output to:')
    for grd in args.grd:
        info('  %s' % grd)

    xyz2grd = shutil.which('xyz2grd')
    if xyz2grd:
        cmd = [xyz2grd]
    else:
        wrapper = shutil.which('GMT')
        if not wrapper:
            raise RuntimeError('neither "xyz2grd" nor "GMT" found')
        cmd = [wrapper, 'xyz2grd']

    info('gridder is %s' % ' '.join(cmd + grdargs))

    gridders = []
    for grd in args.grd:
        gridder = sp.Popen(cmd + ['-G%s' % grd] + grdargs, stdin=sp.PIPE)
        gridders.append(gridder)

    count = 0
    for line in st:
        parts = line.rstrip().split()
        x, y, *vals = parts
        for i in range(ngrd):
            data = bytes(' '.join([x, y, vals[i]]) + "\n", 'ascii')
            gridders[i].stdin.write(data)
        count += 1

    info("wrote %i points" % count)

    for gridder in gridders:
        gridder.stdin.close()
        gridder.wait()

    return 0


def sag2grd(args):
    info('This is sag2grd')

    if len(args.input) > 0:
        info('reading from %s' % (args.input[0]))
        st = open(args.input[0], 'r')
        err = sag2grd_st(args, st)
        st.close()
    else:
        err = sag2grd_st(args, sys.stdin)

    return err

parser = ap.ArgumentParser(prog='sag2grd')
parser.add_argument('--input', '-i', metavar='<sag>', nargs=1,
                    help='read sag input from file')
parser.add_argument('--verbose', '-v', action='store_true',
                    help='verbose operation')
parser.add_argument('--version', '-V', action='version', version='1.0.12')
parser.add_argument('grd', nargs='+', metavar='<grd>', help='GMT grd file')

args = parser.parse_args()

if args.verbose:
    def info(a):
        print(a)
else:
    def info(a):
        pass

err = sag2grd(args)

if err:
    info('failed.')
else:
    info('done.')

exit(err)
