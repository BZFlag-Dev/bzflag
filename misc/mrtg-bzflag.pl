#!/usr/bin/perl -w
#
# BZFlag
# Copyright (c) 1993 - 2008 Tim Riker
#
# This package is free software;  you can redistribute it and/or
# modify it under the terms of the license found in the file
# named COPYING that should have accompanied this file.
#
# THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

use strict;

# example mrtg.conf entries

#Target[bzflag]: `/home/bzflag/bzflag/misc/mrtg-bzflag.pl`
#Options[bzflag]: gauge,nopercent,noinfo,noborder,noarrow,transparent,growright
#Title[bzflag]: BZFlag Players / Servers
#MaxBytes[bzflag]: 100
#YLegend[bzflag]: count
#ShortLegend[bzflag]: &nbsp;
#LegendI[bzflag]: Players
#LegendO[bzflag]: Servers
#Legend1[bzflag]: Current Players according to list server
#Legend2[bzflag]: Current Servers according to list server
#Legend3[bzflag]: Maximal 5 Minute Players
#Legend4[bzflag]: Maximal 5 Minute Servers
#PageTop[bzflag]: <h1>BZFlag Players / Servers</h1>

#Target[xmission]: `/home/bzflag/bzflag/misc/mrtg-bzflag.pl xmission.bzflag.org:5154`
#Options[xmission]: gauge,noinfo,noborder,noarrow,transparent,growright
#Title[xmission]: Players on xmission.bzflag.org:5154
#MaxBytes[xmission]: 12
#YLegend[xmission]: players
#ShortLegend[xmission]: &nbsp;
#LegendI[xmission]: Players:
#LegendO[xmission]: MaxPlayers:
#Legend1[xmission]: Current players according to list server
#Legend2[xmission]: Current player limit according to list server
#Legend3[xmission]: Maximal 5 Minute players
#Legend4[xmission]: Maximal 5 Minute player limit
#PageTop[xmission]: <h1>xmission.bzflag.org:5155</h1>

use Socket;
use LWP::UserAgent;
use File::stat;

my $cacheFile = '/tmp/mrtg-bzflagdb.tmp';
my @lines;

if (-d $cacheFile && time() - stat($cacheFile)->mtime < 60) {
  open(CACHEFILE, "<$cacheFile");
  chomp(@lines = <CACHEFILE>);
  close(CACHEFILE);
} else {
  my $ua = new LWP::UserAgent;
  $ua->timeout(5);
  my $req = HTTP::Request->new('GET', 'http://my.BZFlag.org/db/?action=LIST');
  my $res = $ua->request($req);
  @lines = split("\n",$res->content);
  open(CACHEFILE, ">$cacheFile") or die;
  print(CACHEFILE join("\n", @lines));
  close(CACHEFILE);
}

my $totalServers = 0;
my $totalPlayers = 0;
for my $line (@lines) {
  my ($serverport, $version, $flags, $ip, $comments) = split(" ",$line,5);
  # not "(A4)18" to handle old dumb perl
  my ($style, $maxShots, $shakeWins, $shakeTimeout, $maxPlayerScore, $maxTeamScore, $maxTime,
      $maxPlayers, $rogueSize, $rogueMax, $redSize, $redMax, $greenSize, $greenMax,
      $blueSize, $blueMax, $purpleSize, $purpleMax, $observerSize, $observerMax) =
      unpack("A4A4A4A4A4A4A4A2A2A2A2A2A2A2A2A2A2A2A2A2", $flags);
  my $playerSize = hex($rogueSize) + hex($redSize) + hex($greenSize)
      + hex($blueSize) + hex($purpleSize) + hex($observerSize);
  if (($#ARGV == 0) && ($serverport eq $ARGV[0])) {
    my $playerMax = hex($rogueMax) + hex($redMax) + hex($greenMax)
        + hex($blueMax) + hex($purpleMax) + hex($observerMax);
    $playerMax = hex($maxPlayers) if (hex($maxPlayers) < $playerMax);
    print("$playerSize\n$playerMax\nunknown uptime\nplayers on $ARGV[0]\n");
    exit(0);
  }
  $totalServers += 1;
  $totalPlayers += $playerSize;
}
if ($#ARGV == 0) {
  print("0\n0\nunknown uptime\n$ARGV[0] not found in server list\n");
} else {
  print("$totalPlayers\n$totalServers\nunknown uptime\nBZFlag players/servers\n");
}

# Local Variables: ***
# mode:Perl ***
# tab-width: 8 ***
# c-basic-offset: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8
