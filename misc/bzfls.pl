#!/usr/bin/perl
# CGI bzfls
# Copyright (c) 1993 - 2004 Tim Riker
#
# This package is free software;  you can redistribute it and/or
# modify it under the terms of the license found in the file
# named COPYING that should have accompanied this file.
#
# THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.

use strict;
use CGI qw(:standard);
use DBI;
use Socket;

### CGI parameters ###

# Common to all
my $action   = param("action");

# For ADD
my $nameport = param("nameport");
my $build    = param("build");
my $version  = param("version");
my $gameinfo = param("gameinfo");
my $title    = param("title");
$title =~ s/'/''/g;

###############################################################################

# Connect to the server database.
my $dbpath = "bzflag.sqlite";
my $dbexist = stat $dbpath;
my $servdb = DBI->connect ("dbi:SQLite:$dbpath", "", "", {RaiseError => 1, AutoCommit => 1});

# If the servers table doesn't exist, create it.
if ( not $dbexist ) {
  $servdb->do (
      "CREATE TABLE servers ( " .
      "nameport varchar(60) NOT NULL, ".
      "build varchar(20), ".
      "version varchar(9) NOT NULL, ".
      "gameinfo varchar(73) NOT NULL, ".
      "ipaddr varchar(17) NOT NULL, ".
      "title varchar(80), ".
      "lastmod INT NOT NULL DEFAULT '0', ".
      "PRIMARY KEY (nameport))"
  );
}
# If the table already exists, then remove all inactive servers from the table
else {
  my $timeout = 1800;    # timeout in seconds
  my $staletime = time - $timeout;
  $servdb->do("DELETE FROM servers WHERE lastmod < $staletime");
}

print header(-type=>'text/plain');

###############################################################################
# Do stuff based on what the 'action' is...
#
#  -- LIST --
# Same as LIST in the old bzfls
if ( not defined $action or $action eq "LIST" ) {
  my $all = $servdb->selectall_arrayref
      ("SELECT nameport,version,gameinfo,ipaddr,title FROM servers");
  foreach my $row (@$all) {
    my $line = join ' ', @$row;
    print "$line\n";
  }
}
#  -- ADD --
# Server either requests to be added to DB, or to issue a keep-alive so that it
# does not get dropped due to a timeout...
elsif ( $action eq "ADD" ) {
  # Filter out badly formatted or buggy versions
  print "trying ADD $nameport $version $gameinfo $title\n";
  exit unless (
    $version =~ /^BZFS/
# and $version ne "BZFS1906"
  );

  my $curtime = time;

  # Server does not already exist in DB so try connect and insert into DB
  if ( not defined $servdb->selectrow_array
      ("SELECT nameport FROM servers WHERE nameport = '$nameport'") ) {
    # Test to see whether nameport is valid by attempting to establish a
    # connection to it
    my ($servname, $servport) = split /:/, $nameport;
    $servport = 5154 unless defined $servport;
    my $servip32 = inet_aton ($servname);
    exit unless defined $servip32;
    my $servip = inet_ntoa ($servip32);
    my $sin = sockaddr_in ($servport, inet_aton ($servname));
    socket(SH, AF_INET, SOCK_STREAM, getprotobyname('tcp')) or die $!;
    if (!connect(SH, $sin)) {
      print "failed to connect\n";
      die $!;
    }
    close SH;

    $servdb->do(
	"INSERT INTO servers VALUES (
	'$nameport',
	'$build',
	'$version',
	'$gameinfo',
	'$servip',
	'$title',
	$curtime
       )"
    );
  }
  # Server exists already, so update the table entry
  # ASSUMPTION: only the 'lastmod' column of table needs updating since all
  # else should remain the same as before
  else {
    # don't change nameport or servip
    $servdb->do(
	"UPDATE servers SET " .
	"build = '$build', " .
	"version = '$version', " .
	"gameinfo = '$gameinfo', " .
	"title = '$title', " .
	"lastmod = $curtime " .
	"WHERE nameport = '$nameport'");
  }
  print "ADD complete\n";
}
#  -- REMOVE --
# Server requests to be removed from the DB.
elsif ( $action eq "REMOVE" ) {
  $servdb->do("DELETE FROM servers WHERE nameport = '$nameport'");
}
# Unknown command ....
else {
  print "Unknown command: '$action'\n";
}

# Local Variables: ***
# mode:Perl ***
# tab-width: 8 ***
# c-basic-offset: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8
