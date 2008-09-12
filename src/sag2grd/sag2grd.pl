#! /usr/bin/perl -w
#
# sag2grd
#
# convert simple ascii grid (sag) files to a pair of
# GMT netCDF (grd) files - this is done by piping the
# data to the GMT program xyz2grd 
#
# J.J. Green 2008
# $Id$

use strict;
use POSIX;
use Getopt::Std;

my $usage = <<EOF
usage: sag2grd [options] <grd u> <grd v>

where <grd u> <grd v> are the names of the
grd files to be written and options can include

-h         : brief help
-i <file>  : read sag input from <file> rather than stdin
-v         : verbose
EOF
  ;

my %opt = ();
getopts('hi:v',\%opt);

if ($opt{h})
{
    print $usage;
    exit EXIT_SUCCESS;
}

unless (scalar @ARGV == 2)
{
    warn "exactly two output grd files must be given\n\n$usage";
    exit EXIT_FAILURE;
}

sub info
{
    if ($opt{v})
    {
	print @_;
    }
}

info "This is sag2grd\n";

my %grd;

($grd{u},$grd{v}) = @ARGV;

info "output to\n";
info "  $grd{u}\n";
info "  $grd{v}\n";

my $input = $opt{i} || "-";

my $ist;

unless (open $ist,"< $input")
{
    warn "failed to open $input to read\n";
    exit EXIT_FAILURE;
}

sub END
{
    close($ist);
}

my $grdargs;

if (defined(my $line = <$ist>))
{
    chomp $line;

    my ($magic,$ver,$gdim,$vdim,$gxn,$gyn,
	$gxmin,$gxmax,$gymin,$gymax) = split(/\s+/,$line);

    unless ($magic eq '#sag')
    {
	warn "bad magic $magic - not a sag file\n";
	exit EXIT_FAILURE;
    }

    if ($ver != 1)
    {
	warn "can't read sag version $ver\n";
	exit EXIT_FAILURE;
    }

    if ($gdim != 2)
    {
	warn "can't handle sag file with grid dimension $gdim\n";
	exit EXIT_FAILURE;
    }

    if ($vdim != 2)
    {
	warn "can't handle sag file with vector dimension $vdim\n";
	exit EXIT_FAILURE;
    }

    $grdargs = "-R$gxmin/$gxmax/$gymin/$gymax -I$gxn+/$gyn+";
}

my $xyz2grd;

if ($xyz2grd = `which xyz2grd`)
{
    chomp $xyz2grd;
}
else
{
    my $wrapper = "GMT";

    if (my $gmtcmd = `which $wrapper`)
    {
	chomp $gmtcmd;
	$xyz2grd = "$gmtcmd xyz2grd";
    }
}

unless ($xyz2grd)
{
    warn "failed to find xyz2grd program";
    exit EXIT_FAILURE;
}

info "gridder is $xyz2grd $grdargs\n";

my $ust;
my $ucmd = "| $xyz2grd -G$grd{u} $grdargs";

unless (open($ust,$ucmd))
{
    warn "failed to open process $ucmd\n";
    exit EXIT_FAILURE;
}

sub END
{
    close($ust);
}

my $vst;
my $vcmd = "| $xyz2grd -G$grd{v} $grdargs";

unless (open($vst,$vcmd))
{
    warn "failed to open process $vcmd\n";
    exit EXIT_FAILURE;
}

sub END
{
    close($vst);
}

my $nline = 0;

while (defined(my $line = <$ist>))
{
    chomp $line;

    my ($x,$y,$u,$v) = split(/\s+/,$line);

    print $ust "$x $y $u\n";
    print $vst "$x $y $v\n";

    $nline++;
}

info "wrote $nline points\n";

sub END
{
    info "done.\n";
}

exit EXIT_SUCCESS;
