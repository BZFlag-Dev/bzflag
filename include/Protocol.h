/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Communication protocol constants
 */

#ifndef	BZF_PROTOCOL_H
#define	BZF_PROTOCOL_H

#include "common.h"

// well known service port number for bzflag server
const unsigned int	ServerPort = 5154;

// port for udp broadcasts used to find servers on local lan
const unsigned int	BroadcastPort = 5154;

// URL for default list server
const char* const	DefaultListServerURL = "http://my.BZFlag.org/db/";
const char* const	DefaultMasterBanURL = "http://bzflag.org/master-bans.txt";
const char* const	DefaultMOTDServer = "http://bzflag.org/motd.php";

// maximum size of any message (including header and length fields)
const unsigned int	MaxPacketLen = 1024;

// the banned tag
const char* const	BanRefusalString = "REFUSED:";

// player attributes for the MsgPlayerInfo message
enum PlayerAttribute {
  IsRegistered = 1 << 0,
  IsVerified   = 1 << 1,
  IsAdmin      = 1 << 2
};

// null message code -- should never be sent
const uint16_t		MsgNull = 0x0000;

// server message codes
const uint16_t		MsgAccept = 0x6163;				// 'ac'
const uint16_t		MsgAdminInfo = 0x6169;			// 'ai'
const uint16_t		MsgAlive = 0x616c;				// 'al'
const uint16_t		MsgAllow = 0x696f;				// 'ao'
const uint16_t		MsgAddPlayer = 0x6170;			// 'ap'
const uint16_t		MsgAllowSpawn = 0x6173;			// 'as'
const uint16_t		MsgAutoPilot = 0x6175;			// 'au'
const uint16_t		MsgCapBits = 0x6362;			// 'cb'
const uint16_t		MsgCaptureFlag = 0x6366;		// 'cf'
const uint16_t		MsgCollide = 0x636f;	    // 'co'
const uint16_t		MsgCustomSound = 0x6373;		// 'cs'
const uint16_t		MsgCacheURL = 0x6375;			// 'cu'
const uint16_t		MsgDropFlag = 0x6466;			// 'df'
const uint16_t		MsgEnter = 0x656e;				// 'en'
const uint16_t		MsgExit = 0x6578;				// 'ex'
const uint16_t		MsgFlagType = 0x6674;			// 'ft'
const uint16_t		MsgFlagUpdate = 0x6675;			// 'fu'
const uint16_t		MsgFetchResources = 0x6672;		// 'fr'
const uint16_t		MsgGrabFlag = 0x6766;			// 'gf'
const uint16_t		MsgGMUpdate = 0x676d;			// 'gm'
const uint16_t		MsgGetWorld = 0x6777;			// 'gw'
const uint16_t		MsgGameSettings = 0x6773;		// 'gs'
const uint16_t		MsgGameTime = 0x6774;			// 'gt'
const uint16_t		MsgHandicap = 0x6863;			// 'hc'
const uint16_t		MsgHit = 0x6869;				// 'hi'
const uint16_t		MsgKilled = 0x6b6c;				// 'kl'
const uint16_t		MsgLagState = 0x6c73;			// 'ls'
const uint16_t		MsgLimboMessage = 0x6c6d;			// 'lm'
const uint16_t		MsgMessage = 0x6d67;			// 'mg'
const uint16_t		MsgNewPlayer = 0x6e70;			// 'np'
const uint16_t		MsgNearFlag = 0x4e66;			// 'Nf'
const uint16_t		MsgNewRabbit = 0x6e52;			// 'nR'
const uint16_t		MsgNegotiateFlags = 0x6e66;		// 'nf'
const uint16_t		MsgPause = 0x7061;				// 'pa'
const uint16_t		MsgPlayerInfo = 0x7062;			// 'pb'
const uint16_t		MsgPlayerUpdate = 0x7075;		// 'pu'
const uint16_t		MsgPlayerUpdateSmall = 0x7073;	// 'ps'
const uint16_t		MsgQueryGame = 0x7167;			// 'qg'
const uint16_t		MsgQueryPlayers = 0x7170;		// 'qp'
const uint16_t		MsgReject = 0x726a;				// 'rj'
const uint16_t		MsgRemovePlayer = 0x7270;		// 'rp'
const uint16_t		MsgReplayReset = 0x7272;		// 'rr'
const uint16_t		MsgShotBegin = 0x7362;			// 'sb'
const uint16_t		MsgWShotBegin = 0x7762;			// 'wb'
const uint16_t		MsgWhatTimeIsIt = 0x7774;		// 'wt'
const uint16_t		MsgScore = 0x7363;				// 'sc'
const uint16_t		MsgScoreOver = 0x736f;			// 'so'
const uint16_t		MsgShotEnd = 0x7365;			// 'se'
const uint16_t		MsgSuperKill = 0x736b;			// 'sk'
const uint16_t		MsgSetShot = 0x7373;			// 'ss'
const uint16_t		MsgSetTeam = 0x7374;			// 'st'
const uint16_t		MsgSetVar = 0x7376;			// 'sv'
const uint16_t		MsgTangibilityUpdate = 0x746e;		// 'tn'
const uint16_t		MsgTangibilityReset = 0x7472;		// 'tr'
const uint16_t		MsgTimeUpdate = 0x746f;			// 'to'
const uint16_t		MsgTeleport = 0x7470;			// 'tp'
const uint16_t		MsgTransferFlag = 0x7466;		// 'tf'
const uint16_t		MsgTeamUpdate = 0x7475;			// 'tu'
const uint16_t		MsgWantWHash = 0x7768;			// 'wh'
const uint16_t		MsgWantSettings = 0x7773;		// 'ws'
const uint16_t		MsgPortalAdd = 0x5061;			// 'Pa'
const uint16_t		MsgPortalRemove = 0x5072;		// 'Pr'
const uint16_t		MsgPortalUpdate = 0x5075;		// 'Pu'

// world database codes
const uint16_t		WorldCodeHeader = 0x6865;		// 'he'
const uint16_t		WorldCodeBase = 0x6261;			// 'ba'
const uint16_t		WorldCodeBox = 0x6278;			// 'bx'
const uint16_t		WorldCodeEnd = 0x6564;			// 'ed'
const uint16_t		WorldCodeLink = 0x6c6e;			// 'ln'
const uint16_t		WorldCodePyramid = 0x7079;		// 'py'
const uint16_t		WorldCodeMesh = 0x6D65;			// 'me'
const uint16_t		WorldCodeArc = 0x6172;			// 'ar'
const uint16_t		WorldCodeCone = 0x636e;			// 'cn'
const uint16_t		WorldCodeSphere = 0x7370;		// 'sp'
const uint16_t		WorldCodeTetra = 0x7468;		// 'th'
const uint16_t		WorldCodeTeleporter = 0x7465;		// 'te'
const uint16_t		WorldCodeWall = 0x776c;			// 'wl'
const uint16_t		WorldCodeWeapon = 0x7765;		// 'we'
const uint16_t		WorldCodeZone = 0x7A6e;			// 'zn'
const uint16_t		WorldCodeGroup = 0x6772;		// 'gr'
const uint16_t		WorldCodeGroupDefStart = 0x6473;	// 'ds'
const uint16_t		WorldCodeGroupDefEnd = 0x6465;		// 'de'

// world database sizes
const uint16_t		WorldSettingsSize = 18;
const uint16_t		WorldCodeHeaderSize = 10;
const uint16_t		WorldCodeBaseSize = 31;
const uint16_t		WorldCodeWallSize = 24;
const uint16_t		WorldCodeBoxSize = 29;
const uint16_t		WorldCodeEndSize = 0;
const uint16_t		WorldCodePyramidSize = 29;
const uint16_t		WorldCodeMeshSize = 0xA5;  // dummy value, sizes are variable
const uint16_t		WorldCodeArcSize = 85;
const uint16_t		WorldCodeConeSize = 65;
const uint16_t		WorldCodeSphereSize = 53;
const uint16_t		WorldCodeTetraSize = 66;
const uint16_t		WorldCodeTeleporterSize = 34;
const uint16_t		WorldCodeLinkSize = 4;
const uint16_t		WorldCodeWeaponSize = 24;  // basic size, not including lists
const uint16_t		WorldCodeZoneSize = 34;    // basic size, not including lists

// ping packet sizes, codes and structure
const uint16_t		MsgPingCodeReply = 0x0303;
const uint16_t		MsgPingCodeRequest = 0x0404;

const uint16_t		MsgEchoRequest = 0x6572;	// 'er'
const uint16_t		MsgEchoResponse = 0x6570; // 'ep'

// rejection codes
const uint16_t		RejectBadRequest = 0x0000;
const uint16_t		RejectBadTeam = 0x0001;
const uint16_t		RejectBadType = 0x0002;
const uint16_t		RejectUNUSED = 0x0003;
const uint16_t		RejectTeamFull = 0x0004;
const uint16_t		RejectServerFull = 0x0005;
const uint16_t		RejectBadCallsign = 0x0006;
const uint16_t		RejectRepeatCallsign = 0x0007;
const uint16_t		RejectRejoinWaitTime = 0x0008;
const uint16_t		RejectIPBanned = 0x0009;
const uint16_t		RejectHostBanned = 0x000A;
const uint16_t		RejectIDBanned = 0x000B;

// sound type codes
const uint16_t		LocalCustomSound = 0x0001;

enum BlowedUpReason {
	GotKilledMsg,
	GotShot,
	GotRunOver,
	GotCaptured,
	GenocideEffect,
	SelfDestruct,
	WaterDeath,
	PhysicsDriverDeath,
	LastReason
};

// request for additional UDP link
const uint16_t		MsgUDPLinkRequest = 0x6f66;		// 'of'
const uint16_t		MsgUDPLinkEstablished = 0x6f67;		// 'og'

// server control message
const uint16_t		MsgServerControl = 0x6f69;		// 'oi'

// lag ping sent by server to client and reply from client
const uint16_t		MsgLagPing = 0x7069;			// 'pi'

/* server communication protocol:
  --> incoming messages (to server)
  <-- outgoing messages to single player
  <== outgoing messages to all players

player to server messages:
  MsgEnter		player is joining game
			--> id, type, team, name
			<-- MsgReject (if rejected)
			<-- MsgAccept (if accepted)
			if accepted, new player is sent (following MsgAccept):
			<-- MsgTeamUpdate (one per team)
			<-- MsgFlagUpdate (one per existing flag)
			<-- MsgAddPlayer (one per already joined player)
			add, finally, sent to all:
			<== MsgAddPlayer (player being accepted)
  MsgExit		player is signing off
			--> /id/
			<== MsgRemovePlayer
  MsgGetWorld		request for playing field database
			--> bytes read so far
			<-- MsgGetWorld
  MsgQueryGame		request for game state
			<-- MsgQueryGame
  MsgQueryPlayers	request for player list
			<-- MsgQueryPlayers
  MsgAlive		player says he's coming alive
			--> /id,
			<== MsgAlive
  MsgKilled		player says he's been killed
			--> /id,/ killer-id, reason, killer-shot-id
			<== MsgKilled
  MsgNewRabbit		player is relinquishing rabbitship
  MsgGrabFlag		player wants to grab flag
			--> /id,/ flag
			<== MsgGrabFlag
  MsgDropFlag		player wants to drop flag
			--> /id,/ position
			<== MsgDropFlag
			<== MsgFlagUpdate
  MsgCaptureFlag	player captured flag
			--> /id,/ team (team flag was taken to)
			<== MsgCaptureFlag
			<== MsgFlagUpdate
  MsgSetVar		<== count/[name/value]*
  MsgShotBegin		player has fired a shot
			--> FiringInfo
			<== MsgShotBegin
  MsgShotEnd		shot has terminated
			--> shooter id, shot number, reason
			<== MsgShotEnd
  MsgTeleport		player has teleported
			--> /id,/ from-teleporter, to-teleporter
			<== MsgTeleport
  MsgMessage		player is sending a message
			--> /id,/ target-id/team-id, message string
			<== MsgMessage
  MsgWantWHash		(player wants md5 of world file
			-->
  MsgNegotiateFlags     -->flagCount/[flagabbv]
  MsgPause		-->true or false
  MsgCollide	    player has collided with another player
			--> id, collidee-id, position
			<== MsgAllow if sending player should freeze


server to player messages:
  MsgSuperKill		player must disconnect from server
			<== <none>
  MsgTimeUpdate		game time left, if == 0 player is dead and can't restart
			<== time (left, in seconds)
  MsgScoreOver		score limit reached, player is dead and can't restart
			<== id (winner), team (winner)
  MsgAccept		player request is accepted
			<== <none>
  MsgReject		player request is rejected
			<== <none>
  MsgAddPlayer		notification of new tank in game
			<== id, type, team, name
  MsgRemovePlayer	player has exited the server
			<== id
  MsgAdminInfo		update of players' IP addresses
			only sent to players with the PLAYERLIST permission.
			<-- count, [chunklen, id, bitfield, address]*
  MsgPlayerInfo		update of players status
			<-- count, [id, bitfield]*
  MsgFlagUpdate		update of flag info
			<== count, [flag, flag-info]*
  MsgTeamUpdate		update of team info
			<== teamcount, [team, team-info]
  MsgGetWorld		chunk of world database
			<-- bytes left, next 256 byte chunk of world database
  MsgAlive		player is alive
			<== id, position, forward-vector
  MsgKilled		player is dead
			<== id (victim id), killer-id, reason, killer-shot-id
  MsgGrabFlag		notification that flag is grabbed
			<== id (grabber), flag, flag-info
  MsgDropFlag		notification that flag is in air
			<== id (dropper), flag, flag-info
  MsgCaptureFlag	notification that flag has been captured
			<== id (capturer), flag, team
  MsgShotBegin		some player has fired a shot
			<== FiringInfo
  MsgShotEnd		shot has expired
			<== id (shooter id), shot number, reason
  MsgScore		player score has changed
			<== num-scores [id (player id), wins, losses, tks]*n
  MsgTeleport		player has teleported
			<== id, from-teleporter, to-teleporter
  MsgMessage		message to players
			<== from-id, to-id/team-id, message string
  MsgQueryGame		game status
  MsgQueryPlayers	list of players
  MsgWantWHash		md5 digest of world file
			<== temp|perm, digest
  MsgNegotiateFlags	<== flagCount/[flagabbv]
  MsgNewRabbit		a new rabbit has been anointed
			<== id
  MsgPause		<== id/true or false
  MsgAllow		<== id, movement (bool), shooting (bool)
*/
/**
 * Base class of all protocol messages.
 */
class BZProtocolMsg
{
public:
  /// Destructor
  ///
  /// Although this class doesn't allocate the memory, free it here so
  /// no one else has to think about it.
  virtual ~BZProtocolMsg();

  /// Return the total size of the packet
  size_t size() const;

  /// TBD if these need public exposure
  int code() const { return msgCode; }
  int len() const { return msgLen; }

  /// construct a network protocol object
  void* pack();

protected:
  /// Construct for an explicit code (before sending)
  BZProtocolMsg(uint16_t code_, uint16_t len_);
  /// Construct from a memory buffer (after receiving)
  ///
  /// @param buf_ a reference to the buffer pointer. Note that it is
  /// advanced past the arguments extracted here
  BZProtocolMsg(void*& buf_);

  virtual void* doPack(void* buf_);

  uint16_t msgCode;
  uint16_t msgLen;

  void* buf;
};

class EchoRequest : public BZProtocolMsg
{
public:
  EchoRequest(unsigned char tag_);
  EchoRequest(void* buf_);
  unsigned char tag();

protected:
  void* doPack(void* buf_);

private:
  unsigned char msgTag;
};

class EchoResponse : public BZProtocolMsg
{
public:
  EchoResponse(unsigned char tag_);
  EchoResponse(void* buf_);

protected:
  void* doPack(void* buf_);

private:
  unsigned char msgTag;
};


#endif // BZF_PROTOCOL_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
