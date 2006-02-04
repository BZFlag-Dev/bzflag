/* bzflag
* Copyright (c) 1993 - 2006 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef _BZFS_MESSAGES_H_
#define _BZFS_MESSAGES_H_

#include "global.h"
#include "GameKeeper.h"
#include "bzfs.h"

// general messages sent to players ( local and remove )
void sendRemovePlayerMessage ( int playerID );
void sendFlagUpdateMessage ( int playerID );
void sendExistingPlayerUpdates ( int newPlayer );
bool sendTeamUpdateMessage( int newPlayer );
void sendRejectPlayerMessage ( int playerID, uint16_t code , const char* reason );
bool sendAcceptPlayerMessage ( int playerID );
void sendHandycapInfoUpdate (int playerID );
void sendAdminInfoMessage ( int aboutPlayer, int toPlayer, bool record = false );
void broadcastPlayerStateUpdate ( void );

// messages sent to just network users ( like client query )
int sendPlayerUpdateDirect(NetHandler *handler, GameKeeper::Player *otherData);
int sendTeamUpdateDirect(NetHandler *handler);

// receving network messages
void getGeneralMessageInfo ( void **buffer, uint16_t &code, uint16_t &len );
GameKeeper::Player *getPlayerMessageInfo ( void **buffer, uint16_t &code, int &playerID );

// utilitys
bool isUDPAtackMessage ( uint16_t &code );

#endif //_BZFS_MESSAGES_H_


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
