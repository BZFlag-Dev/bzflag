<?php

// bzflsLite.php
//
// Copyright (c) 1993 - 2004 Tim Riker
//
// This package is free software;  you can redistribute it and/or
// modify it under the terms of the license found in the file
// named COPYING that should have accompanied this file.
//
// THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
// WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.

# Common to all
$action   = $_GET['action'];
$version  = $_GET['version'];

# For ADD
$nameport = $_GET['nameport'];
$build    = $_GET['build'];
$gameinfo = $_GET['gameinfo'];
$title    = $_GET['title'];
# $title =~ s/'/''/g;  to write in php'

if(!extension_loaded('sqlite')) {
	dl('sqlite.so');
}
###############################################################################
# Connect to the server database.
$dbpath  = "bzflag.sqlite";
$dbexist = is_writable($dbpath);
$servdb  = sqlite_open($dbpath, 0666, $sqliteerror);

# If the servers table does not exist, create it.
if (!$dbexist) {
  $result = sqlite_query($servdb,
			 "CREATE TABLE servers" .
			 "(nameport varchar(60) NOT NULL,".
			 " build varchar(20),".
			 " version varchar(9) NOT NULL,".
			 " gameinfo varchar(73) NOT NULL,".
			 " ipaddr varchar(17) NOT NULL,".
			 " title varchar(80),".
			 " lastmod INT NOT NULL DEFAULT '0',".
			 " PRIMARY KEY (nameport))");
}
# remove all inactive servers from the table
$timeout = 1800;    # timeout in seconds
$staletime = time() - $timeout;
$result = sqlite_query($servdb,
		       "DELETE FROM servers WHERE lastmod < $staletime");

header("Content-type: text/plain");
###############################################################################
# Do stuff based on what the 'action' is...
#
#  -- LIST --
# Same as LIST in the old bzfls
if (!array_key_exists("action", $_GET) || $action == "LIST" ) {
  if ($version)
    $result = sqlite_unbuffered_query($servdb,
				      "SELECT nameport,version,gameinfo,"
				      . "ipaddr,title FROM servers "
				      . "WHERE version = '$version'");
  else
    $result = sqlite_unbuffered_query($servdb,
				      "SELECT nameport,version,gameinfo,"
				      . "ipaddr,title FROM servers");
  while (TRUE) {
    $row = sqlite_fetch_array($result, SQLITE_ASSOC);
    if (!$row)
      break;
    $line = implode(" ", $row);
    print "$line\n";
  }
} elseif ($action == "ADD") {
#  -- ADD --
# Server either requests to be added to DB, or to issue a keep-alive so that it
# does not get dropped due to a timeout...
# Filter out badly formatted or buggy versions
  print "trying ADD $nameport $version $gameinfo $title\n";
  $pos = strpos($version, "BZFS");
  if ($pos === false || $pos > 0)
    return;

  $curtime = time();

  $result = sqlite_array_query($servdb,
			       "SELECT nameport FROM servers "
			       . "WHERE nameport = '$nameport'");
  $count = count($result);
  if (!$count) {
# Server does not already exist in DB so try connect and insert into DB
# Test to see whether nameport is valid by attempting to establish a
# connection to it
    $split = explode(":", $nameport);
    $servname = $split[0];
    if (array_key_exists(1, $split))
      $servport = $split[1];
    else
      $servport = 5154;
    $servip = gethostbyname($servname);
    $fp = fsockopen ($servname, $servport, $errno, $errstring, 30);
    if (!$fp) {
      print "failed to connect\n";
      return;
    }
    fclose ($fp);

    $result = sqlite_query($servdb,"INSERT INTO servers VALUES "
			   . "('$nameport', '$build', '$version',"
			   . " '$gameinfo', '$servip', '$title', $curtime)");
  } else {
# Server exists already, so update the table entry
# ASSUMPTION: only the 'lastmod' column of table needs updating since all
# else should remain the same as before
    $result = sqlite_query($servdb, "UPDATE servers SET " .
			   "build = '$build', version = '$version', " .
			   "gameinfo = '$gameinfo', title = '$title', " .
			   "lastmod = $curtime WHERE nameport = '$nameport'");
  }
  print "ADD complete\n";
} elseif ($action == "REMOVE") {
#  -- REMOVE --
# Server requests to be removed from the DB.
  $result = sqlite_query($servdb,
			 "DELETE FROM servers WHERE nameport = '$nameport'");
  print "Unknown command: '$action'\n";
}

# Local Variables: ***
# mode:php ***
# tab-width: 8 ***
# c-basic-offset: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8

?>
