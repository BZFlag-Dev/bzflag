#!/usr/bin/perl
#
# bzflag
# Copyright (c) 1993 - 2007 Tim Riker
#
# This package is free software;  you can redistribute it and/or
# modify it under the terms of the license found in the file
# named COPYING that should have accompanied this file.
#
# THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

=pod

=head1 NAME

bzfquery.pl - Contact a bzflag server and print the game status

=head1 SYNOPSIS

B<bzfquery.pl> I<servername> [I<port>]

=head1 DESCRIPTION

Generate a report to standard output describing the status of a bzflag game.
The report includes a player count, team listings, score, and the flags
controlling game option and state.

=head1 SEE ALSO

L<bzflag(6)>, L<bzadmin(6)>

=cut

use Socket;

# get arguments:  server [port]
($servername,$port) = @ARGV;
# handle :port in first arg
($servername,$port) = split(":", $servername) if ($servername =~ /:/);
$port = 5154 unless $port;

# some socket defines
$sockaddr = 'S n a4 x8';

# port to port number
($name,$aliases,$proto) = getprotobyname('tcp');
($name,$aliases,$port)  = getservbyname($port,'tcp') unless $port =~ /^\d+$/;

# get server address
($name,$aliases,$type,$len,$serveraddr) = gethostbyname($servername);
$server = pack($sockaddr, AF_INET, $port, $serveraddr);

# connect
die $! unless socket(S, AF_INET, SOCK_STREAM, $proto);
die $! unless connect(S, $server);

# don't buffer
select(S); $| = 1; select(STDOUT);

# get hello
die $! unless sysread(S, $buffer, 9) == 9;

# parse reply
($magic,$protocol,$id) = unpack("a4 a4 C", $buffer);

# quit if version isn't valid
die "not a bzflag server" if ($magic ne "BZFS");
die "incompatible version" if ($protocol ne "0026");

# quit if rejected
die "rejected by server" if ($id == 255);

# send game request
print S pack("n2", 0, 0x7167);

my $nbytes = sysread(S, $buffer, 46);
if ($nbytes == 12) {  # if MsgGameTime rxed ... ignore it
  my ($len, $code) = unpack("n2", $buffer);
  $nbytes = sysread(S, $buffer, 46);
}
if ($nbytes != 46) {
  die "Error: $nbytes bytes, expecting 46: $^E\n";
}

($len,$code,$style,$maxPlayers,$maxShots,
	$rogueSize,$redSize,$greenSize,$blueSize,$purpleSize,$obsSize,
	$rogueMax,$redMax,$greenMax,$blueMax,$purpleMax,$obsMax,
	$shakeWins,$shakeTimeout,
	$maxPlayerScore,$maxTeamScore,$maxTime,$timeElapsed) = unpack("n23", $buffer);
die $! unless $code == 0x7167;

# print info
print "style:";
print " CTF" if $style & 0x0001;
print " flags" if $style & 0x0002;
print " jumping" if $style & 0x0008;
print " inertia" if $style & 0x0010;
print " ricochet" if $style & 0x0020;
print " shaking" if $style & 0x0040;
print " antidote" if $style & 0x0080;
print " handicap" if $style & 0x0100;
print " rabbit-hunt" if $style & 0x0200;
print "\n";
print "maxPlayers: $maxPlayers\nmaxShots: $maxShots\n";
print "team sizes: $rogueSize $redSize $greenSize $blueSize $purpleSize $obsSize" .
	" (rogue red green blue purple observer)\n";
print "max sizes:  $rogueMax $redMax $greenMax $blueMax $purpleMax $obsMax\n";
if ($style & 0x0040) {
  print "wins to shake bad flag: $shakeWins\n";
  print "time to shake bad flag: " . $shakeTimeout / 10 . "\n";
}
print "max player score: $maxPlayerScore\n";
print "max team score: $maxTeamScore\n";
print "max time: " . $maxTime / 10 . "\n";

print "time elapsed: " . $timeElapsed / 10 . "\n\n";

# send players request
print S pack("n2", 0, 0x7170);

# get number of teams and players we'll be receiving
die $! unless sysread(S, $buffer, 8) == 8;
($len,$code,$numTotalTeams,$numPlayers) = unpack("n4", $buffer);
die $! unless $code == 0x7170;

# get the teams
# TimRiker: MsgTeamUpdate has numTotalTeams but this is how many we will get
die $! unless sysread(S, $buffer, 5) == 5;

($len,$code,$numTeams) = unpack("n n C", $buffer);
die $! unless $code == 0x7475;
@teamName = ("Rogue", "Red", "Green", "Blue", "Purple", "Observer", "Rabbit");
for (1..$numTeams) {
 die $! unless sysread(S, $buffer, 8) == 8;
 ($team,$size,$won,$lost) = unpack("n4", $buffer);
 $score = $won - $lost;
 print "$teamName[$team] team: $size players, score: $score ($won wins, $lost losses)\n";
}
print "\n";

# get the players
@playerType = ("tank", "observer", "robot tank");
for (1..$numPlayers) {
 # one MsgAddPlayer per player
 $bytesRead = sysread(S, $buffer, 175);
 while ($bytesRead != 175 && $bytesRead != 0){
  $bytesRead += sysread(S, $buffer, 175-$bytesRead)
 }
 if ($bytesRead == undef || $bytesRead < 175){ die $!; }

 ($len,$code,$pID,$type,$team,$won,$lost,$tks,$sign,$email) =
					unpack("n2Cn5A32A128", $buffer);
 die $! unless $code == 0x6170;
 $score = $won - $lost;
 print "player $sign ($teamName[$team] team) is a $playerType[$type]:\n";
 print "  score: $score ($won wins, $lost losses)\n";
 print "  $email\n";
}

# close socket
close(S);

# done
exit 0;

# Local Variables: ***
# mode:Perl ***
# tab-width: 8 ***
# c-basic-offset: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8
