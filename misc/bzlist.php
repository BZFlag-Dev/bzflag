<?php

// bzlist.php
//
// original by D. John <g33k@despammed.com>
// php native code by Tim Riker <Tim@Rikers.org>
//
// Copyright (c) 1993 - 2006 Tim Riker
//
// This package is free software;  you can redistribute it and/or
// modify it under the terms of the license found in the file
// named COPYING that should have accompanied this file.
//
// THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// This is a simple script that reports current public servers
// and creates links to server stats.

$listserver = 'http://my.BZFlag.org/db/?action=LIST';

$title="BZFlag Server List";

if ($_GET["hostport"]) {
  $title .= " for " .$_GET["hostport"];
}

function bzfquery1910 ($server,$fp) {
  # MsgQueryGame + MsgQueryPlayers
  $request = pack("n2", 0, 0x7167);
  $request .= pack("n2", 0, 0x7170);
  //var_dump($request);
  fwrite($fp, $request);
  $buffer=fread($fp, 40);
  //var_dump($buffer);
  $server += unpack("nlen/ncode/nstyle/nmaxPlayers/nmaxShots/nrogueSize/nredSize/ngreenSize/nblueSize/npurpleSize/nrogueMax/nredMax/ngreenMax/nblueMax/npurpleMax/nshakeWins/nshakeTimeout/nmaxPlayerScore/nmaxTeamScore/nmaxTime", $buffer);

  # MsgQueryPlayers reply
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
  fclose($fp);
  return $server;
}

function bzfquery ($hostport) {
  list($server['host'], $server['port']) = split(":", $hostport, 2);
  $protocol = 'tcp';
  $get_prot = getprotobyname($protocol);
  if ($get_prot == -1) {
     // if nothing found, returns -1
     echo 'Invalid Protocol';
     return $server;
  }
  if (!$server['port']) {
    $server['port'] = 5154;
  } elseif (!ctype_digit($server['port'])) {
    $server['port'] = getservbyname($server['port'], $protocol);
  }
  $server['ip'] = gethostbyname($server['host']);
  $fp = fsockopen($server['host'], $server['port'], $errno, $errstr, 5);
  if (!$fp) {
    echo "$errstr ($errno)\n";
    return $server;
  }
  $buffer=fread($fp, 9);
  //var_dump($buffer);
  # parse reply
  $server += unpack("a4magic/a4protocol/Cid", $buffer);
  //var_dump($server);
  if ($server['magic'] != "BZFS") {
    echo "not a bzflag server\n";
    fclose($fp);
    return $server;
  }
  switch ($server['protocol']) {
  case "1910":
  case "0011":
   return bzfquery1910($server,$fp);
   break;
  default:
    echo "incompatible version\n";
    fclose($fp);
    return $server;
  }
}

function bzfdump ($server) {
  echo $server['host'] . ":" . $server['port'] . " (" . $server['ip'] . ")\n";
  echo "style:";
  if ($server['style'] & 0x0001) echo " CTF";
  if ($server['style'] & 0x0002) echo " flags";
  if ($server['style'] & 0x0008) echo " jumping";
  if ($server['style'] & 0x0010) echo " inertia";
  if ($server['style'] & 0x0020) echo " ricochet";
  if ($server['style'] & 0x0040) echo " shaking";
  if ($server['style'] & 0x0080) echo " antidote";
  if ($server['style'] & 0x0100) echo " handicap";
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
  $teamName = array(0=>"Rogue", 1=>"Red", 2=>"Green", 3=>"Blue", 4=>"Purple", 5=>"Observer", 6=>"Rabbit");
  for ( $team = 0; $team < $server['numTeams']; $team++ ) {
    echo $teamName[$team] . " team: "
	. $server['team'][$team]['size'] . " players, "
	. "score: " . $server['team'][$team]['score']
        . " (" . $server['team'][$team]['won'] . " wins, "
	. $server['team'][$team]['lost'] . " losses)\n";
  }
  echo "\n";
  $playerType = array(0=>"tank", 1=>"observer", 2=>"robot tank");
  for ( $player = 0; $player < $server['numPlayers']; $player++ ) {
    echo "player " . $server['player'][$player]['sign']
	. " (" . $teamName[$server['player'][$player]['team']]
	. " team) is a " . $playerType[$server['player'][$player]['type']] . ":\n";
    echo "  score: " . ( $server['player'][$player]['won'] - $server['player'][$player]['lost'] )
	. " (" . $server['player'][$player]['won']
	. " wins, " . $server['player'][$player]['lost'] . " losses)\n";
    echo "  " . $server['player'][$player]['email'] . "\n";
  }
  //var_dump($server);
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
      <td>
	<pre><? bzfdump(bzfquery($hostport)); echo "\n"; ?></pre>
	<!-- <pre><? #system("bzfquery.pl $hostport"); ?></pre> -->
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
      echo "<td>$LINK&nbsp;</td>";
      echo "<td>$array[3]&nbsp;</td>";
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
