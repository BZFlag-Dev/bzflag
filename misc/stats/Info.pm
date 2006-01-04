# Info.pm
#
# Copyright (c) 2003 - 2006 Tucker McLean, Tim Riker.
#
# This package is free software;  you can redistribute it and/or
# modify it under the terms of the license found in the file
# named COPYING that should have accompanied this file.
#
# THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
###

package BZFlag::Info;

use 5.6.1;
use strict;
use warnings;

use LWP::UserAgent;
use Socket;

our $VERSION = '1.7.1';

sub new {
  my $self = { };
  bless $self, "BZFlag::Info";
  return $self;
}

sub serverlist(%) {
  my $self = shift;

  my %options;
  while (my @option = splice(@_, 0, 2)) {
    $options{$option[0]} = $option[1];
  }

  my $proxy = $options{Proxy};
  my $response;
  my $ua = new LWP::UserAgent;
  $ua->proxy('http', $proxy) if defined($proxy);

  $ua->timeout(5);

  my $req = HTTP::Request->new('GET', $self->listserver);
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
  $response->{totalservers} = $totalServers;
  $response->{totalplayers} = $totalPlayers;

  foreach my $key (sort {$servers{$b} <=> $servers{$a}} (keys(%servers))) {
#	if ($servers{$key} > 0) {
    $response->{servers}->{$key}->{numplayers} = $servers{$key};
#	}
  }

  return ($response);

}

sub queryserver(%) {
  my $self = shift;

  my %options;
  while (my @option = splice(@_, 0, 2)) {
    $options{$option[0]} = $option[1];
  }

  my $hostandport = $options{Server};
  my $timeout = $options{Timeout};

  #my @teamName = ("Rogue", "Red", "Green", "Blue", "Purple");
  my @teamName = ("X", "R", "G", "B", "P");
  my ($message, $server);
  my $response;
  my ($servername, $port) = split(/:/, $hostandport);
  $port = 5155 unless $port;

  # socket define
  my $sockaddr = 'S n a4 x8';

  # port to port number
  my ($name,$aliases,$proto) = getprotobyname('tcp');
  ($name,$aliases,$port)  = getservbyname($port,'tcp') unless $port =~ /^\d+$/;

  # get server address
  my ($type,$len,$serveraddr);
  ($name,$aliases,$type,$len,$serveraddr) = gethostbyname($servername);
  $server = pack($sockaddr, AF_INET, $port, $serveraddr);

  # connect
  unless (socket(S1, AF_INET, SOCK_STREAM, $proto)) {
    $self->{error} = 'errSocketError';
    return undef;
  }

  unless (connect(S1, $server)) {
    $self->{error} = "errCouldNotConnect: $servername:$port";
    return undef;
  }

  # don't buffer
  select(S1); $| = 1; select(STDOUT);

  # get hello
  my $buffer;
  unless (read(S1, $buffer, 10) == 10) {
    $self->{error} = 'errReadError';
    return undef;
  }

  # parse reply
  my ($magic,$major,$minor,$revision);
  ($magic,$major,$minor,$revision,$port) = unpack("a4 a1 a2 a1 n", $buffer);

  # quit if version isn't valid
  if ($magic ne "BZFS") {
    $self->{error} = 'errNotABzflagServer';
    return undef;
  }

  # try incompatible for BZFlag:Zero etc.
  if (($major < 1) or ($major == 1 && $minor < 7) or ($major == 1 && $minor == 7 && $revision eq "b")) {
    $self->{error} = 'errIncompatibleVersion';
    return undef;
  }

  # quit if rejected
  if ($port == 0) {
    $self->{error} = 'errRejectedByServer';
    return undef;
  }


  # reconnect on new port
  $server = pack($sockaddr, AF_INET, $port, $serveraddr);
  unless (socket(S, AF_INET, SOCK_STREAM, $proto)) {
    $self->{error} = 'errSocketErrorOnReconnect';
    return undef;
  }

  unless (connect(S, $server)) {
    $self->{error} = "errCouldNotReconnect: $servername:$port";
    return undef;
  }

  select(S); $| = 1; select(STDOUT);

  # close first socket
  close(S1);

  # send game request
  print S pack("n2", 0, 0x7167);

  # get reply
  unless (read(S, $buffer, 40) == 40) {
    $self->{error} = 'errServerReadError';
    return undef;
  }

  my ($infolen,$infocode,$style,$maxPlayers,$maxShots,
      $rogueSize,$redSize,$greenSize,$blueSize,$purpleSize,
      $rogueMax,$redMax,$greenMax,$blueMax,$purpleMax,
      $shakeWins,$shakeTimeout,
      $maxPlayerScore,$maxTeamScore,$maxTime) = unpack("n20", $buffer);
  unless ($infocode == 0x7167) {
    $self->{error} = 'errBadServerData';
    return undef;
  }

  # send players request
  print S pack("n2", 0, 0x7170);

  # get number of teams and players we'll be receiving
  unless (read(S, $buffer, 8) == 8) {
    $self->{error} = 'errCountReadError';
    return undef;
  }

  my ($countlen,$countcode,$numTeams,$numPlayers) = unpack("n4", $buffer);
  unless ($countcode == 0x7170) {
    $self->{error} = 'errBadCountData';
    return undef;
  }

  # get the teams
  for (1..$numTeams) {
    unless (read(S, $buffer, 14) == 14) {
      $self->{error} = 'errTeamReadError';
      return undef;
    }

    my ($teamlen,$teamcode,$team,$size,$aSize,$wins,$losses) = unpack("n7", $buffer);
    unless ($teamcode == 0x7475) {
      $self->{error} = 'errBadTeamData';
      return undef;
    }

    my $score = $wins - $losses;

    $response->{teams}->{$teamName[$team]}->{size}   = $size;
    $response->{teams}->{$teamName[$team]}->{score}  = $score;
    $response->{teams}->{$teamName[$team]}->{wins}   = $wins;
    $response->{teams}->{$teamName[$team]}->{losses} = $losses;

  }

  # get the players
  for (1..$numPlayers) {
    next if read(S, $buffer, 180) == 18;
    my ($playerlen,$playercode,$pAddr,$pPort,$pNum,$type,$team,$wins,$losses,$sign,$email) =
	unpack("n2Nn2 n4A32A128", $buffer);
    unless ($playercode == 0x6170) {
      $self->{error} = 'errBadPlayerData';
      return undef;
    }

    my $score = $wins - $losses;

    $response->{players}->{$sign}->{team}   = $teamName[$team];
    $response->{players}->{$sign}->{email}  = $email;
    $response->{players}->{$sign}->{score}  = $score;
    $response->{players}->{$sign}->{wins}   = $wins;
    $response->{players}->{$sign}->{losses} = $losses;
    $response->{players}->{$sign}->{ip}     = inet_ntoa(inet_aton($pAddr));

  }
  if ($numPlayers <= 1) {
    $self->{error} = 'errNoPlayers';
    return undef;
  }

  # close socket
  close(S);

  return $response;

}

sub geterror {
  my $self = shift;
  return $self->{error};
}

sub listserver {
  my $self = shift;
  my %options;
  while (my @option = splice(@_, 0, 2)) {
    $options{$option[0]} = $option[1];
  }

  my $ua = new LWP::UserAgent;
  $ua->proxy('http', $options{Proxy}) if defined($options{Proxy});
  $ua->timeout(5);

  my $req = HTTP::Request->new('GET', 'http://BZFlag.SourceForge.net/list-server.txt');
  my $res = $ua->request($req);
  my ($listserver) = split("\n",$res->content);
  $listserver =~ s/^bzflist/http/;

  return $listserver;
}

1;

__END__

=head1 NAME

BZFlag::Info - Extracts infomation about BZFlag servers and players

=head1 SYNOPSIS

use BZFlag::Info;

my $bzinfo = new BZFlag::Info;

my $serverlist = $bzinfo->serverlist;
my $serverlist = $bzinfo->serverlist(Proxy => 'host:port');

my $serverinfo = $bzinfo->queryserver(Server => 'host:port');


=head1 DESCRIPTION

C<BZFlag::Info> is a class for extracting information about BZFlag
clients and servers. Currently, 4 methods are implemented, C<new>,
C<serverlist>, C<queryserver>, and C<geterror>.

=head1 METHODS

=over 4

=item my $bzinfo = new BZFlag::Info;

C<new> constructs a new C<BZFlag::Info> object. It takes no arguments.

=item my $serverlist = $bzinfo->serverlist;

C<serverlist> retrieves the current list of public servers. Then
returns a data structure that would be displayed by C<Data::Dumper>
like this:

$VAR1 = {
  'totalservers' => 8,
  'totalplayers' => 42,
  'servers' => {
    'ducati.bzflag.org:5155' => {
      'numplayers' => 0
    },
    'quol.bzflag.org:8085' => {
      'numplayers' => 0
    },
    'bzflag.secretplace.us:5155' => {
      'numplayers' => 18
    },
    'lbdpc15.epfl.ch:5155' => {
      'numplayers' => 8
    },
    'q2.bzflag.org:8083' => {
      'numplayers' => 0
    },
    'ducati.bzflag.org:5156' => {
      'numplayers' => 7
    },
    'bzflag.servegame.com:5155' => {
      'numplayers' => 0
    },
    'bzflag.freedomlives.net:5155' => {
      'numplayers' => 9
    },
  }
};


It can also take one option, Proxy, where you can specify a proxy
server to handle the HTTP request.

=item my $serverinfo = $bzinfo->queryserver(Server => 'host:port');

C<queryserver> extracts information about players and teams from the
BZFlag server specified with the Server option. It returns a data
structure that would be displayed by C<Data::Dumper> like this:

$VAR1 = {
  'teams' => {
    'X' => {
      'losses' => 0,
      'wins' => 0,
      'score' => 0,
      'size' => 0
    },
    'P' => {
      'losses' => 0,
      'wins' => 7,
      'score' => 7,
      'size' => 1
    },
    'R' => {
      'losses' => 8,
      'wins' => 0,
      'score' => -8,
      'size' => 1
    },
    'G' => {
      'losses' => 0,
      'wins' => 0,
      'score' => 0,
      'size' => 0
    },
    'B' => {
      'losses' => 0,
      'wins' => 0,
      'score' => 0,
      'size' => 0
    }
  },
  'players' => {
    'xabner' => {
      'losses' => 8,
      'wins' => 0,
      'email' => '',
      'ip' => '123.123.123.123',
      'score' => -8,
      'team' => 'R'
    },
    'mackattack' => {
      'losses' => 0,
      'wins' => 7,
      'email' => 'user@hostname',
      'ip' => '123.123.123.123',
      'score' => 7,
      'team' => 'P'
    }
  }
};


X, R, G, B, and P stand for Rogue, Red, Green, Blue, and Purple,
respectively.

If there was an error retrieving information on a BZFlag server,
C<queryserver> will return undef, C<geterror> will return the error.

=back

=head1 BUGS

I have no idea, tell me if there are any.

=head1 AUTHOR

Tucker McLean, tuckerm@noodleroni.com

=head1 COPYRIGHT

Copyright (c) 2003 - 2006 Tucker McLean, Tim Riker.

This program is free software; you can redistribute it and/or modify
it under the same terms as Perl itself.

# Local Variables: ***
# mode: Perl ***
# tab-width: 8 ***
# c-basic-offset: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8
