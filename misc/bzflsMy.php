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

// define dbhost/dbuname/dbpass/dbname here
include('serversettings.php');
// $dbhost  = "localhost";
// $dbname  = "bzflag";
// $dbuname = "bzflag";
// $dbpass  = "bzflag";

# Common to all
$action   = $_GET['action'];

# For bzfs
$nameport = $_GET['nameport'];

# For ADD
$build    = $_GET['build'];
$version  = $_GET['version']; # also on LIST
$gameinfo = $_GET['gameinfo'];
$title    = $_GET['title'];

# For players
$callsign = $_GET['callsign'];  # urlencoded
$email    = $_GET['email'];     # urlencoded
$password = $_GET['password'];  # urlencoded
$conntime = $_GET['conntime'];

# For karma
$target   = $_GET['target'];    # urlencoded
$karma    = $_GET['karma'];

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
  $remote_addr = $_SERVER['REMOTE_ADDR'];
  debug($debugFile, "Connection rejected from $remote_addr");
  die("Connection attempt rejected.  See #bzflag on irc.freenode.net");
}

if (!array_key_exists("action", $_GET)) {
  header("Content-type: text/html");
  die('<html>
<head>
<title>BZFlag db server</title>
</head>
<body>
  <h1>BZFlag db server</h1>
  <p>
  <form action="" method="get">
    action:<input type="text" name="action" value="LIST" size="80"><br>
    actions: LIST<br>
    version:<input type="text" name="version" size="80"><br>
    actions: ADD REMOVE<br>
    nameport:<input type="text" name="nameport" size="80"><br>
    build:<input type="text" name="build" size="80"><br>
    gameinfo:<input type="text" name="gameinfo" size="80"><br>
    title:<input type="text" name="title" size="80"><br>
    actions:  REGISTER CONFIRM AUTH JOIN QUIT<br>
    callsign:<input type="text" name="callsign" size="80"><br>
    password:<input type="text" name="password" size="80"><br>
    email:<input type="text" name="email" size="80"><br>
    conntime:<input type="text" name="conntime" size="80"><br>
    actions: SETKARMA GETKARMA<br>
    target:<input type="text" name="target" size="80"><br>
    karma:<input type="text" name="karma" size="80"><br>
    <br>
    <input type="submit" value="Post entry">
    <input type="reset" value="Clear form">
</form>
</p>
</body>
</html>');
}
 
header("Content-type: text/plain");
debug($debugFile, "Connecting to the database");

# Connect to the server database persistently.
$link = mysql_pconnect($dbhost, $dbuname, $dbpass)
     or die("Could not connect: " . mysql_error());
if (!mysql_select_db($dbname)) {
  debug($debugFile, "Database did not exist, creating a new one");

  mysql_create_db($dbname) or die("Could not create db: " . mysql_error());
}

# FIXME something quicker to test for the table?
$result = mysql_query("SELECT * FROM servers", $link);

# If the servers table does not exist, create it.
if (!$result) {
  debug($debugFile, "Database table did not exist, creating a new one");

  mysql_query("CREATE TABLE servers " .
              "(nameport varchar(60) NOT NULL, " .
              " build varchar(60), " .
              " version varchar(9) NOT NULL, " .
              " gameinfo varchar(73) NOT NULL, " .
              " ipaddr varchar(17) NOT NULL, " .
              " title varchar(80), " .
              " lastmod INT NOT NULL DEFAULT '0', ".
	      " PRIMARY KEY (nameport))", $link)
    or die ("Could not create table: " . mysql_error());
}

# FIXME don't do this check on every LIST
# if the registered players table does not exist, create .
# FIXME something quicker to test for the table?
$result = mysql_query("SELECT * FROM players", $link);

if (!$result) {
  debug($debugFile, "Registered players table did not exist, creating a new one");
  
  mysql_query("CREATE TABLE players " .
              "(email varchar(64) NOT NULL, " .
              " callsign varchar(32) NOT NULL, " .
              " password varchar(16) NOT NULL, " .
              " karma TINYINT NOT NULL DEFAULT '0', " .
              " provisional BOOL NOT NULL DEFAULT '1', " .
              " assignments TEXT, " .
              " playtime BIGINT UNSIGNED NOT NULL DEFAULT '0', " .
              " lastmod INT NOT NULL, " .
              " randtext varchar(8), " .
              " PRIMARY KEY (email))", $link)
    or die ("Could not create players table: " . mysql_error());
}
    
debug($debugFile, "Deleting inactive servers from list");

# remove all inactive servers from the table
$timeout = 1800;    # timeout in seconds
$staletime = time() - $timeout;
mysql_query("DELETE FROM servers WHERE lastmod < $staletime", $link)
     or die("Could not drop old items" . mysql_error());

# remove all inactive registered players from the table
# FIXME this should not happen on every request
$timeout = 31536000; # timeout in seconds, 365 days
$staletime = time() - $timeout;
mysql_query("DELETE FROM players WHERE lastmod < $staletime", $link)
     or die ("Could not remove inactive players" . mysql_error());

# remove all players who have not confirmed registration
# FIXME this should not happen on every request
$timeout = 259200;  # timeout in seconds, 72h
$staletime = time() - $timeout;
mysql_query("DELETE FROM players WHERE lastmod < $staletime AND randtext != NULL", $link)
     or die ("Could not remove inactive players" . mysql_error());



# Do stuff based on what the 'action' is...
if ($action == "LIST" ) {
  #  -- LIST --
  # Same as LIST in the old bzfls
  debug($debugFile, "Fetching LIST");

  if ($version)
    $result = mysql_query("SELECT nameport,version,gameinfo,ipaddr,title "
			  . " FROM servers WHERE version = '$version'"
			  . " ORDER BY `nameport` ASC", $link)
      or die ("Invalid query: ". mysql_error());
  else
    $result = mysql_query("SELECT nameport,version,gameinfo,ipaddr,title "
			  . " FROM servers ORDER BY `nameport` ASC", $link)
      or die ("Invalid query: ". mysql_error());
  while (TRUE) {
    $row = mysql_fetch_row($result);
    if (!$row)
      break;
    $line = implode(" ", $row);
    print "$line\n";
  }
} elseif ($action == "ADD") {
  #  -- ADD --
  # Server either requests to be added to DB, or to issue a keep-alive so that it
  # does not get dropped due to a timeout...
  debug($debugFile, "Attempting to ADD $nameport $version $gameinfo $title");

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
    print("Requesting address is " . $_SERVER['REMOTE_ADDR']
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
  #  -- REMOVE --
  # Server requests to be removed from the DB.
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
    print("Requesting address is " . $_SERVER['REMOTE_ADDR']
                      . " while server is at " . $servip );
    die();
  }

  $result = mysql_query("DELETE FROM servers WHERE nameport = '$nameport'", $link)
    or die ("Invalid query: ". mysql_error());
} elseif ($action == "REGISTER") {
  #  -- REGISTER --
  # Registers a player onto the players database.
  # see if there is an existing entry
  $result = mysql_query("SELECT * FROM players WHERE email = '$email'", $link)
    or die ("Invalid query: ". mysql_error());
  if ( mysql_num_rows($result) > 0 ) {
    print("Registration FAILED: ");
    print("A player has already registered with the email address: $email");
    exit;
  }
  $result = mysql_query("SELECT * FROM players WHERE callsign = '$callsign'", $link)
    or die ("Invalid query: ". mysql_error());
  if ( mysql_num_rows($result) > 0 ) {
    print("Registration FAILED: ");
    print("A player has already registered with the callsign: $callsign");
    exit;
  }
  
  # no existing entry found - proceed to complete the registration
  $alphanum = "abcdefghijklmnopqrstuvwxyz0123456789";
  $randtext = "";
  srand(microtime());
  for ( $i = 0; $i < 8; $i++ )
    $randtext .= $alphanum{rand(0,35)};
  # FIXME remove `` etc from email
  $to = urldecode($email);
  mail($to, "BZFlag player registration",
       "You have just registered a BZFlag player account with\n" .
       "    callsign: $callsign\n" .
       "    password: $password\n" .
       "To activate this account, please go to the following URL:\n\n" .
       "http://db.bzflag.org/bzflsMyTest.php?action=CONFIRM&email=$email&password=$randtext\n")
    or die ("Could not send mail");
  $result = mysql_query("INSERT INTO players "
                      . "(email, callsign, password, lastmod, randtext) VALUES "
                      . "('$email', '$callsign', '$password', '" . time() . "', "
                      . "'$randtext')", $link)
  or die ("Invalid query: ". mysql_error());
  print("Registration SUCCESSFUL: ");
  print("You will receive an email informing you on how to complete your account registration\n");
} elseif ($action == "CONFIRM") {
  #  -- CONFIRM --
  # Confirms a registration
  $result = mysql_query("SELECT randtext FROM players WHERE email='$email'", $link)
    or die ("Invalid query: " . mysql_error());
  $row = mysql_fetch_row($result);
  $randtext = $row[0];
  if ( $randtext == NULL ) {
    print("You have already confirmed your account registration");
  } else {
    if ( $password != $randtext ) {
      print("Failed to confirm registration since generated key did not match");
    } else {
      $result = mysql_query("UPDATE players SET randtext=NULL " .
                            "WHERE email='$email'", $link)
        or die ("Invalid query: " . mysql_error());
      print("Your account has been successfully activated");
    }
  }
} elseif ($action == "AUTH") {
  #  -- AUTH --
  # A player authenticates
  $result = mysql_query("SELECT password FROM players WHERE callsign='$callsign'", $link)
    or die ("Invalid query: " . mysql_error());
  $row = mysql_fetch_row($result);
  $pwd = $row[0];
  if ( $pwd != $password ) {
    print("Authentication FAILED: incorrect password");
  } else {
    setcookie("BZPlayerAuth", md5($pwd), time()+60);
    print("Authentication SUCCESSFUL");
  }
} elseif ($action == "JOIN") {
  #  -- JOIN --
  # Server to handle a joining player
  $result = mysql_query("SELECT password FROM players WHERE callsign='$callsign'", $link)
    or die ("Invalid query: " . mysql_error());
  $row = mysql_fetch_row($result);
  $pwd = $row[0];
  if ( md5($pwd) != $password )
    print ("OK");
  else
    print ("FAIL");
} elseif ($action == "QUIT") {
  #  -- QUIT --
  # Server to handle a disconnecting player
  $result = mysql_query("UPDATE players SET lastmod=" . time() .
                        ", playtime=playtime+$conntime WHERE callsign='$callsign'", $link)
    or die ("Invalid query: " . mysql_error());
} elseif ($action == "SETKARMA") {
  #  -- KARMA_ADJ --
  # Set's a player's karma
  $result = mysql_query("SELECT assignments FROM players WHERE callsign='$callsign' " .
                        "AND password='$password'", $link)
    or die ("Invalid query: " . mysql_error());
  if ( mysql_num_rows($result) == 0 ) {
    print ("Karma adjustment FAILED: incorrect callsign/password");
    exit;
  } else {
    $row = mysql_fetch_row($result);
    $assignments = $row[0];
    
    $result = mysql_query("SELECT * FROM players WHERE callsign='$target'", $link)
      or die ("Invalid query: " . mysql_error());
    if ( mysql_num_rows($result) == 0 ) {
      print ("Karma adjustment FAILED: target player is not registered");
      exit;
    } else {
      $indv_assignments = explode(",", $assignments);
      $found = 0;
      for ( $i = 0; $i < count($indv_assignments); $i++ ) {
        list($player, $kval) = explode("=", $indv_assignments[$i]);
        if ( $player == $target ) {
          $indv_assignments[$i] = "$target=$karma";
          $found = 1;
        }
      }
      if ( !found ) {
        $indv_assignments[$i] = "$target=$karma";
      }
      $assignments = implode(",", $indv_assignments);
      $result = mysql_query("UPDATE players SET assignments='$assignments' " .
                            "WHERE callsign='$callsign'", $link)
        or die ("Invalid query: " . mysql_error());
      print ("Karma adjustment SUCCESSFUL");
    }
  }
} elseif ($action == "GETKARMA") {
  #  -- KARMA_LIST --
  # Views the karma for a particular player
  $result = mysql_query("SELECT karma,provisional FROM players WHERE callsign='$callsign'", $link)
    or die ("Invalid query: " . mysql_error());
  $row = mysql_fetch_row($result);
  print_r($row);
} else {
  print "Unknown command: '$action'\n";
  # TODO dump the default form here but still close the database connection
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
