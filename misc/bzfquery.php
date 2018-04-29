<?php

// bzfquery.php
// based on bzflist.php
//
// original by D. John <g33k@despammed.com>
// php native code by Tim Riker <Tim@Rikers.org>
// updated by blast007 <blast007@users.sourceforge.net>
//
// Copyright (c) 1993-2018 Tim Riker
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


////////////////////////////////////
//  THIS IS THE FUNCTION YOU RUN  //
/////////////////////////////////////////////////////////
//
//  Run this function like this:
//    $data = bzfquery("someserver.com:5154");
//  where 'someserver.com' is the server address and 5154 is the server port.
//  This will return an array that contains a list of server settings, players,
//  player scores, and team scores.

define("MsgQueryGame", 0x7167);			// 'qg'
define("MsgQueryPlayers", 0x7170);		// 'qp'
define("MsgTeamUpdate", 0x7475);		// 'tu'
define("MsgAddPlayer", 0x6170);			// 'ap'

$GLOBALS['debug'] = false;

function readpacket(&$fp) {
  $loop = 0;
  $data = '';
  while (strlen($data) < 4 && $loop < 8) {
    $data .= fread($fp, 4 - strlen($data));
    $loop++;
  }

  if (strlen($data) != 4) return false;

  $return = unpack("nlen/ncode", $data);

  $loop = 0;
  $data = '';
  while (strlen($data) < $return['len'] && $loop < 64) {
    $data .= fread($fp, $return['len'] - strlen($data));
    $loop++;
  }
  $return['data'] = $data;
  return $return;
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

  fwrite($fp, "BZFLAG\r\n\r\n");

  $buffer=fread($fp, 9);
  //var_dump($buffer);
  if (strlen($buffer) != 9) {
    echo "not a bzflag server";
    return $server;
  }
  # parse reply
  $server += unpack("a4magic/a4protocol/Cid", $buffer);
  //var_dump($server);
  if ($server['magic'] != "BZFS") {
    echo "not a bzflag server\n";
    fclose($fp);
    return $server;
  }

  if ($server['protocol'] != '0221') {
    echo "incompatible version\n";
    fclose($fp);
    return $server;
  }

  # MsgQueryGame + MsgQueryPlayers
  $request = pack("n2", 0, 0x7167);
  $request .= pack("n2", 0, 0x7170);
  //var_dump($request);
  fwrite($fp, $request);

  $loop = 0;

  $have = Array();
  $have['QueryGame'] = false;
  $have['QueryPlayers'] = false;
  $have['TeamUpdate'] = false;
  $have['AllAddPlayer'] = false;

  while (in_array(false, $have) && $loop < 64) {
    $loop++;
    $packet = readpacket($fp);

    if ($GLOBALS['debug']) {
      echo "Length: " . $packet['len'] . "\n";
      echo "Code: " . $packet['code'] . " (" . dechex($packet['code']) . ") [" . chr(hexdec(substr(dechex($packet['code']), 0, 2))) . chr(hexdec(substr(dechex($packet['code']), 2, 2)))  . "]\n";
      echo "Data: " . $packet['data'] . "\n\n";
    }

    switch ($packet['code']) {
      case MsgQueryGame:
        $server += unpack("ngameStyle/ngameOptions/nmaxPlayers/nmaxShots/nrogueSize/nredSize/ngreenSize/nblueSize/npurpleSize/nobserverSize/nrogueMax/nredMax/ngreenMax/nblueMax/npurpleMax/nobserverMax/nshakeWins/nshakeTimeout/nmaxPlayerScore/nmaxTeamScore/nmaxTime/ntimeElapsed", $packet['data']);
        $have['QueryGame'] = true;
        break;
      case MsgQueryPlayers:
        $server += unpack("nnumTotalTeams/nnumPlayers", $packet['data']);
        unset($server['numTotalTeams']);
        $have['QueryPlayers'] = true;
        if ($server['numPlayers'] == 0) $have['AllAddPlayer'] = true;
        break;
      case MsgTeamUpdate:
        $server += unpack("CnumTeams", $packet['data']);
        $packet['data'] = substr($packet['data'], 1);
        for ( $team = 0; $team < $server['numTeams']; $team++ ) {
          $server['team'][$team] = unpack("nnum/nsize/nwon/nlost", $packet['data']);
          $packet['data'] = substr($packet['data'], 8);
        }
        $have['TeamUpdate'] = true;
        break;
      case MsgAddPlayer:
        $player = unpack("Cid/ntype/nteam/nwon/nlost/ntks/a32sign/a128motto", $packet['data']);
        $server['player'][ $player['id'] ] = $player;
        if (sizeof($server['player']) >= $server['numPlayers']) $have['AllAddPlayer'] = true;
        break;
    }
  }

  fclose($fp);
  return $server;
}

function bzfdump ($server) {
  if ($server['protocol'] != '0221')
    return;

  $styles = Array('TeamFFA', 'ClassicCTF', 'OpenFFA', 'RabbitChase');

  echo $server['host'] . ":" . $server['port'] . " (" . $server['ip'] . ")\n";
  echo "gameStyle: ".$styles[$server['gameStyle']]."\n";
  echo "gameOptions:";	# must mirror enum GameOptions in global.h
  if ($server['gameOptions'] & 0x0002) echo " flags";
  if ($server['gameOptions'] & 0x0008) echo " jumping";
  if ($server['gameOptions'] & 0x0010) echo " inertia";
  if ($server['gameOptions'] & 0x0020) echo " ricochet";
  if ($server['gameOptions'] & 0x0040) echo " shaking";
  if ($server['gameOptions'] & 0x0080) echo " antidote";
  if ($server['gameOptions'] & 0x0100) echo " handicap";
  if ($server['gameOptions'] & 0x0400) echo " no-team-kills";
  echo "\n";
  echo "maxPlayers: " . $server['maxPlayers'] . "\n";
  echo "maxShots: " . $server['maxShots'] . "\n";
  echo "team sizes: " . $server['rogueSize'] . " " .
      $server['redSize'] . " " . $server['greenSize'] . " " .
      $server['blueSize'] . " " . $server['purpleSize'] . " " . $server['observerSize'] .
      " (rogue red green blue purple observer)\n";
  echo  "max sizes:  " . $server['rogueMax'] . " " .
      $server['redMax'] . " " . $server['greenMax'] . " " .
      $server['blueMax'] . " " . $server['purpleMax'] .  " " . $server['observerMax'] . "\n";
  if ($server['gameOptions'] & 0x0040) {
    echo "wins to shake bad flag: " . $server['shakeWins'] . "\n";
    echo "time to shake bad flag: " . $server['shakeTimeout'] / 10 . "\n";
  }
  echo "max player score: " . $server['maxPlayerScore'] . "\n";
  echo "max team score: " . $server['maxTeamScore'] . "\n";
  echo "max time: " . $server['maxTime'] . " seconds\n";
  $teamName = array(0=>"Rogue", 1=>"Red", 2=>"Green", 3=>"Blue", 4=>"Purple", 5=>"Observer", 6=>"Rabbit");
  for ( $team = 0; $team < $server['numTeams']; $team++ ) {
    echo $teamName[$team] . " team: "
    . $server['team'][$team]['size'] . " players, "
    . "score: " . ($server['team'][$team]['won'] - $server['team'][$team]['lost'])
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
    echo "  " . $server['player'][$player]['motto'] . "\n";
  }
}



# Local Variables: ***
# mode: php ***
# tab-width: 8 ***
# c-basic-offset: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8

?>
