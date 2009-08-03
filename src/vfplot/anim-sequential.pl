# copies the file anim-*.*.png to a sequence of files
# sqa-*.png as needed by ffmpeg 

use POSIX;

$dir = ".";
opendir(DIR, $dir) || die "can't opendir $dir: $!";
@pngs = grep { /\.png/ } readdir(DIR);
closedir DIR;

@pngs = sort @pngs;

$n = log10(scalar @pngs) + 1;

$i = 1;
while ($png = shift @pngs)
{
    $seq = sprintf "sqa-%.*i.png",$n,$i;

    system "cp $png $seq";

    $i++;
}
