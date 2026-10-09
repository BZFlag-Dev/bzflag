<?php

declare(strict_types=1);

// bzfquery.php
// based on bzflist.php
//
// original by D. John <g33k@despammed.com>
// php native code by Tim Riker <Tim@Rikers.org>
// updated by blast007 <blast007@users.sourceforge.net>
//
// Copyright (c) 1993-2025 Tim Riker
//
// This package is free software;  you can redistribute it and/or
// modify it under the terms of the license found in the file
// named COPYING that should have accompanied this file.
//
// THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//

class BZFServerInfo
{
  public string $host;
  public int $port;
  public string $protocol;

  // MsgQueryGame results
  public int $gameStyle;
  public int $gameOptions;
  public int $maxPlayers;
  public int $maxShots;
  public int $rogueSize;
  public int $redSize;
  public int $greenSize;
  public int $blueSize;
  public int $purpleSize;
  public int $observerSize;
  public int $rogueMax;
  public int $redMax;
  public int $greenMax;
  public int $blueMax;
  public int $purpleMax;
  public int $observerMax;
  public int $shakeWins;
  public int $shakeTimeout;
  public int $maxPlayerScore;
  public int $maxTeamScore;
  public int $maxTime;
  public int $timeElapsed;

  // MsgQueryPlayers results
  public int $numPlayers;

  // MsgTeamUpdate results
  public int $numTeams;
  public array $teamInfo = [];

  // MsgAddPlayer results
  public array $playerInfo = [];
}

class BZFQueryException extends RuntimeException
{
}

class BZFQuery
{
  // Message code constants
  private const MsgQueryGame = 0x7167; //    qg
  private const MsgQueryPlayers = 0x7170; // qp
  private const MsgTeamUpdate = 0x7475; //   tu
  private const MsgAddPlayer = 0x6170; //    ap

  private static bool $isDebug = false;
  // Enable debugging for all BZFQuery instances
  public static function enableDebug(): void
  {
    self::$isDebug = true;
  }

  private $fp;
  private BZFServerInfo $info;

  public function __construct(string $host = 'localhost', int $port = 5154)
  {
    // Ensure a host was specified
    if (strlen($host) === 0) {
      throw new BZFQueryException("Server host cannot be empty.");
    }

    // Ensure the port is within range
    if ($port < 1 || $port > 65535) {
      throw new BZFQueryException("Server port must be between 1 and 65535.");
    }

    // Create a new BZFServerInfo object to store the information we query
    $this->info = new BZFServerInfo();

    // Store the passed in host and port
    $this->info->host = $host;
    $this->info->port = $port;


    // Attempt to open a connection to the server
    $this->fp = @fsockopen($host, $port, $errno, $errstr, 5);
    if (!$this->fp) {
      if (self::$isDebug) {
        throw new BZFQueryException("Cannot connect to $host:$port - $errstr ($errno)");
      } else {
        throw new BZFQueryException("Cannot connect to server.");
      }
    }

    // Tell the BZFlag server we are a BZFlag client
    fwrite($this->fp, "BZFLAG\r\n\r\n");

    // Attempt to read 9 bytes from the server which will contain the protocol and the player ID
    $buffer = @fread($this->fp, 9);
    if (strlen($buffer) != 9) {
      throw new BZFQueryException("Did not receive magic string from server.");
    }

    // Parse the magic string, protocol, and player ID
    $data = unpack("a4magic/a4protocol/Cid", $buffer);
    $magic = $data["magic"];
    $this->info->protocol = $data["protocol"];
    $playerID = $data["id"];

    // Require the BZFS magic string
    if ($magic !== "BZFS") {
      fclose($this->fp);
      throw new BZFQueryException("Not a BZFlag server.");
    }

    // 2.4.x
    if ($this->info->protocol == '0221') {
      $this->query0221();
    }
    // 2.5.x
    else if ($this->info->protocol == '0223') {
      $this->query0223();
    }
    else {
      fclose($this->fp);
      if (self::$isDebug) {
        throw new BZFQueryException("Unsupported protocol version ($this->info->protocol).");
      } else {
        throw new BZFQueryException("Unsupported protocol version.");
      }
    }

    fclose($this->fp);
  }

  public function getServerInfo(): BZFServerInfo
  {
    return $this->info;
  }

  private function readPacket(): array
  {
    // Try to read the 4 bytes for the packet header
    $loop = 0;
    $data = '';
    while (strlen($data) < 4 && $loop < 8) {
      $data .= fread($this->fp, 4 - strlen($data));
      $loop++;
    }

    // Failed to get the header
    if (strlen($data) != 4) {
      throw new BZFQueryException("Failed to read packet header.");
    }

    // Parse out the length and code
    $packet = unpack("nlen/ncode", $data);

    // Try to read the packet data payload
    $loop = 0;
    $packet['data'] = '';
    while (strlen($packet['data']) < $packet['len'] && $loop < 64) {
      $packet['data'] .= fread($this->fp, $packet['len'] - strlen($packet['data']));
      $loop++;
    }

    // If the payload length doesn't match, bail out
    if (strlen($packet['data']) != $packet['len']) {
      throw new BZFQueryException("Failed to read packet payload.");
    }

    return $packet;
  }

  private function query0221(): void
  {
    // Send MsgQueryGame and MsgQueryPlayers
    $request = pack("n2", 0, $this::MsgQueryGame);
    $request .= pack("n2", 0, $this::MsgQueryPlayers);
    fwrite($this->fp, $request);

    // Track if we have received all the necessary data
    $have = [];
    $have['QueryGame'] = false;
    $have['QueryPlayers'] = false;
    $have['TeamUpdate'] = false;
    $have['AllAddPlayer'] = false;

    // Try to receive all the data
    $loop = 0;
    while (in_array(false, $have, true) && $loop < 64) {
      $loop++;

      // Try to read a packet
      $packet = $this->readpacket();

      if (self::$isDebug && php_sapi_name() === "cli") {
        echo "Length: {$packet['len']}" . PHP_EOL;
        echo "Code: {$packet['code']} (" . dechex($packet['code']) . ") [" . chr(hexdec(substr(dechex($packet['code']), 0, 2))) . chr(hexdec(substr(dechex($packet['code']), 2, 2)))  . "]" . PHP_EOL;
        echo "Data: {$packet['data']}" . PHP_EOL . PHP_EOL;
      }

      switch ($packet['code']) {
        case $this::MsgQueryGame:
          // Parse the packet
          $data = unpack("ngameStyle/ngameOptions/nmaxPlayers/nmaxShots/nrogueSize/nredSize/ngreenSize/nblueSize/npurpleSize/nobserverSize/nrogueMax/nredMax/ngreenMax/nblueMax/npurpleMax/nobserverMax/nshakeWins/nshakeTimeout/nmaxPlayerScore/nmaxTeamScore/nmaxTime/ntimeElapsed", $packet['data']);
          // For each key, assign that value to the matching BZFSServerInfo field
          foreach ($data as $key => $value) {
            $this->info->$key = $value;
          }
          // Indicate that we have received the MsgQueryGame packet
          $have['QueryGame'] = true;
          break;
        case $this::MsgQueryPlayers:
          // Parse the packet
          $data = unpack("nnumTotalTeams/nnumPlayers", $packet['data']);
          // Store the numPlayers value
          $this->info->numPlayers = $data['numPlayers'];
          // Indicate that we have received the MsgQueryPlayers packet
          $have['QueryPlayers'] = true;
          // If the number of players is 0, we won't be receiving any MsgAddPlayer messages
          if ($this->info->numPlayers === 0) {
            $have['AllAddPlayer'] = true;
          }
          break;
        case $this::MsgTeamUpdate:
          // Parse the number of teams and strip off the first byte we read
          $data = unpack("CnumTeams", $packet['data']);
          $this->info->numTeams = $data['numTeams'];
          $packet['data'] = substr($packet['data'], 1);

          // Loop through each team
          for ($team = 0; $team < $this->info->numTeams; $team++) {
            // Parse the info for this team
            $data = unpack("nid/nsize/nwon/nlost", $packet['data']);

            // Rip out the team ID
            $id = $data['id'];
            unset($data['id']);

            // Store the team info
            $this->info->teamInfo[$id] = $data;

            // Step off the 8 bytes we read for this team
            $packet['data'] = substr($packet['data'], 8);
          }
          // Indicate that we have received the MsgTeamUpdate message
          $have['TeamUpdate'] = true;
          break;
        case $this::MsgAddPlayer:
          // Parse the packet
          $data = unpack("Cid/ntype/nteam/nwon/nlost/ntks/a32sign/a128motto", $packet['data']);
          // Rip out the player ID
          $id = $data['id'];
          unset($data['id']);

          // Store the player info
          $this->info->playerInfo[$id] = $data;

          // Indicate that we have received all the MsgAddPlayer messages
          if (count($this->info->playerInfo) >= $this->info->numPlayers) {
            $have['AllAddPlayer'] = true;
          }
          break;
      }
    }
  }

  private function query0223(): void
  {
    // Currently, this is the same as 2.4.x
    $this->query0221();
  }

  public function dump(): void
  {
    if (!isset($this->info->protocol)) {
      return;
    }
    if ($this->info->protocol !== '0221' && $this->info->protocol !== '0223') {
      return;
    }

    $styles = ['TeamFFA', 'ClassicCTF', 'OpenFFA', 'RabbitChase'];

    echo "server: {$this->info->host}:{$this->info->port}" . PHP_EOL;
    echo "gameStyle: {$styles[$this->info->gameStyle]}" . PHP_EOL;
    echo "gameOptions:";	# must mirror enum GameOptions in global.h
    if ($this->info->gameOptions & 0x0002) {
      echo " flags";
    }
    if ($this->info->gameOptions & 0x0008) {
      echo " jumping";
    }
    if ($this->info->gameOptions & 0x0010) {
      echo " inertia";
    }
    if ($this->info->gameOptions & 0x0020) {
      echo " ricochet";
    }
    if ($this->info->gameOptions & 0x0040) {
      echo " shaking";
    }
    if ($this->info->gameOptions & 0x0080) {
      echo " antidote";
    }
    if ($this->info->gameOptions & 0x0100) {
      echo " handicap";
    }
    if ($this->info->gameOptions & 0x0400) {
      echo " no-team-kills";
    }
    echo PHP_EOL;
    echo "maxPlayers: {$this->info->maxPlayers}" . PHP_EOL;
    echo "maxShots: {$this->info->maxShots}" . PHP_EOL;
    echo "team sizes (current / max):" . PHP_EOL;
    echo "  rogue:    {$this->info->rogueSize} / {$this->info->rogueMax}" . PHP_EOL;
    echo "  red:      {$this->info->redSize} / {$this->info->redMax}" . PHP_EOL;
    echo "  green:    {$this->info->greenSize} / {$this->info->greenMax}" . PHP_EOL;
    echo "  blue:     {$this->info->blueSize} / {$this->info->blueMax}" . PHP_EOL;
    echo "  purple:   {$this->info->purpleSize} / {$this->info->purpleMax}" . PHP_EOL;
    echo "  observer: {$this->info->observerSize} / {$this->info->observerMax}" . PHP_EOL;

    if ($this->info->gameOptions & 0x0040) {
      echo "wins to shake bad flag: {$this->info->shakeWins}" . PHP_EOL;
      $shakeTimeoutSeconds = $this->info->shakeTimeout / 10;
      echo "time to shake bad flag: $shakeTimeoutSeconds" . PHP_EOL;
    }
    echo "max player score: {$this->info->maxPlayerScore}" . PHP_EOL;
    echo "max team score: {$this->info->maxTeamScore}" . PHP_EOL;
    echo "max time: {$this->info->maxTime} seconds" . PHP_EOL;

    $teamName = [0 => "Rogue", 1 => "Red", 2 => "Green", 3 => "Blue", 4 => "Purple", 5 => "Observer", 6 => "Rabbit"];
    for ($team = 0; $team < $this->info->numTeams; $team++) {
      $score = $this->info->teamInfo[$team]['won'] - $this->info->teamInfo[$team]['lost'];
      echo "{$teamName[$team]} team: "
        . "{$this->info->teamInfo[$team]['size']} players, "
        . "score: {$score} "
        . "({$this->info->teamInfo[$team]['won']} wins, "
        . "{$this->info->teamInfo[$team]['lost']} losses)" . PHP_EOL;
    }
    echo PHP_EOL;
    $playerType = [0 => "tank", 1 => "observer", 2 => "robot tank"];

    foreach ($this->info->playerInfo as $playerID => $player) {
      $score = $player['won'] - $player['lost'];
      echo "player {$player['sign']} "
        . "({$player['team']} team) "
        . "is a {$playerType[$player['type']]}: "
        . "score: $score "
        . "({$player['won']} wins, "
        . "{$player['lost']} losses) "
        . "[{$player['motto']}]" . PHP_EOL;
    }
  }
}

# Handle CLI mode
if (php_sapi_name() === "cli") {
  $hostname = 'localhost';
  $port = 5154;

  $options = getopt('hd', ['help', 'debug', 'host:', 'port:']);
  if (array_key_exists('h', $options) || array_key_exists('help', $options)) {
    echo 'Usage: php bzfquery.php [-d] [--host hostname] [--port port]' . PHP_EOL . PHP_EOL;
    echo '-h, --help   Display this help.' . PHP_EOL;
    echo '--host ...   Specifies the hostname (defaults to localhost).' . PHP_EOL;
    echo '--port ...   Specifies the port (defaults to 5154).' . PHP_EOL;
    echo '-d, --debug  Enable debug output.' . PHP_EOL . PHP_EOL;
    echo 'Report bugs on https://github.com/BZFlag-Dev/bzflag/issues' . PHP_EOL;
    exit(0);
  }

  if (array_key_exists('d', $options) || array_key_exists('debug', $options)) {
    BZFQuery::enableDebug();
  }

  $host = 'localhost';
  $port = 5154;

  if (array_key_exists('host', $options)) {
    $host = $options['host'];
  }

  if (array_key_exists('port', $options)) {
    $port = (int)$options['port'];
  }

  try {
    $query = new BZFQuery($host, $port);
    $query->dump();
  } catch (BZFQueryException $e) {
    echo $e->getMessage() . PHP_EOL;
  }
}



# Local Variables: ***
# mode: php ***
# tab-width: 8 ***
# c-basic-offset: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8
