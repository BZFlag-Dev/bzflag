#!/usr/bin/perl
#
# bzflag
# Copyright (c) 1993 - 2002 Tim Riker
#
# This package is free software;  you can redistribute it and/or
# modify it under the terms of the license found in the file
# named LICENSE that should have accompanied this file.
#
# THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.

#
# contact a bzflag server and print the game status to stdout
#
use Socket;

# get arguments:  server [port]
($servername,$port) = @ARGV;
$port = 5155 unless $port;

# some socket defines
$sockaddr = 'S n a4 x8';

# port to port number
($name,$aliases,$proto) = getprotobyname('tcp');
($name,$aliases,$port)  = getservbyname($port,'tcp') unless $port =~ /^\d+$/;

# get server address
($name,$aliases,$type,$len,$serveraddr) = gethostbyname($servername);
$server = pack($sockaddr, AF_INET, $port, $serveraddr);

# connect
die $! unless socket(S1, AF_INET, SOCK_STREAM, $proto);
die $! unless connect(S1, $server);

# don't buffer
select(S1); $| = 1; select(STDOUT);

# get hello
die $! unless sysread(S1, $buffer, 10) == 10;

# parse reply
($magic,$major,$minor,$revision,$port) = unpack("a4 a1 a2 a1 n", $buffer);

# quit if version isn't valid
die "not a bzflag server" if ($magic ne "BZFS");
die "incompatible version" if ($major < 1);
die "incompatible version" if ($major == 1 && $minor < 7);
die "incompatible version" if ($major == 1 && $minor == 7 && $revision eq "b");

# quit if rejected
die "rejected by server" if ($port == 0);

# reconnect on new port
$server = pack($sockaddr, AF_INET, $port, $serveraddr);
die $! unless socket(S, AF_INET, SOCK_STREAM, $proto);
die $! unless connect(S, $server);
select(S); $| = 1; select(STDOUT);

# close first socket
close(S1);

# send game request
print S pack("n2", 0, 0x7167);

# get reply
die $! unless sysread(S, $buffer, 40) == 40;
($len,$code,$style,$maxPlayers,$maxShots,
	$rogueSize,$redSize,$greenSize,$blueSize,$purpleSize,
	$rogueMax,$redMax,$greenMax,$blueMax,$purpleMax,
	$shakeWins,$shakeTimeout,
	$maxPlayerScore,$maxTeamScore,$maxTime) = unpack("n20", $buffer);
die $! unless $code == 0x7167;

# print info
print "style:";
print " CTF" if $style & 0x0001;
print " flags" if $style & 0x0002;
print " rogues" if $style & 0x0004;
print " jumping" if $style & 0x0008;
print " inertia" if $style & 0x0010;
print " ricochet" if $style & 0x0020;
print " shaking" if $style & 0x0040;
print " antidote" if $style & 0x0080;
print " time-sync" if $style & 0x0100;
print "\n";
print "maxPlayers: $maxPlayers\nmaxShots: $maxShots\n";
print "team sizes: $rogueSize $redSize $greenSize $blueSize $purpleSize" .
	" (rogue red green blue purple)\n";
print "max sizes:  $rogueMax $redMax $greenMax $blueMax $purpleMax\n";
if ($style & 0x0040) {
  print "wins to shake bad flag: $shakeWins\n";
  print "time to shake bad flag: ";
  print $shakeTimeout / 10;
  print "\n";
}
print "max player score: $maxPlayerScore\n";
print "max team score: $maxTeamScore\n";
print "max time: ";
print $maxTime / 10;
print "\n";

# send players request
print S pack("n2", 0, 0x7170);

# get number of teams and players we'll be receiving
die $! unless sysread(S, $buffer, 8) == 8;
($len,$code,$numTeams,$numPlayers) = unpack("n4", $buffer);
die $! unless $code == 0x7170;

# get the teams
print "\n";
@teamName = ("Rogue", "Red", "Green", "Blue", "Purple");
for (1..$numTeams) {
die $! unless sysread(S, $buffer, 14) == 14;
($len,$code,$team,$size,$aSize,$won,$lost) = unpack("n7", $buffer);
die $! unless $code == 0x7475;
$score = $won - $lost;
print "$teamName[$team] team:\n";
print "  $aSize players\n  score: $score ($won wins, $lost losses)\n";
}
print "\n";

# get the players
@playerType = ("tank", "observer", "robot tank");
for (1..$numPlayers) {
die $! unless sysread(S, $buffer, 180) == 180;
($len,$code,$pAddr,$pPort,$pNum,$type,$team,$won,$lost,$sign,$email) =
					unpack("n2Nn2 n4A32A128", $buffer);
die $! unless $code == 0x6170;
$score = $won - $lost;
print "player $sign ($teamName[$team] team) is a $playerType[$type]:\n";
print "  $email\n";
print "  score: $score ($won wins, $lost losses)\n";
}

# close socket
close(S);

# done
exit 0;

