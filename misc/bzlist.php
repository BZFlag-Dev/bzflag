<?php

// bzlist.php
//
// original by D. John <g33k@despammed.com>
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
//
// This is a simple script that reports current public servers
// and creates links to server stats via bzfquery.pl.


$listserver = 'http://db.bzflag.org/db/?action=LIST';
$bzfquery = './bzfquery.pl';

$title="BZFlag Server List";

if ($_GET["hostport"]) {
  $title .= " for " .$_GET["hostport"];
}

function query ($hostport) {
  list($host, $port) = split(":", $hostport, 2);
  $protocol = 'tcp';
  $get_prot = getprotobyname($protocol);
  if ($get_prot == -1) {
     // if nothing found, returns -1
     echo 'Invalid Protocol';
     return;
  }
  if (!ctype_digit($port)) {
    $port = getservbyname($port, 'tcp');
  }
  $ip = gethostbyname($host);
  echo "$host:$port:$ip";
  $fp = fsockopen($host, $port, $errno, $errstr, 5);
  if (!$fp) {
    echo "$errstr ($errno)<br>\n";
    return;
  }
  $buffer=fread($fp, 9);
  //var_dump($buffer);
  # parse reply
  $server = unpack("a4magic/a4protocol/Cid", $buffer);
  //var_dump($server);
  $magic = $server['magic'];
  $protocol = $server['protocol'];
  $id = $server['id'];
  echo "$magic $protocol $id\n";
  if ($magic != "BZFS") {
    echo "not a bzflag server";
    fclose($fp);
    return;
  }
  if ($protocol != "1910") {
    echo "incompatible version";
    fclose($fp);
    return;
  }
  # MsgQueryGame
  $request = pack("n2", 0, 0x7167);
  //var_dump($request);
  fwrite($fp, $request);
  $buffer=fread($fp, 40);
  //var_dump($buffer);
  $server += unpack("nlen/ncode/nstyle/nmaxPlayers/nmaxShots/nrogueSize/nredSize/ngreenSize/nblueSize/npurpleSize/nrogueMax/nredMax/ngreenMax/nblueMax/npurpleMax/nshakeWins/nshakeTimeout/nmaxPlayerScore/nmaxTeamScore/nmaxTime", $buffer);

  # MsgQueryPlayers
  $request = pack("n2", 0, 0x7170);
  //var_dump($request);
  fwrite($fp, $request);
  $buffer=fread($fp, 8);
  //var_dump(unpack("c8", $buffer));
  $server += unpack("nlen/ncode/nnumTotalTeams/nnumPlayers", $buffer);

  $buffer=fread($fp, 5);
  $server += unpack("nlen/ncode/CnumTeams", $buffer);
  //var_dump(unpack("c5", $buffer));

  for ( $team = 0; $team < $server['numTeams']; $team++ ) {
    $buffer=fread($fp, 8);
    $server['team'][$team] = unpack("nnum/nsize/nwon/nlost", $buffer);
  }

  for ( $player = 0; $player < $server['numPlayers']; $player++ ) {
    $buffer='';
    # handle packet fragmentation - untested
    while(strlen($buffer) < 175) {
      $buffer .= fread($fp, 175 - strlen($buffer));
      //echo strlen($buffer) . "\n";
    }
    $server['player'][$player] = unpack("nlen/ncode/Cid/ntype/nteam/nwon/nlost/ntks/a32sign/a128email", $buffer);
  }
  var_dump($server);
  fclose($fp);
  return $server;
}

function dump ($server) {
  echo "style:";
  if ($server['style'] & 0x0001) echo " CTF";
  if ($server['style'] & 0x0002) echo " flags";
  if ($server['style'] & 0x0004) echo " rogues";
  if ($server['style'] & 0x0008) echo " jumping";
  if ($server['style'] & 0x0010) echo " inertia";
  if ($server['style'] & 0x0020) echo " ricochet";
  if ($server['style'] & 0x0040) echo " shaking";
  if ($server['style'] & 0x0080) echo " antidote";
  if ($server['style'] & 0x0100) echo " time-sync";
  if ($server['style'] & 0x0200) echo " rabbit-hunt";
  echo "\n";
  echo "maxPlayers: " . $server['maxPlayers'] . "\n";
  echo "maxShots: " . $server['maxShots'] . "\n";
  echo "team sizes: " . $server['rogueSize'] . " " .
      $server['redSize'] . " " . $server['greenSize'] . " " .
      $server['blueSize'] . " " . $server['purpleSize'] .
      " (rogue red green blue purple)\n";
  echo  "max sizes:  " . $server['rogueMax'] . " " .
      $server['redMax'] . " " . $server['greenMax'] . " " .
      $server['blueMax'] . " " . $server['purpleMax'] . "\n";
  if ($server['style'] & 0x0040) {
    echo "wins to shake bad flag: " . $server['shakeWins'] . "\n";
    echo "time to shake bad flag: " . $server['shakeTimeout'] / 10 . "\n";
  }
  echo "max player score: " . $server['maxPlayerScore'] . "\n";
  echo "max team score: " . $server['maxTeamScore'] . "\n";
  echo "max time: " . $maxTime / 10 . "\n";

  var_dump($server);
  return;
}

?><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<title><? echo $title; ?></title>
</head>
<body>
<?
if($_GET["hostport"]) {
  ?>
  <table border=0 cellpadding=0 cellspacing=0>
    <tr>
      <td><? echo "$_GET[hostport] Stats:"; ?></td>
    </tr>
    <tr><td>&nbsp;</td></tr>
    <tr>
      <td>
	<pre><? dump(query($hostport)); ?></pre>
	<pre><? system("$bzfquery $hostport"); ?></pre>
      </td>
    </tr>
  </table>
  <?
}

$fp = fopen("$listserver", "r");
if ($fp) {
?>
<table border=0 cellpadding=0 cellspacing=0>
  <tr>
    <td width=300>Server Name</td>
    <td width=125>Server Address</td>
    <td>Server Description (short)</td>
  </tr>
  <!-- <tr><td colspan=3>&nbsp;</td></tr> -->
<?
  while (!feof($fp)) {
    $buffer = fgets($fp, 1024);
    $array = preg_split('/\s+/', $buffer);
    $count = count($array);
    $LINK = "<a href=\"$_SERVER[PHP_SELF]?hostport=$array[0]\">$array[0]</a>";
    if (($array[0])) {
      echo "<tr>";
      echo "<td>$LINK</td>";
      echo "<td>$array[3]</td>";
      unset($array[0], $array[1], $array[2], $array[3]);
      foreach ( $array as $line ) {
	$description .= "$line ";
      }
      //$bzdesc = substr("$description", 0, 50);
      //echo "<td>$bzdesc</td>";
      echo "<td>$description</td>";
      unset($description);
      echo "</tr>\n";
    }
  }
  echo "</table>\n";
  fclose ($fp);
} ?>

<p><a href="http://validator.w3.org/check?uri=referer"><img border="0" src="http://www.w3.org/Icons/valid-html401" alt="Valid HTML 4.01!" height="31" width="88"></a></p>
</body>
</html>

<?

# Local Variables: ***
# mode:php ***
# tab-width: 8 ***
# c-basic-offset: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8

?>
