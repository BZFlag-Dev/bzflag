<?php

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

# Connect to the server database.
$link = mysql_connect($dbhost, $dbuname, $dbpass) 
     or die("Could not connect: " . mysql_error());
if (!mysql_select_db($dbname)) {
  mysql_create_db($dbname) or die("Could not create db: " . mysql_error());
}
$result = mysql_query("SELECT * FROM servers");

# If the servers table does not exist, create it.
if (!$result) {
  mysql_query("CREATE TABLE servers " .
              "(nameport varchar(20), " .
              " build varchar(10), " .
              " version varchar(40), " .
              " gameinfo varchar(80), " .
              " ipaddr varchar(20), " .
              " title varchar(80), " .
              " lastmod varchar(10))")
    or die ("Could not create table: " . mysql_error());
}
# remove all inactive servers from the table
$timeout = 1800;    # timeout in seconds
$staletime = time() - $timeout;
mysql_query("DELETE FROM servers WHERE lastmod < $staletime")
     or die("Could not drop old items" . mysql_error());

header("Content-type: text/plain");
# Do stuff based on what the 'action' is...
#
#  -- LIST --
# Same as LIST in the old bzfls
if (!array_key_exists("action", $_GET) || $action == "LIST" ) {
  $result = mysql_query("SELECT nameport,version,gameinfo,ipaddr,title "
			. " FROM servers")
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
  $fp = fsockopen ($servname, $servport, $errno, $errstring, 30);
  if (!$fp) {
    print "failed to connect\n";
    return;
  }
  fclose ($fp);

  $curtime = time();

  $result = mysql_query("SELECT * FROM servers " .
			"WHERE nameport = '$nameport'") 
    or die ("Invalid query: ". mysql_error());
  $count = mysql_num_rows($result);
  if (!$count) {
# Server does not already exist in DB so insert into DB
    $result = mysql_query("INSERT INTO servers "
			  . "(nameport, build, version, gameinfo, ipaddr,"
                          . " title, lastmod) VALUES "
			  . "('$nameport', '$build', '$version',"
			  . " '$gameinfo', '$servip', '$title', $curtime)")
      or die ("Invalid query: ". mysql_error());
  } else {
# Server exists already, so update the table entry
# ASSUMPTION: only the 'lastmod' column of table needs updating since all
# else should remain the same as before
    $result = mysql_query("UPDATE servers "
			  . "SET gameinfo = '$gameinfo', lastmod = $curtime "
			  . "WHERE nameport = '$nameport'")
      or die ("Invalid query: ". mysql_error());
  }
  print "ADD complete\n";
} elseif ($action == "REMOVE") {
#  -- REMOVE --
# Server requests to be removed from the DB.
  $result = mysql_query("DELETE FROM servers WHERE nameport = '$nameport'")
    or die ("Invalid query: ". mysql_error());
} else {
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
