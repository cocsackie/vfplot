#
# sag2grd
#
# convert simple ascii grid (sag) files to a pair of
# GMT netCDF (grd) files - this is done by piping the
# data to the GMT program xyz2grd 
#
# J.J. Green 2008
# $Id: sag2grd.pl,v 1.4 2010/08/09 19:20:39 jjg Exp jjg $

use strict;
use POSIX;
use Getopt::Std;

my $ist;
my @gsts = ();

my $usage = <<EOF
usage: sag2grd [options] <grd u> ...

where <grd u> ...  are the names of the grd files to be 
written and options can include

-h         : brief help
-i <file>  : read sag input from <file> rather than stdin
-v         : verbose
-V         : version
EOF
  ;

my %opt = ();
getopts('hi:vV',\%opt);

if ($opt{h})
{
    print $usage;
    exit EXIT_SUCCESS;
}

if ($opt{V})
{
    print "sag2grd $version\n";
    exit EXIT_SUCCESS;
}

sub info
{
    if ($opt{v})
    {
	print @_;
    }
}

info "This is sag2grd\n";

my $input = $opt{i} || "-";

unless (open $ist,"< $input")
{
    warn "failed to open $input to read\n";
    exit EXIT_FAILURE;
}

my $grdargs;
my $ngrd = scalar @ARGV;

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

    if ($ngrd != $vdim)
    {
	warn "sag file has vector dimension $vdim, but $ngrd grd files specified\n";
	exit EXIT_FAILURE;
    }

    $grdargs = "-R$gxmin/$gxmax/$gymin/$gymax -I$gxn+/$gyn+";
}

my @grds = @ARGV;

info "output to\n";

foreach my $grd (@grds)
{
    info "  $grd\n";
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

# create an array of stream to gridding processes,

foreach my $grd (@grds)
{
    my $gst;
    my $cmd = "| $xyz2grd -G$grd $grdargs";

    unless (open($gst,$cmd))
    {
	warn "failed to open process $cmd\n";
	exit EXIT_FAILURE;
    }

    push @gsts,$gst;
}

my $nline = 0;

sub trim
{
    my $s = shift;

    $s =~ s/^\s+//;
    $s =~ s/\s+$//;

    return $s;
}

while (defined(my $line = <$ist>))
{
    my ($x,$y,@u) = split(/\s+/,trim($line));

    for (my $i=0 ; $i<$ngrd ; $i++)
    {
	my $gst = $gsts[$i];
	printf $gst "%f %f %f\n",$x,$y,$u[$i];
    }

    $nline++;
}

info "wrote $nline points\n";

sub END
{
    my $err = 0;

    foreach my $gst (@gsts)
    { 
	close $gst;
    }

    if (defined $ist)
    {
	close $ist;
    }

    info "done.\n";
}

