<?php

// bzflsMy.php
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

$dbhost  = "localhost";
$dbuname = "bzflag";
$dbpass  = "";
$dbname  = "bzflag";

# Common to all
$action   = $_GET['action'];
$nameport = $_GET['nameport'];

# For ADD
$build    = $_GET['build'];
$version  = $_GET['version'];
$gameinfo = $_GET['gameinfo'];
$title    = $_GET['title'];

# where to send debug printing
$enableDebug	= 0;
$debugFile 	= "/dev/null";

# for banning.  provide key => value pairs where the key is an
# ip address. value is not used at present.
# FIXME this should be in an sql table with a remote admin interface
$banlist = array(
#  "68.109.43.46" => "knightmare.kicks-ass.net",
#  "127.0.0.1" => "localhost"
  "255.255.255.255" => "globalbroadcast"
);

# log function
if ($enableDebug) {
  function debug ($filename, $message) {
    $fp = fopen($filename, "a");
    if ($fp) {
      # output the message with a BSD-style timestamp
      fwrite($fp, date("D M j G:i:s T Y") . " " . $_SERVER['REMOTE_ADDR'] . " " . $message . "\n");
      fclose($fp);
    } else {
      print("Unable to write to to log file [$filename]");
    }
  }
} else {
  function debug ($filename, $message) {
  }
}

# ignore banned servers outright
if ($banlist[$_SERVER['REMOTE_ADDR']] != "") {
  # reject the connection attempt
  debug($debugFile, "Connection rejected from $_SERVER['REMOTE_ADDR]");
  die("Connection attempt rejected.  See #bzflag on irc.freenode.net");
}

debug($debugFile, "Connecting to the database");

# Connect to the server database persistently.
$link = mysql_pconnect($dbhost, $dbuname, $dbpass)
     or die("Could not connect: " . mysql_error());
if (!mysql_select_db($dbname)) {
  debug($debugFile, "Database did not exist, creating a new one");

  mysql_create_db($dbname) or die("Could not create db: " . mysql_error());
}
$result = mysql_query("SELECT * FROM servers", $link);

# If the servers table does not exist, create it.
if (!$result) {
  debug($debugFile, "Database table did not exist, creating a new one");

  mysql_query("CREATE TABLE servers " .
              "(nameport varchar(60) NOT NULL, " .
              " build varchar(20), " .
              " version varchar(9) NOT NULL, " .
              " gameinfo varchar(73) NOT NULL, " .
              " ipaddr varchar(17) NOT NULL, " .
              " title varchar(80), " .
              " lastmod INT NOT NULL DEFAULT '0', ".
	      " PRIMARY KEY (nameport))", $link)
    or die ("Could not create table: " . mysql_error());
}

debug($debugFile, "Deleting inactive servers from list");

# remove all inactive servers from the table
$timeout = 1800;    # timeout in seconds
$staletime = time() - $timeout;
mysql_query("DELETE FROM servers WHERE lastmod < $staletime", $link)
     or die("Could not drop old items" . mysql_error());


header("Content-type: text/plain");
# Do stuff based on what the 'action' is...
#
#  -- LIST --
# Same as LIST in the old bzfls
if (!array_key_exists("action", $_GET) || $action == "LIST" ) {
  debug($debugFile, "Fetching LIST");

  if ($version)
    $result = mysql_query("SELECT nameport,version,gameinfo,ipaddr,title "
			  . " FROM servers WHERE version = '$version'", $link)
      or die ("Invalid query: ". mysql_error());
  else
    $result = mysql_query("SELECT nameport,version,gameinfo,ipaddr,title "
			  . " FROM servers", $link)
      or die ("Invalid query: ". mysql_error());
  while (TRUE) {
    $row = mysql_fetch_row($result);
    if (!$row)
      break;
    $line = implode(" ", $row);
    print "$line\n";
  }
} elseif ($action == "ADD") {

  debug($debugFile, "Attempting to ADD $nameport $version $gameinfo $title");

#  -- ADD --
# Server either requests to be added to DB, or to issue a keep-alive so that it
# does not get dropped due to a timeout...
# Filter out badly formatted or buggy versions
  print "trying ADD $nameport $version $gameinfo $title\n";
  $pos = strpos($version, "BZFS");
  if ($pos === false || $pos > 0)
    return;
# Test to see whether nameport is valid by attempting to establish a
# connection to it
  $split = explode(":", $nameport);
  $servname = $split[0];
  if (array_key_exists(1, $split))
    $servport = $split[1];
  else
    $servport = 5154;
  $servip = gethostbyname($servname);

  if ($_SERVER['REMOTE_ADDR'] != $servip) {
    debug($debugFile, "Requesting address is " . $_SERVER['REMOTE_ADDR']
        . " while server is at " . $servip );
    die();
  }

  $fp = fsockopen ($servname, $servport, $errno, $errstring, 30);
  if (!$fp) {
    print "failed to connect\n";
    return;
  }
  fclose ($fp);

  $curtime = time();

  $result = mysql_query("SELECT * FROM servers " .
			"WHERE nameport = '$nameport'", $link)
    or die ("Invalid query: ". mysql_error());
  $count = mysql_num_rows($result);
  if (!$count) {
    debug($debugFile, "Server does not already exist in database -- adding");

    # Server does not already exist in DB so insert into DB
    $result = mysql_query("INSERT INTO servers "
			  . "(nameport, build, version, gameinfo, ipaddr,"
                          . " title, lastmod) VALUES "
			  . "('$nameport', '$build', '$version',"
			  . " '$gameinfo', '$servip', '$title', $curtime)", $link)
      or die ("Invalid query: ". mysql_error());
  } else {

    debug($debugFile, "Server already exists in database -- updating");

# Server exists already, so update the table entry
# ASSUMPTION: only the 'lastmod' column of table needs updating since all
# else should remain the same as before
    $result = mysql_query("UPDATE servers SET " .
			  "build = '$build', " .
			  "version = '$version', " .
			  "gameinfo = '$gameinfo', " .
			  "title = '$title', " .
			  "lastmod = $curtime " .
			  "WHERE nameport = '$nameport'", $link)
      or die ("Invalid query: ". mysql_error());
  }

  debug($debugFile, "ADD complete");

  print "ADD complete\n";
} elseif ($action == "REMOVE") {
  debug($debugFile, "REMOVE request from $nameport");

  $split = explode(":", $nameport);
  $servname = $split[0];
  if (array_key_exists(1, $split))
    $servport = $split[1];
  else
    $servport = 5154;
  $servip = gethostbyname($servname);
  if ($_SERVER['REMOTE_ADDR'] != $servip) {
    debug($debugFile, "Requesting address is " . $_SERVER['REMOTE_ADDR']
                      . " while server is at " . $servip );
    die();
  }

#  -- REMOVE --
# Server requests to be removed from the DB.
  $result = mysql_query("DELETE FROM servers WHERE nameport = '$nameport'", $link)
    or die ("Invalid query: ". mysql_error());

} else {
  print "Unknown command: '$action'\n";
}


# make sure the connection to mysql is severed
if ($link) {

  # for a transaction commit just in case
  debug($debugFile, "Commiting any pending transactions");
  mysql_query("COMMIT", $link);

  # debug($debugFile, "Closing link to database");

  # say bye bye (shouldn't need to ever really, especially for persistent..)
  #	mysql_close($link);
}

debug($debugFile, "End session");

# Local Variables: ***
# mode:php ***
# tab-width: 8 ***
# c-basic-offset: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8
?>
