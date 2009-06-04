#!/usr/bin/perl

# This is a simple script that uses bzadmin to create a chat gateway between
# two bzflag servers.
#
# Use it like this:
#
#   bzbz.pl <CALLSIGN1>@<HOST1>:<PORT1> <CALLSIGN2>@<HOST2>:<PORT2> [OPTIONS]
#
# The options will be given to both bzadmin processes.
#

use IPC::Open2;


# parse the command line
$bz1 = $ARGV[0];
$bz2 = $ARGV[1];
$options = join(' ', @ARGV[2 .. $#ARGV]);
$callsign1 = '@' . substr($bz1, 0, index($bz1, '@'));
$callsign2 = '@' . substr($bz2, 0, index($bz2, '@'));

print "Creating a gateway between $bz1 and $bz2...\n";

# start the two bzadmin processes
$pid1 = open2(\*RDRFH1, \*WTRFH1, "bzadmin -ui stdboth $bz1 $options");
$pid2 = open2(\*RDRFH2, \*WTRFH2, "bzadmin -ui stdboth $bz2 $options");

while (true) {

  # check for new data from the servers
  $rin = $win = $ein = '';
  vec($rin, fileno(RDRFH1), 1) = 1;
  vec($rin, fileno(RDRFH2), 1) = 1;
  select($rin, $win, $ein, 0.01);

  # new data from server 1, print it to server 2
  if (vec($rin, fileno(RDRFH1), 1) == 1) {
    sysread RDRFH1, $c, 1;
    if ($c ne "\n") {
      $line1 = $line1 . $c;
    }
    elsif (substr($line1, 0,
		  length("    $callsign1: ")) eq "    $callsign1: ") {
      $line1 = "";
    }
    else {
      $line1 = $line1 . $c;
      syswrite WTRFH2, $line1, length($line1);
      $line1 = "";
    }
  }

  # new data from server 2, print it to server 1
  if (vec($rin, fileno(RDRFH2), 1) == 1) {
    sysread RDRFH2, $c, 1;
    if ($c ne "\n") {
      $line2 = $line2 . $c;
    }
    elsif (substr($line2, 0, length("    $callsign2: ")) eq
	   "    $callsign2: ") {
      $line2 = "";
    }
    else {
      $line2 = $line2 . $c;
      syswrite WTRFH1, $line2, length($line2);
      $line2 = "";
    }
  }
}

# Local Variables: ***
# mode:Perl ***
# tab-width: 8 ***
# c-basic-offset: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8
