/* bzflag
* Copyright (c) 1993 - 2007 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef _BZFS_CLIENT_MESSAGES_H_
#define _BZFS_CLIENT_MESSAGES_H_

#include "global.h"
#include "bzfs.h"

void handleWhatTimeMessage( NetHandler *handler, void* buf, uint16_t len );
void handeCapBits ( void*, uint16_t len, GameKeeper::Player *playerData );
void handleClientEnter ( void **buf, GameKeeper::Player *playerData );
void handleClientExit ( GameKeeper::Player *playerData );
void handleSetVar ( NetHandler *netHandler );
void handleFlagNegotiation( NetHandler *handler, void **buf, int len );
void handleWorldChunk( NetHandler *handler, void *buf );
void handleWorldSettings( NetHandler *handler );
void handleWorldHash( NetHandler *handler );
void handlePlayerKilled( GameKeeper::Player *playerData, void* buffer );
void handlePlayerFlagDrop( GameKeeper::Player *playerData, void* buffer );
void handleGameJoinRequest( GameKeeper::Player *playerData );
void handlePlayerUpdate ( void **buf, uint16_t &code, GameKeeper::Player *playerData, const void* rawbuf, int len );	//once relay is based on state, remove rawbuf
void handlePlayerMessage ( GameKeeper::Player *playerData, void* buffer );
void handleFlagCapture ( GameKeeper::Player *playerData, void* buffer);
void handleCollide ( GameKeeper::Player *playerData, void* buffer);
void handleFlagTransfer ( GameKeeper::Player *playerData, void* buffer);
void handleShotFired(void *buf, int len, NetHandler *handler);
void handleShotEnded(GameKeeper::Player *playerData, void *buf, int len);
void handleTankHit( GameKeeper::Player *playerData, void *buf, int len);
void handleTeleport( GameKeeper::Player *playerData, void *buf, int len);
void handleRabbitMessage( GameKeeper::Player *playerData );
void handlePauseMessage( GameKeeper::Player *playerData, void *buf, int len);
void handleAutoPilotMessage( GameKeeper::Player *playerData, void *buf, int len);
void handleLagPing( GameKeeper::Player *playerData, void *buf, int len);
void handleShotUpdate ( GameKeeper::Player *playerData, void *buf, int len );

// util functions
bool updatePlayerState(GameKeeper::Player *playerData, PlayerState &state, float timeStamp, bool shortState);

// using from bzfs
extern int bz_pwrite(NetHandler *handler, const void *b, int l);
extern void pwriteBroadcast(const void *b, int l, int mask);

#endif //_BZFS_CLIENT_MESSAGES_H_


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
