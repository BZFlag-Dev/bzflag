#!/usr/bin/perl -w
#
# BZFlag
# Copyright (c) 1993 - 2003 Tim Riker
#
# This package is free software;  you can redistribute it and/or
# modify it under the terms of the license found in the file
# named LICENSE that should have accompanied this file.
#
# THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.

use strict;

my $no_BZFlag;

use Socket;
use LWP::UserAgent;

my $ua = new LWP::UserAgent;

$ua->timeout(5);

my $req = HTTP::Request->new('GET', 'http://list.bzflag.org:5156/');
my $res = $ua->request($req);
my $totalServers = 0;
my $totalPlayers = 0;
for my $line (split("\n",$res->content)) {
  my ($serverport, $version, $flags, $ip, $comments) = split(" ",$line,5);
  # not "(A4)18" to handle old dumb perl
  my ($style,$maxPlayers,$maxShots,
      $rogueSize,$redSize,$greenSize,$blueSize,$purpleSize,
      $rogueMax,$redMax,$greenMax,$blueMax,$purpleMax,
      $shakeWins,$shakeTimeout,
      $maxPlayerScore,$maxTeamScore,$maxTime) =
      unpack("A4A4A4A4A4A4A4A4A4A4A4A4A4A4A4A4A4A4", $flags);
  my $playerSize = hex($rogueSize) + hex($redSize) + hex($greenSize)
      + hex($blueSize) + hex($purpleSize);
  my $playerMax = hex($rogueMax) + hex($redMax) + hex($greenMax)
      + hex($blueMax) + hex($purpleMax);
  if (($#ARGV == 0) && ($serverport eq $ARGV[0])) {
    print("$playerSize\n$playerMax\nunknown uptime\nplayers on $ARGV[0]\n");
    exit(0);
  }
  $totalServers += 1;
  $totalPlayers += $playerSize;
}
if ($#ARGV == 0) {
  print("$ARGV[0] not found in server list\n");
} else {
  print("$totalPlayers\n$totalServers\nunknown uptime\nBZFlag players/servers\n");
}
