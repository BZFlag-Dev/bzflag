/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Communication protocol constants
 */

#ifndef BZF_PROTOCOL_H
#define BZF_PROTOCOL_H

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

const char* const		ServerVersion = "BZFS108a";

// well known service port number for bzflag server
const int				ServerPort = 5155;

// URL for default list server
const char* const		DefaultListServerURL = "http://BZFlag.SourceForge.net/list-server.txt";

// maximum size of any message (including header and length fields)
const int				MaxPacketLen = 1024;

// null message code -- should never be sent
const uint16_t			MsgNull = 0x0000;

// unreliable message codes
const uint16_t			MsgPlayerUpdate = 0x7075;				// 'pu'
const uint16_t			MsgGMUpdate = 0x676d;					// 'gm'
const uint16_t			MsgAudio = 0x6175;						// 'au'
const uint16_t			MsgVideo = 0x7669;						// 'vi'

// server message codes
const uint16_t			MsgAccept = 0x6163;						// 'ac'
const uint16_t			MsgAlive = 0x616c;						// 'al'
const uint16_t			MsgAddPlayer = 0x6170;					// 'ap'
const uint16_t			MsgAcquireRadio = 0x6172;				// 'ar'
const uint16_t			MsgCaptureFlag = 0x6366;				// 'cf'
const uint16_t			MsgDropFlag = 0x6466;					// 'df'
const uint16_t			MsgEnter = 0x656e;						// 'en'
const uint16_t			MsgExit = 0x6578;						// 'ex'
const uint16_t			MsgFlagUpdate = 0x6675;					// 'fu'
const uint16_t			MsgGrabFlag = 0x6766;					// 'gf'
const uint16_t			MsgGetWorld = 0x6777;					// 'gw'
const uint16_t			MsgKilled = 0x6b6c;						// 'kl'
const uint16_t			MsgMessage = 0x6d67;					// 'mg'
const uint16_t			MsgQueryGame = 0x7167;					// 'qg'
const uint16_t			MsgQueryPlayers = 0x7170;				// 'qp'
const uint16_t			MsgReject = 0x726a;						// 'rj'
const uint16_t			MsgRemovePlayer = 0x7270;				// 'rp'
const uint16_t			MsgReleaseRadio = 0x7272;				// 'rr'
const uint16_t			MsgShotBegin = 0x7362;					// 'sb'
const uint16_t			MsgScore = 0x7363;						// 'sc'
const uint16_t			MsgScoreOver = 0x736f;					// 'so'
const uint16_t			MsgShotEnd = 0x7365;					// 'se'
const uint16_t			MsgSuperKill = 0x736b;					// 'sk'
const uint16_t			MsgTimeUpdate = 0x746f;					// 'to'
const uint16_t			MsgTeleport = 0x7470;					// 'tp'
const uint16_t			MsgTeamUpdate = 0x7475;					// 'tu'

// request for additional UDP link
const uint16_t			MsgUDPLinkRequest = 0x6f66;				// 'of'
const uint16_t			MsgUDPLinkEstablished = 0x6f67;			// 'og'
const uint16_t			MsgUDPLinkUpdate = 0x6f68;				// 'oh'
const uint16_t			MsgClientVersion = 0x6f6a;				// 'oj'

// server control message
const uint16_t			MsgServerControl = 0x6f69;				// 'oi'

// world database codes
const uint16_t			WorldCodeBase = 0x6261;					// 'ba'
const uint16_t			WorldCodeBox = 0x6278;					// 'bx'
const uint16_t			WorldCodeEnd = 0x6564;					// 'ed'
const uint16_t			WorldCodeLink = 0x6c6e;					// 'ln'
const uint16_t			WorldCodePyramid = 0x7079;				// 'py'
const uint16_t			WorldCodeStyle = 0x7374;				// 'st'
const uint16_t			WorldCodeTeleporter = 0x7465;			// 'te'
const uint16_t			WorldCodeWall = 0x776c;					// 'wl'

// ping packet sizes, codes and structure
const uint16_t			PingCodeOldReply = 0x0101;
const uint16_t			PingCodeOldRequest = 0x0202;
const uint16_t			PingCodeReply = 0x0303;
const uint16_t			PingCodeRequest = 0x0404;

// radio flags
const uint16_t			RadioToAll = 0x0001;

// rejection codes
const uint16_t			RejectBadRequest = 0x0000;
const uint16_t			RejectBadTeam = 0x0001;
const uint16_t			RejectBadType = 0x0002;
const uint16_t			RejectNoRogues = 0x0003;
const uint16_t			RejectTeamFull = 0x0004;
const uint16_t			RejectServerFull = 0x0005;

#endif // BZF_PROTOCOL_H
