/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Communication protocol constants
 */

#ifndef BZF_PROTOCOL_H
#define	BZF_PROTOCOL_H

#include "common.h"

// magic to identify a bzflag server.  first four characters are the
// bzflag magic.  the last four give the version number.  the first
// digit is the major release, the second and third digits are the
// minor release, and the last character is the revision.  The major
// number should increment when there've been sweeping changes in the
// whole program.  The minor release should increment for smaller
// changes that make the new version incompatible with older servers
// or clients.  The revision should be incremented for minor changes
// that don't cause incompatibility.
// adding new flags or changing the communication protocol require
// minor release number changes.  adding new graphics effects normally
// only require incrementing the revision number.

// well known service port number for bzflag server
const int		ServerPort = 5154;

// address and port for multicast communication between players
// this is the sgi-dog multicast address.
const char* const	BroadcastAddress = "224.0.1.2";
const int		BroadcastPort = 5154;

// URL for default list server
const char* const	DefaultListServerURL = "http://db.bzflag.org/db/";

// multicast ttl's
const int		DefaultTTL = 8;
const int		MaximumTTL = 255;

// maximum size of any message (including header and length fields)
const int		MaxPacketLen = 1024;

// player attributes for the MsgAdminInfo message
enum PlayerAttribute {
  IsRegistered = 1 << 0,
  IsIdentified = 1 << 1,
  IsAdmin      = 1 << 2
};

// null message code -- should never be sent
const uint16_t		MsgNull = 0x0000;

// server message codes
const uint16_t		MsgAccept = 0x6163;			// 'ac'
const uint16_t		MsgAdminInfo = 0x6169;			// 'ai'
const uint16_t		MsgAlive = 0x616c;			// 'al'
const uint16_t		MsgAddPlayer = 0x6170;			// 'ap'
const uint16_t		MsgAudio = 0x6175;			// 'au'
const uint16_t		MsgCaptureFlag = 0x6366;		// 'cf'
const uint16_t		MsgDropFlag = 0x6466;			// 'df'
const uint16_t		MsgEnter = 0x656e;			// 'en'
const uint16_t		MsgExit = 0x6578;			// 'ex'
const uint16_t		MsgFlagUpdate = 0x6675;			// 'fu'
const uint16_t		MsgGrabFlag = 0x6766;			// 'gf'
const uint16_t		MsgGMUpdate = 0x676d;			// 'gm'
const uint16_t		MsgGetWorld = 0x6777;			// 'gw'
const uint16_t		MsgKilled = 0x6b6c;			// 'kl'
const uint16_t		MsgMessage = 0x6d67;			// 'mg'
const uint16_t		MsgNewRabbit = 0x6e52;			// 'nR'
const uint16_t		MsgNegotiateFlags = 0x6e66;		// 'nf'
const uint16_t		MsgPause = 0x7061;			// 'pa'
const uint16_t		MsgPlayerUpdate = 0x7075;		// 'pu'
const uint16_t		MsgQueryGame = 0x7167;			// 'qg'
const uint16_t		MsgQueryPlayers = 0x7170;		// 'qp'
const uint16_t		MsgReject = 0x726a;			// 'rj'
const uint16_t		MsgRemovePlayer = 0x7270;		// 'rp'
const uint16_t		MsgShotBegin = 0x7362;			// 'sb'
const uint16_t		MsgScore = 0x7363;			// 'sc'
const uint16_t		MsgScoreOver = 0x736f;			// 'so'
const uint16_t		MsgShotEnd = 0x7365;			// 'se'
const uint16_t		MsgSuperKill = 0x736b;			// 'sk'
const uint16_t		MsgSetVar = 0x7376;			// 'sv'
const uint16_t		MsgTimeUpdate = 0x746f;			// 'to'
const uint16_t		MsgTeleport = 0x7470;			// 'tp'
const uint16_t		MsgTransferFlag = 0x7466;		// 'tf'
const uint16_t		MsgTeamUpdate = 0x7475;			// 'tu'
const uint16_t		MsgVersion = 0x7672;			// 've'
const uint16_t		MsgVideo = 0x7669;			// 'vi'
const uint16_t		MsgWantWHash = 0x7768;			// 'wh'

// world database codes
const uint16_t		WorldCodeHeader = 0x6865;		// 'he'
const uint16_t		WorldCodeBase = 0x6261;			// 'ba'
const uint16_t		WorldCodeBox = 0x6278;			// 'bx'
const uint16_t		WorldCodeEnd = 0x6564;			// 'ed'
const uint16_t		WorldCodeLink = 0x6c6e;			// 'ln'
const uint16_t		WorldCodePyramid = 0x7079;		// 'py'
const uint16_t		WorldCodeTeleporter = 0x7465;		// 'te'
const uint16_t		WorldCodeWall = 0x776c;			// 'wl'

// world database sizes
const uint16_t		WorldCodeHeaderSize = 30;
const uint16_t		WorldCodeEpochOffset = 4+26; // huge hack, someone fix!!
const uint16_t		WorldCodeBaseSize = 42;
const uint16_t		WorldCodeWallSize = 24;
const uint16_t		WorldCodeBoxSize = 29;
const uint16_t		WorldCodeEndSize = 0;
const uint16_t		WorldCodePyramidSize = 29;
const uint16_t		WorldCodeTeleporterSize = 33;
const uint16_t		WorldCodeLinkSize = 4;

// ping packet sizes, codes and structure
const uint16_t		MsgPingCodeReply = 0x0303;
const uint16_t		MsgPingCodeRequest = 0x0404;

// rejection codes
const uint16_t		RejectBadRequest = 0x0000;
const uint16_t		RejectBadTeam = 0x0001;
const uint16_t		RejectBadType = 0x0002;
const uint16_t		RejectBadEmail = 0x0003;
const uint16_t		RejectTeamFull = 0x0004;
const uint16_t		RejectServerFull = 0x0005;
const uint16_t		RejectBadCallsign = 0x0006;
const uint16_t		RejectRepeatCallsign = 0x0007;

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
			--> id, type, team, name, email address
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
			<== id, type, team, name, email
  MsgRemovePlayer	player has exited the server
			<== id
  MsgAdminInfo		update of players' IP addresses and some other 
                        properties, only sent to players with the PLAYERLIST
                        permission.
			<-- count, [chunklen, id, bitfield, address]*
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
*/

#endif // BZF_PROTOCOL_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

