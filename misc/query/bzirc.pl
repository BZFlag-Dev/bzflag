#!/usr/bin/perl
#
# bzirc.pl - A script that echoes messages from a bzflag server to an IRC
#            channel and from the channel to the server.
#
# Use it like this:
# bzirc.pl <IRCNICK> <IRCHOST>:<PORT> <CHANNEL> <BZADMINOPTS> <BZNICK>@<BZHOST>
#
# Inspired by ircbot.pl by detour@metalshell.com (http://www.metalshell.com).
#
# This script uses the POE and POE::Component::IRC modules.
# A simple way to get these is using the cpan module.
# perl -MCPAN -eshell
# cpan> install POE
# cpan> install POE::Component::IRC


#FIXME: Why not make this available as a bzfs command? something like /irc irc.freenode.net #bzflag Hello world!
use strict;
use IPC::Open2;
use POE;
use POE::Component::IRC;


# some global variables
my ($nick, $host, $port, $channel, $callsign, $bzopts,
    $bzadminpid, $bzin, $bzout, $c);
$nick = @ARGV[0];
$host = @ARGV[1];
$c = index($host, ':');
if ($c != -1) {
  $port = substr($host, $c + 1, length($host));
  $host = substr($host, 0, $c);
}
else {
  $port = 6667;
}
$channel = @ARGV[2];
$callsign = '@' . substr($ARGV[$#ARGV], 0, index($ARGV[$#ARGV], '@'));
$bzopts = join ' ', @ARGV[3..$#ARGV];
$bzadminpid;
$bzin;
$bzout;


# do some POE magic
POE::Component::IRC->new("irc_client");
POE::Session->new(_start     => \&irc_start,
		  irc_join   => \&irc_join,
		  irc_quit   => \&irc_quit,
		  irc_376    => \&irc_connect,
		  irc_public => \&irc_pub_msg,
		  bzreadable => \&readfrombz);


# a handler that is called once when this session is started
sub irc_start {
  my $kernel = $_[KERNEL];
  my $heap = $_[HEAP];
  my $session = $_[SESSION];

  $kernel->refcount_increment( $session->ID(), "bzirc");
  $kernel->post(irc_client=>register=> "all");
  $kernel->post(irc_client=>connect=>{  Nick     => $nick,
					Username => $nick,
					Ircname  => $nick,
					Server   => $host,
					Port     => $port,
				     });
}


# a handler that is called when we have connected to the IRC server and
# received the MOTD
sub irc_connect {
  my $kernel = $_[KERNEL];
  $kernel->post(irc_client=>join=>$channel);
}


# a handler that is called when someone joins a channel
sub irc_join {
  my $joiner = (split /!/, $_[ARG0])[0];
  my $joinchannel = $_[ARG1];
  my $kernel = $_[KERNEL];
  if ($joiner eq $nick && $joinchannel eq $channel) {
    print "Joined $channel, starting bzadmin...\n";
    $bzadminpid = open2($bzout, $bzin, "bzadmin -ui stdboth $bzopts");
    $kernel->select_read($bzout, 'bzreadable');
  }
}


# a handler that is called when someone quits IRC
sub irc_quit {
  my $quitter = $_[ARG0];
  my $reason = $_[ARG1];
  if ($quitter eq $nick) {
    print "Disconnected.\n";
  }
}


# a handler that is called when someone sends a message to the channel or to us
sub irc_pub_msg{
  my $talker = (split /!/, $_[ARG0])[0];
  my $talkchannel = $_[ARG1]->[0];
  my $msg = $_[ARG2];
  if ($talker ne $nick && $talkchannel eq $channel) {
    print "->bz: $talker: $msg\n";
    print $bzin "$talker: $msg\n";
  }
}


# a handler that is called when the bzadmin pipe is readable
sub readfrombz {
  my $msg = <$bzout>;
  my $kernel = $_[KERNEL];
  if (substr($msg, 0, length("    $callsign: ")) ne "    $callsign: ") {
    print "->irc: $msg";
    $kernel->post('irc_client'=>privmsg=>$channel,$msg);
  }
}


# start the POE kernel
$poe_kernel->run();

# Local Variables: ***
# mode:Perl ***
# tab-width: 8 ***
# c-basic-offset: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8
