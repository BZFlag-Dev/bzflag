#!/usr/bin/perl -w
#
# Quick hack to convert world rotation formats
# by Tim@Rikers.org
# 1.7e2 and earlier use radians
# 1.7e3 and later use degrees
# script converts either way by looking to see
# if all values are < or > 2 * pi
# usage: ./bzwcvt.pl < hix.bzw > hixr.bzw
$pi = 3.14159265358979;
@bzw = <> ;
@rotations = grep( /^rotation/ , @bzw ) ;
$max = 0 ;
foreach ( @rotations ) {
  ( undef, $rotation ) = split /  */ ;
  $max = $rotation if ( $max < $rotation ) ;
}
chomp( $max );
print STDERR "max rotation is $max, converting " ;
if ( $max > 2 * $pi ) {
  print STDERR "degrees to radians\n" ;
} else {
  print STDERR "radians to degrees\n" ;
}
foreach ( @bzw ) {
  if ( /^rotation/ ) {
    ( undef, $rotation ) = split /  */ ;
    print "rotation " ;
    if ( $max > 2 * $pi ) {
      print $rotation / 180 * $pi;
    } else {
      print sprintf("%.1f", $rotation * 180 / $pi );
    }
    print "\n";
  } else {
    print "$_";
  }
}

# Local Variables: ***
# mode:Perl ***
# tab-width: 8 ***
# c-basic-offset: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8
