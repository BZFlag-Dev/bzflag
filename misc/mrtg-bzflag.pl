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
my %servers;
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
  $servers{$serverport} = $playerSize;
  $totalServers += 1;
  $totalPlayers += $playerSize;
}
if ($#ARGV == 0) {
  if (defined($servers{$ARGV[0]})) {
    print("$servers{$ARGV[0]}\n0\nunknown uptime\nplayers on $ARGV[0]\n");
  } else {
    print("$ARGV[0] not found in server list\n");
  }
} else {
  print $ARGV[0];
  print("$totalServers\n$totalPlayers\nunknown uptime\nBZFlag servers/players\n");
}
