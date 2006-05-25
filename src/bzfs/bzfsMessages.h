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
void sendTeamUpdateMessageBroadcast( int teamIndex1 = -1, int teamIndex2 = -1 );
void sendRejectPlayerMessage ( int playerID, uint16_t code , const char* reason );
bool sendAcceptPlayerMessage ( int playerID );
void sendHandycapInfoUpdate (int playerID );
void sendSingleHandycapInfoUpdate ( GameKeeper::Player* playerData );
void sendAdminInfoMessage ( int aboutPlayer, int toPlayer, bool record = false );
void sendWorldChunk(NetHandler *handler, uint32_t ptr);
void broadcastPlayerStateUpdate ( void );
void sendTextMessage ( int destPlayer, int sourcePlayer, const char* text, int len, bool recordOnly = false );
void sendMessageAlive ( int playerID, float pos[3], float rot );
bool sendPlayerStateMessage ( GameKeeper::Player *playerData, bool shortState);
void sendPlayerKilledMessage (int victimIndex, int killerIndex, BlowedUpReason reason, int16_t shotIndex, const FlagType*flagType, int phydrv);
void sendPlayerScoreUpdate ( GameKeeper::Player *player );
void sendScoreOverMessage ( int playerID, TeamColor team  );
void sendDropFlagMessage ( int playerIndex, FlagInfo &flag );
void sendFlagCaptureMessage ( int playerIndex, int flagIndex, int teamCaptured );
void sendRabbitUpdate ( int playerIndex, unsigned char mode );

// messages sent to just network users ( like client query )
int sendPlayerUpdateDirect(NetHandler *handler, GameKeeper::Player *otherData);
int sendTeamUpdateDirect(NetHandler *handler);

// net message utils
void  broadcastMessage(uint16_t code, int len, const void *msg, bool alsoTty = true);
void setGeneralMessageInfo ( void **buffer, uint16_t &code, uint16_t &len );

// receving network messages
void getGeneralMessageInfo ( void **buffer, uint16_t &code, uint16_t &len );
GameKeeper::Player *getPlayerMessageInfo ( void **buffer, uint16_t &code, int &playerID );

/** class to send a bunch of BZDB variables via MsgSetVar.
* dtor does the actual send
*/
class PackVars
{
public:
	PackVars(void *buffer, NetHandler *_handler);
	~PackVars();
	// callback forwarder
	static void packIt(const std::string &key, void *pv);
	void sendPackVars(const std::string &key);

private:
	void * const bufStart;
	void *buf;
	NetHandler *handler;
	unsigned int len;
	unsigned int count;
};

// utilitys
bool isUDPAtackMessage ( uint16_t &code );
void playerStateToAPIState ( bz_PlayerUpdateState &apiState, PlayerState &playerState );
void APIStateToplayerState ( PlayerState &playerState, bz_PlayerUpdateState &apiState );

#endif //_BZFS_MESSAGES_H_


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
