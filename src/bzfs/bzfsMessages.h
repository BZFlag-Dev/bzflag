/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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

#include "common.h"

// system headers
#include "bzfs.h"

// common headers
#include "global.h"
#include "Address.h"

// local headers
#include "bzfs.h"
#include "GameKeeper.h"


// general messages sent to players ( local and remove )
void sendRemovePlayerMessage ( int playerID );
void sendFlagUpdateMessage ( int playerID );
void sendFlagUpdateMessage ( FlagInfo &flag );
void sendExistingPlayerUpdates ( int newPlayer );
bool sendTeamUpdateMessage( int newPlayer );
void sendTeamUpdateMessageBroadcast( int teamIndex1 = -1, int teamIndex2 = -1 );
void sendRejectPlayerMessage ( int playerID, uint16_t code , const char* reason );
bool sendAcceptPlayerMessage ( int playerID );
void sendHandicapInfoUpdate (int playerID );
void sendSingleHandicapInfoUpdate ( GameKeeper::Player* playerData );
void sendAdminInfoMessage ( int aboutPlayer, int toPlayer, bool record = false );
void sendWorldChunk(NetHandler *handler, uint32_t &ptr);
void broadcastPlayerStateUpdate ( void );
void sendTextMessage ( int destPlayer, int sourcePlayer, const char* text, int len, bool recordOnly = false );
void sendMessageAlive ( int playerID, float pos[3], float rot );
void sendMessageAllow ( int recipID, int playerID, unsigned char allow );
void sendMessageAllow ( int playerID, unsigned char allow );
bool sendPlayerStateMessage ( GameKeeper::Player *playerData, bool shortState);
void sendPlayerKilledMessage (int victimIndex, int killerIndex, BlowedUpReason reason, int16_t shotIndex, const FlagType*flagType, int phydrv);
void sendPlayerScoreUpdate ( GameKeeper::Player *player );
void sendScoreOverMessage ( int playerID, TeamColor team  );
void sendDropFlagMessage ( int playerIndex, FlagInfo &flag );
void sendFlagCaptureMessage ( int playerIndex, int flagIndex, int teamCaptured );
void sendRabbitUpdate ( int playerIndex, unsigned char mode );
void sendSetTeam ( int playerIndex, int team );
void sendClosestFlagMessage(int playerIndex,FlagType *type, float pos[3] );
bool sendGrabFlagMessage (int playerIndex, FlagInfo &flag );
void sendFlagTransferMessage (int toPlayer, int fromPlayer , FlagInfo &flag );
void sendSetShotType ( int playerIndex, ShotType type );
void sendMsgShotBegin ( int player, unsigned short id, FiringInfo &firingInfo );
void sendMsgShotEnd ( int player, unsigned short id, unsigned short reason );
void sendMsgTeleport ( int player, unsigned short from, unsigned short to );
void sendMsgAutoPilot ( int player, unsigned char autopilot );
void sendMsgGMUpdate ( int player, ShotUpdate *shot, int target );
void sendMsgWhatTimeIsIt ( NetHandler *handler, unsigned char tag, double time );
void sendMsgTimeUpdate ( int32_t timeLimit );
void sendMsgLuaData(PlayerId srcPlayerID, int16_t srcScriptID,
                    PlayerId dstPlayerID, int16_t dstScriptID,
                    uint8_t status, const std::string& data);

void sendMsgTangibilityUpdate ( unsigned int object, unsigned char tang, int player = AllPlayers );
void sendMsgTangibilityReset ( void );

void sendMsgCanSpawn ( int player, bool canSpawn );
void sendMsgLimboMessage ( int player, const std::string  &text );

void sendEchoResponse (struct sockaddr_in *uaddr, unsigned char tag);

// messages sent to just network users ( like client query )
int sendTeamUpdateDirect(NetHandler *handler);
int sendPlayerUpdateDirect(NetHandler *handler, GameKeeper::Player *otherData);

// receving network messages
void getGeneralMessageInfo ( void **buffer, uint16_t &code, uint16_t &len );

// utils
void playerStateToAPIState ( bz_PlayerUpdateState &apiState, const PlayerState &playerState );
void APIStateToplayerState ( PlayerState &playerState, const bz_PlayerUpdateState &apiState );

// custom data
void sendPlayerCustomDataPair ( int playerID, const std::string &key, const std::string &value );
void sendPlayerCustomDataPair ( int playerID, const char* key, const char* value );

/** class to pack a bunch of variables into one or more BufferedNetworkMessage.
 *  they are then automatically sent as they complete, or in the destructor.
 */
class PackVars
{
public:
  PackVars(NetHandler* _handler);
  ~PackVars();
  // callback forwarder
  static void packIt(const std::string &key, void *pv);

private:
  void sendPackVars(const std::string &key);
  void startMessage();
  void endMessage();
  NetHandler* handler;
  NetMsg msg;
  unsigned int count;
};

// utilities
bool isUDPAttackMessage ( uint16_t &code );
void playerStateToAPIState ( bz_PlayerUpdateState &apiState, const PlayerState &playerState );
void APIStateToplayerState ( PlayerState &playerState, const bz_PlayerUpdateState &apiState );

#endif //_BZFS_MESSAGES_H_


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
