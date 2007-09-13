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

// bzflag global header
#include "bzfsMessages.h"
#include "PlayerState.h"
#include <assert.h>

void flagToAPIFlag ( FlagInfo &flag, bz_FlagUpdateRecord *flagRecord )
{
	bool hide = (flag.flag.type->flagTeam == ::NoTeam) && (flag.player == -1);

	flagRecord->index = flag.getIndex();

	if (hide)
	{
		flagRecord->type[0] = 'P';
		flagRecord->type[1] = 'Z';
	}
	else
		strncpy(flagRecord->type,flag.flag.type->flagAbbv,2);

	flagRecord->status = flag.flag.status;
	flagRecord->endurance = flag.flag.endurance;
	flagRecord->owner = flag.flag.owner;
	flagRecord->position[0] = flag.flag.position[0];flagRecord->position[1] = flag.flag.position[1];flagRecord->position[2] = flag.flag.position[2];
	flagRecord->launchPosition[0] = flag.flag.launchPosition[0];flagRecord->launchPosition[1] = flag.flag.launchPosition[1];flagRecord->launchPosition[2] = flag.flag.launchPosition[2];
	flagRecord->landingPosition[0] = flag.flag.landingPosition[0];flagRecord->landingPosition[1] = flag.flag.landingPosition[1];flagRecord->landingPosition[2] = flag.flag.landingPosition[2];
	flagRecord->flightTime = flag.flag.flightTime;
	flagRecord->flightEnd = flag.flag.flightEnd;
	flagRecord->initialVelocity = flag.flag.initialVelocity;
}

void sendRemovePlayerMessage ( int playerID )
{
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, playerID);
	broadcastMessage(MsgRemovePlayer, (char*)buf-(char*)bufStart, bufStart);

	for (int i = 0; i < curMaxPlayers; i++)
	{
		GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
		if ((p == NULL) || !p->playerHandler || playerID == p->getIndex())
			continue;
		p->playerHandler->playerRemoved(playerID);
	}
}

void sendFlagUpdateMessage ( int playerID )
{
	GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerID);
	if (!playerData)
		return;

	int result;
	int cnt = 0;

	std::vector<bz_FlagUpdateRecord*> flagRecordList;
	if (playerData->playerHandler)
	{
		for (int flagIndex = 0; flagIndex < numFlags; flagIndex++)
		{
			FlagInfo &flag = *FlagInfo::get(flagIndex);
			if (flag.exist())
			{
				bz_FlagUpdateRecord	*flagRecord = new bz_FlagUpdateRecord;
				flagToAPIFlag(flag,flagRecord);
				flagRecordList.push_back(flagRecord);
			}
		}

		bz_FlagUpdateRecord**	flagHandle = (bz_FlagUpdateRecord**)malloc(sizeof(bz_FlagUpdateRecord*)*flagRecordList.size());
		for(unsigned int i = 0; i < flagRecordList.size(); i++)
			flagHandle[i] = flagRecordList[i];

		playerData->playerHandler->flagUpdate ( (int)flagRecordList.size(), flagHandle );

		delete(flagHandle);
		for(unsigned int i = 0; i < flagRecordList.size(); i++)
			delete(flagRecordList[i]);
	}
	else
	{
		void *buf, *bufStart = getDirectMessageBuffer();

		buf = nboPackUShort(bufStart,0); //placeholder
		int length = sizeof(uint16_t);
		for (int flagIndex = 0; flagIndex < numFlags; flagIndex++)
		{
			FlagInfo &flag = *FlagInfo::get(flagIndex);
			if (flag.exist())
			{
				if ((length + sizeof(uint16_t) + FlagPLen) > MaxPacketLen - 2*sizeof(uint16_t))
				{
					nboPackUShort(bufStart, cnt);
					result = directMessage(playerData->netHandler, MsgFlagUpdate,(char*)buf - (char*)bufStart, bufStart);

					if (result == -1)
						return;

					cnt    = 0;
					length = sizeof(uint16_t);
					buf    = nboPackUShort(bufStart,0); //placeholder
				}

				bool hide = (flag.flag.type->flagTeam == ::NoTeam) && (flag.player == -1);
				buf = flag.pack(buf, hide);
				length += sizeof(uint16_t)+FlagPLen;
				cnt++;
			}
		}

		if (cnt > 0)
		{
			nboPackUShort(bufStart, cnt);
			result = directMessage(playerData->netHandler, MsgFlagUpdate,
				(char*)buf - (char*)bufStart, bufStart);
		}
	}
}

void sendExistingPlayerUpdates ( int newPlayer )
{
	GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(newPlayer);

	if (!playerData)
		return;

	GameKeeper::Player *otherData;
	for (int i = 0; i < curMaxPlayers; i++)
	{
		if (i == newPlayer)
			continue;
		otherData = GameKeeper::Player::getPlayerByIndex(i);
		if (!otherData ||!otherData->player.isPlaying())
			continue;
		if (playerData->playerHandler)
		{
			bz_PlayerInfoUpdateRecord	playerRecord;

			playerRecord.index = i;
			playerRecord.type = (bz_ePlayerType)otherData->player.getType();
			playerRecord.team = convertTeam(otherData->player.getTeam());
			playerRecord.score.wins = otherData->score.getWins();
			playerRecord.score.losses = otherData->score.getLosses();
			playerRecord.score.tks = otherData->score.getTKs();
			memset(playerRecord.callsign,32,0);
			memset(playerRecord.email,128,0);
			strncpy(playerRecord.callsign,otherData->player.getCallSign(),31);
			strncpy(playerRecord.email,otherData->player.getEMail(),127);

			playerData->playerHandler->playerInfoUpdate(&playerRecord);
		}
		else
		{
			if (sendPlayerUpdateDirect(playerData->netHandler,otherData) < 0)
				break;
		}
		sendMessageAllow(newPlayer, i, otherData->player.getAllow());
	}
}

bool sendTeamUpdateMessage( int newPlayer )
{
	GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(newPlayer);

	if (!playerData)
		return false;

	if (!playerData->playerHandler)
		return sendTeamUpdateDirect(playerData->netHandler) == 0;
	else
	{
		bz_TeamInfoRecord	**teams = (bz_TeamInfoRecord**)malloc(sizeof(bz_TeamInfoRecord*)*CtfTeams);
		for (int t = 0; t < CtfTeams; t++)
		{
			teams[t] = new bz_TeamInfoRecord;

			teams[t]->id = t;
			teams[t]->size = team[t].team.size;
			teams[t]->wins = team[t].team.won;
			teams[t]->losses = team[t].team.won;
		}
		playerData->playerHandler->teamUpdate ( CtfTeams, teams );

		for (int t = 0; t < CtfTeams; t++)
			delete(teams[t]);

		free(teams);
	}
	return true;
}

void sendTeamUpdateMessageBroadcast( int teamIndex1, int teamIndex2 )
{
	if (!allowTeams())
		return;

	// If teamIndex1 is -1, send all teams
	// If teamIndex2 is -1, just send teamIndex1 team
	// else send both teamIndex1 and teamIndex2 teams

	void *buf, *bufStart = getDirectMessageBuffer();
	if (teamIndex1 == -1)
	{
		buf = nboPackUByte(bufStart, CtfTeams);
		for (int t = 0; t < CtfTeams; t++)
		{
			buf = nboPackUShort(buf, t);
			buf = team[t].team.pack(buf);
		}
	}
	else if (teamIndex2 == -1)
	{
		buf = nboPackUByte(bufStart, 1);
		buf = nboPackUShort(buf, teamIndex1);
		buf = team[teamIndex1].team.pack(buf);
	}
	else
	{
		buf = nboPackUByte(bufStart, 2);
		buf = nboPackUShort(buf, teamIndex1);
		buf = team[teamIndex1].team.pack(buf);
		buf = nboPackUShort(buf, teamIndex2);
		buf = team[teamIndex2].team.pack(buf);
	}

	broadcastMessage(MsgTeamUpdate, (char*)buf - (char*)bufStart, bufStart, false);

	bz_TeamInfoRecord	**teams = NULL;

	int teamCount = 0;

	if (teamIndex1 == -1)
	{
		teamCount = CtfTeams;
		teams = (bz_TeamInfoRecord**)malloc(sizeof(bz_TeamInfoRecord*)*CtfTeams);
		for (int t = 0; t < CtfTeams; t++)
		{
			teams[t] = new bz_TeamInfoRecord;

			teams[t]->id = t;
			teams[t]->size = team[t].team.size;
			teams[t]->wins = team[t].team.won;
			teams[t]->losses = team[t].team.won;
		}
	}
	else if (teamIndex2 == -1)
	{
		teamCount = 1;
		teams = (bz_TeamInfoRecord**)malloc(sizeof(bz_TeamInfoRecord*));

		teams[0] = new bz_TeamInfoRecord;

		teams[0]->id = teamIndex2;
		teams[0]->size = team[teamIndex2].team.size;
		teams[0]->wins = team[teamIndex2].team.won;
	}
	else
	{
		teamCount = 2;
		teams = (bz_TeamInfoRecord**)malloc(sizeof(bz_TeamInfoRecord*)*2);

		teams[0] = new bz_TeamInfoRecord;

		teams[0]->id = teamIndex1;
		teams[0]->size = team[teamIndex1].team.size;
		teams[0]->wins = team[teamIndex1].team.won;

		teams[1] = new bz_TeamInfoRecord;

		teams[1]->id = teamIndex2;
		teams[1]->size = team[teamIndex2].team.size;
		teams[1]->wins = team[teamIndex2].team.won;

	}

	// now do everyone who dosn't have network
	for (int i = 0; i < curMaxPlayers; i++)
	{
		GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
		if (otherData && otherData->playerHandler)
			otherData->playerHandler->teamUpdate ( teamCount, teams );
	}

	for (int t = 0; t < teamCount; t++)
		delete(teams[t]);

	free(teams);
}

void sendRejectPlayerMessage ( int playerID, uint16_t code , const char* reason )
{
	GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerID);

	if (playerData->playerHandler)
		playerData->playerHandler->playerRejected ( (bz_eRejectCodes)code,reason );
	else
	{
		void *buf, *bufStart = getDirectMessageBuffer();
		buf = nboPackUShort(bufStart, code);
		buf = nboPackString(buf, reason, strlen(reason) + 1);
		directMessage(playerID, MsgReject, sizeof (uint16_t) + MessageLen, bufStart);
	}
}

bool sendAcceptPlayerMessage ( int playerID )
{
	GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerID);

	if (playerData->playerHandler)
		playerData->playerHandler->playerAccepted();
	else
	{
		void *buf, *bufStart = getDirectMessageBuffer();
		buf = nboPackUByte(bufStart, playerID);
		int result = directMessage(playerData->netHandler, MsgAccept,(char*)buf-(char*)bufStart, bufStart);
		if (result < 0)
			return false;
	}

	return true;
}

void sendHandicapInfoUpdate(int playerID)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerID);
  if (!playerData)
    return;

  GameKeeper::Player *otherData;
  if (clOptions->gameOptions & HandicapGameStyle) {
    if (playerData->playerHandler) {
      std::vector<bz_HandicapUpdateRecord*> handyList;
      for (int i = 0; i < curMaxPlayers; i++) {
	otherData = GameKeeper::Player::getPlayerByIndex(i);
	if (otherData) {
	  bz_HandicapUpdateRecord *handyData = new bz_HandicapUpdateRecord;
	  handyData->player   = i;
	  handyData->handicap = otherData->score.getHandicap();
	  handyList.push_back(handyData);
	}

	bz_HandicapUpdateRecord **handyPtrList
	  = (bz_HandicapUpdateRecord**)malloc(sizeof(bz_HandicapUpdateRecord*)
					      * handyList.size());
	for (int j = 0; j < (int)handyList.size(); j++)
	  handyPtrList[j] = handyList[j];

	playerData->playerHandler->handicapUpdate((int)handyList.size(),
						  handyPtrList);

	free(handyPtrList);
	for (int k = 0; k < (int)handyList.size(); k++)
	  delete(handyList[k]);
      }
    } else {
      int numHandicaps = 0;

      // Send handicap for all players
      void *bufStart = getDirectMessageBuffer();
      void *buf      = nboPackUByte(bufStart, numHandicaps);
      for (int i = 0; i < curMaxPlayers; i++) {
	if (i != playerID) {
	  otherData = GameKeeper::Player::getPlayerByIndex(i);
	  if (otherData) {
	    numHandicaps++;
	    buf = nboPackUByte(buf, i);
	    buf = nboPackShort(buf, otherData->score.getHandicap());
	  }
	}
      }
      nboPackUByte(bufStart, numHandicaps);
      broadcastMessage(MsgHandicap, (char*)buf - (char*)bufStart, bufStart);
    }
  }
}

void sendSingleHandicapInfoUpdate ( GameKeeper::Player* playerData )
{
	if (!playerData)
		return;

	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, 1);
	buf = nboPackUByte(buf, playerData->getIndex());
	buf = nboPackShort(buf, playerData->score.getHandicap());
	broadcastMessage(MsgHandicap, (char*)buf-(char*)bufStart, bufStart);

	bz_HandicapUpdateRecord *handyData = new bz_HandicapUpdateRecord;
	handyData->player = playerData->getIndex();
	handyData->handicap = playerData->score.getHandicap();

	// now do everyone who dosn't have network
	for (int i = 0; i < curMaxPlayers; i++)
	{
		GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
		if (otherData && otherData->playerHandler)
			otherData->playerHandler->handicapUpdate(1,&handyData);
	}
	delete(handyData);
}

void sendAdminInfoMessage ( int aboutPlayer, int toPlayer, bool record )
{
	GameKeeper::Player *aboutPlayerData = NULL;
	GameKeeper::Player *toPlayerData = NULL;

	aboutPlayerData = GameKeeper::Player::getPlayerByIndex(aboutPlayer);
	if (!aboutPlayerData || !aboutPlayerData->netHandler)
		return;

	if (!record)
	{
		toPlayerData = GameKeeper::Player::getPlayerByIndex(toPlayer);
		if (!toPlayerData)
			return;

		if (toPlayerData->playerHandler)
		{
			if (!aboutPlayerData->netHandler)
				toPlayerData->playerHandler->playerIPUpdate (aboutPlayer,"local.player");
			else
				toPlayerData->playerHandler->playerIPUpdate (aboutPlayer,aboutPlayerData->netHandler->getTargetIP());
		}
	}

	void *buf, *bufStart = getDirectMessageBuffer();
	if (toPlayerData || record)
	{
		buf = nboPackUByte(bufStart, 1);
		buf = aboutPlayerData->packAdminInfo(buf);
	}

	if (toPlayerData)
	{
		if (!toPlayerData->playerHandler)
			directMessage(toPlayer, MsgAdminInfo,(char*)buf - (char*)bufStart, bufStart);
	}

	if (record)
		Record::addPacket(MsgAdminInfo,(char*)buf - (char*)bufStart, bufStart, HiddenPacket);
}

void sendFlagTransferMessage (int toPlayer, int fromPlayer , FlagInfo &flag )
{
	void *obufStart = getDirectMessageBuffer();
	void *obuf = nboPackUByte(obufStart, toPlayer);
	obuf = nboPackUByte(obuf, fromPlayer);

	GameKeeper::Player *toData = GameKeeper::Player::getPlayerByIndex(toPlayer);
	GameKeeper::Player *fromData = GameKeeper::Player::getPlayerByIndex(fromPlayer);

	toData->efectiveShotType = fromData->efectiveShotType;
	fromData->efectiveShotType = StandardShot;
	flag.flag.owner = toPlayer;
	flag.player = toPlayer;
	toData->player.resetFlag();
	toData->player.setFlag(flag.getIndex());
	fromData->player.resetFlag();
	obuf = flag.pack(obuf);
	obuf = nboPackUByte(obuf,toData->efectiveShotType);

	broadcastMessage(MsgTransferFlag, (char*)obuf - (char*)obufStart, obufStart);

	// now do everyone who dosn't have network
	for (int i = 0; i < curMaxPlayers; i++)
	{
		GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
		if (otherData && otherData->playerHandler)
			otherData->playerHandler->flagTransfer(fromData->getIndex(),toData->getIndex(),flag.getIndex(),(bz_eShotType)toData->efectiveShotType);
	}
}

void sendClosestFlagMessage(int playerIndex,FlagType *type , float pos[3] )
{
	GameKeeper::Player* playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);

	if (!type || !playerData)
		return;
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackVector(bufStart, pos);
	buf = nboPackStdString(buf, std::string(type->flagName));
	if ( playerData->playerHandler)
		playerData->playerHandler->nearestFlag(type->flagName,pos);
	else
		directMessage(playerIndex, MsgNearFlag,(char*)buf - (char*)bufStart, bufStart);

}

bool sendGrabFlagMessage (int playerIndex, FlagInfo &flag )
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return false;

  bz_AllowFlagGrabEventData_V1	allow;
  allow.flagID = flag.getIndex();
  allow.flagType = flag.flag.type->flagAbbv;
  allow.shotType = (bz_eShotType)flag.flag.type->flagShot;
  allow.playerID = playerIndex;
  allow.allow = true;

  worldEventManager.callEvents(bz_eAllowFlagGrabEvent,&allow);

  if (!allow.allow)
    return false;

  flag.grab(playerIndex);
  playerData->player.setFlag(flag.getIndex());

  // send MsgGrabFlag
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = flag.pack(buf);

  bz_FlagGrabbedEventData_V1	data;
  data.flagID = flag.getIndex();
  data.flagType = flag.flag.type->flagAbbv;
  data.shotType = (bz_eShotType)flag.flag.type->flagShot;
  data.playerID = playerIndex;

  worldEventManager.callEvents(bz_eFlagGrabbedEvent,&data);

  // pack in the shot type, it may have been modified
  buf = nboPackUByte(buf,data.shotType);
  playerData->efectiveShotType = (ShotType)data.shotType;

  broadcastMessage(MsgGrabFlag, (char*)buf - (char*)bufStart, bufStart);

  // now do everyone who dosn't have network
  for (int i = 0; i < curMaxPlayers; i++)
  {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->grabFlag(playerIndex,flag.getIndex(),flag.flag.type->flagAbbv,(bz_eShotType)playerData->efectiveShotType);
  }

  playerData->flagHistory.add(flag.flag.type);
  return true;
}

void sendSetShotType ( int playerIndex, ShotType type )
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  if (type == playerData->efectiveShotType )
    return; // it's the same as what they have

  playerData->efectiveShotType = type;

  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUByte(buf,type);

  broadcastMessage(MsgSetShot, (char*)buf - (char*)bufStart, bufStart);

  // now do everyone who dosn't have network
  for (int i = 0; i < curMaxPlayers; i++)
  {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->setShotType(playerIndex,(bz_eShotType)playerData->efectiveShotType);
  }
}

void sendMsgShotBegin ( int player, unsigned short id, FiringInfo &firingInfo )
{
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, player);
  buf = nboPackUShort(buf, id);
  buf = firingInfo.pack(buf);

  relayMessage(MsgShotBegin, (char*)buf-(char*)bufStart, bufStart);

  // now do everyone who dosn't have network
  for (int i = 0; i < curMaxPlayers; i++)
  {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->shotFired(player,id,(bz_eShotType)firingInfo.shotType);
  }
}

void sendMsgShotEnd ( int player, unsigned short id, unsigned short reason )
{
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, player);
  buf = nboPackShort(buf, id);
  buf = nboPackUShort(buf, reason);
  relayMessage(MsgShotEnd, (char*)buf-(char*)bufStart, bufStart);

  // now do everyone who dosn't have network
  for (int i = 0; i < curMaxPlayers; i++)
  {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->shotEnded(player,id,reason);
  }
 }

void sendMsgTeleport( int player, unsigned short from, unsigned short to )
{
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, player);
  buf = nboPackUShort(buf, from);
  buf = nboPackUShort(buf, to);

  broadcastMessage(MsgTeleport, (char*)buf-(char*)bufStart, bufStart, false);

  // now do everyone who dosn't have network
  for (int i = 0; i < curMaxPlayers; i++)
  {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->playerTeleported(player,from,to);
  }
}

void sendMsgAutoPilot( int player, unsigned char autopilot )
{
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, player);
  buf = nboPackUByte(buf, autopilot);

  broadcastMessage(MsgAutoPilot, (char*)buf-(char*)bufStart, bufStart);

  // now do everyone who dosn't have network
  for (int i = 0; i < curMaxPlayers; i++)
  {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->playerAutopilot(player,autopilot != 0);
  }
}

// network only messages
int sendPlayerUpdateDirect(NetHandler *handler, GameKeeper::Player *otherData)
{
  if (!otherData->player.isPlaying())
    return 0;

  void *bufStart = getDirectMessageBuffer();
  void *buf      = otherData->packPlayerUpdate(bufStart);

  return directMessage(handler, MsgAddPlayer,
    (char*)buf - (char*)bufStart, bufStart);
}

int sendTeamUpdateDirect(NetHandler *handler)
{
  // send all teams
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, CtfTeams);
  
  for (int t = 0; t < CtfTeams; t++)
  {
    buf = nboPackUShort(buf, t);
    buf = team[t].team.pack(buf);
  }
  return directMessage(handler, MsgTeamUpdate,
    (char*)buf - (char*)bufStart, bufStart);
}

void sendWorldChunk(NetHandler *handler, uint32_t &ptr)
{
  worldWasSentToAPlayer = true;
  // send another small chunk of the world database
  assert((world != NULL) && (worldDatabase != NULL));

  void *buf, *bufStart = getDirectMessageBuffer();
  uint32_t size = MaxPacketLen - 2*sizeof(uint16_t) - sizeof(uint32_t);
  uint32_t left = worldDatabaseSize - ptr;

  if (ptr >= worldDatabaseSize)
  {
    size = 0;
    left = 0;
  }
  else if (ptr + size >= worldDatabaseSize)
  {
    size = worldDatabaseSize - ptr;
    left = 0;
  }
  buf = nboPackUInt(bufStart, uint32_t(left));
  buf = nboPackString(buf, (char*)worldDatabase + ptr, size);
  directMessage(handler, MsgGetWorld, (char*)buf - (char*)bufStart, bufStart);
}

void sendTextMessage(int destPlayer, int sourcePlayer, const char *text,
		     int len, bool recordOnly)
{
  bool broadcast = false;
  bool toGroup   = false;
  GameKeeper::Player *destPlayerData = NULL;

  if (destPlayer == AllPlayers) {
    broadcast = true;
  } else {
    if (destPlayer > LastRealPlayer) {
      toGroup = true;
    } else {
      destPlayerData = GameKeeper::Player::getPlayerByIndex(destPlayer);
    }
  }

  if (!destPlayerData && (!broadcast && !toGroup && !recordOnly))
    return;

  char *localtext = (char*)malloc(len+1);
  strncpy(localtext, text, (size_t)len);
  localtext[len] = '\0';

  if (destPlayerData && destPlayerData->playerHandler && !recordOnly) {
    destPlayerData->playerHandler->textMessage(destPlayer, sourcePlayer, localtext);
  }

  if (recordOnly || (destPlayerData && !destPlayerData->playerHandler)
      || broadcast || toGroup) {
    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUByte(bufStart, sourcePlayer);
    buf = nboPackUByte(buf, destPlayer);
    buf = nboPackString(buf, localtext, len);

    ((char*)bufStart)[MessageLen - 1 + 2] = '\0'; // always terminate

    if (recordOnly) {
      Record::addPacket(MsgMessage, len+2, bufStart, HiddenPacket);
    } else {
      if (!broadcast && !toGroup) {
        if (sourcePlayer != destPlayer)
	  directMessage(sourcePlayer, MsgMessage, (len + 2), bufStart);
	directMessage(destPlayer, MsgMessage, (len + 2), bufStart);
      } else {
	if (broadcast) {
	  broadcastMessage(MsgMessage, (len + 2), bufStart);

	  // now do everyone who isn't a net player
	  for (int i = 0; i < curMaxPlayers; i++) {
	    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
	    if (otherData && otherData->playerHandler) {
	      otherData->playerHandler->textMessage(destPlayer, sourcePlayer, localtext);
	    }
	  }
	} else {
	  if (toGroup) {
	    if (destPlayer == AdminPlayers) {
	      directMessage(sourcePlayer, MsgMessage, (len + 2), bufStart);
	      std::vector<int> admins  = GameKeeper::Player::allowed(PlayerAccessInfo::adminMessageReceive);
	      for (unsigned int i = 0; i < admins.size(); ++i) {
		if (admins[i] != sourcePlayer) {
		  directMessage(admins[i], MsgMessage, (len + 2), bufStart);
		}
	      }
	    } else { // to a team
	      TeamColor destTeam = TeamColor(250 - destPlayer);	// FIXME this teamcolor <-> player id conversion is in several files now
	      for (int i = 0; i < curMaxPlayers; i++) {
		GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
		if (otherData && otherData->player.isPlaying() && otherData->player.isTeam(destTeam)) {
		  directMessage(i, MsgMessage, (len + 2), bufStart);
		}
	      }
	    }
	  }
	}
      }
    }
  }
}

void sendMessageAlive ( int playerID, float pos[3], float rot )
{
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, playerID);
	buf = nboPackVector(buf, pos);
	buf = nboPackFloat(buf, rot);
	broadcastMessage(MsgAlive, (char*)buf - (char*)bufStart, bufStart);

	// now do everyone who dosn't have network
	for (int i = 0; i < curMaxPlayers; i++)
	{
		GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
		if (otherData && otherData->playerHandler)
			otherData->playerHandler->playerSpawned(playerID,pos,rot);
	}
}

void sendMessageAllow ( int recipID, int playerID, unsigned char allow )
{
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, playerID);
	buf = nboPackUByte(buf, allow);
	directMessage(recipID, MsgAllow, (char*)buf - (char*)bufStart, bufStart);
}

void sendMessageAllow ( int playerID, unsigned char allow )
{
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, playerID);
	buf = nboPackUByte(buf, allow);
	broadcastMessage(MsgAllow, (char*)buf - (char*)bufStart, bufStart);

	// Please note that non-network players do not currently receive the message.
}

bool sendPlayerStateMessage( GameKeeper::Player *playerData, bool shortState )
{
	playerData->doPlayerDR();

	// pack up the data and send it to the net players
	uint16_t len = PlayerUpdatePLenMax;	// this len is dumb, it shoudl be the REAl len of the packet
	uint16_t code = shortState ? MsgPlayerUpdateSmall : MsgPlayerUpdate;
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = bufStart;
	setGeneralMessageInfo(&buf,code,len);
	buf = nboPackUByte(buf, playerData->getIndex());
	buf = nboPackFloat(buf, playerData->stateTimeStamp);
	buf = playerData->lastState.pack(buf,code,false);	// don't increment the order cus this is just a relay

	// send the packet to everyone else who is playing and NOT a server side bot
	relayPlayerPacket(playerData->getIndex(), len, bufStart, code);

	// bots need love too
	bz_PlayerUpdateState	apiState;
	playerStateToAPIState(apiState,playerData->getCurrentStateAsState());
	// now do everyone who dosn't have network
	for (int i = 0; i < curMaxPlayers; i++)
	{
		GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
		if (otherData && otherData->playerHandler && otherData->player.isPlaying())
			otherData->playerHandler->playerStateUpdate(playerData->getIndex(),&apiState,playerData->stateTimeStamp);
	}
	return true;
}

void sendPlayerKilledMessage(int victimIndex, int killerIndex, BlowedUpReason reason, int16_t shotIndex, const FlagType*flagType, int phydrv)
{
	// send MsgKilled
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, victimIndex);
	buf = nboPackUByte(buf, killerIndex);
	buf = nboPackShort(buf, reason);
	buf = nboPackShort(buf, shotIndex);
	buf = flagType->pack(buf);

	if (reason == PhysicsDriverDeath)
		buf = nboPackInt(buf, phydrv);

	broadcastMessage(MsgKilled, (char*)buf-(char*)bufStart, bufStart);

	for (int i = 0; i < curMaxPlayers; i++)
	{
		GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
		if (otherData && otherData->playerHandler && otherData->player.isPlaying())
			otherData->playerHandler->playerKilledMessage(victimIndex,killerIndex,(bz_ePlayerDeathReason)reason,shotIndex,flagType->flagAbbv,phydrv);
	}
}

void sendPlayerScoreUpdate( GameKeeper::Player *player )
{
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, 1);
	buf = nboPackUByte(buf, player->getIndex());
	buf = player->score.pack(buf);
	broadcastMessage(MsgScore, (char*)buf-(char*)bufStart, bufStart);

	for (int i = 0; i < curMaxPlayers; i++)
	{
		GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
		if (otherData && otherData->playerHandler)
			otherData->playerHandler->playerScoreUpdate(player->getIndex(),player->score.getWins(),player->score.getLosses(),player->score.getTKs());
	}
}

void sendScoreOverMessage(int playerID, TeamColor _team)
{
  void *buf,*bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerID);
  buf = nboPackUShort(buf, uint16_t(_team));
  broadcastMessage(MsgScoreOver, (char*)buf - (char*)bufStart, bufStart);

  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->scoreLimitReached(playerID,
						  convertTeam(_team));
  }
}

void sendDropFlagMessage ( int playerIndex, FlagInfo &flag )
{
	void *bufStart = getDirectMessageBuffer();
	void *buf      = nboPackUByte(bufStart, playerIndex);
	buf	    = flag.pack(buf);
	broadcastMessage(MsgDropFlag, (char*)buf-(char*)bufStart, bufStart);

	bz_FlagUpdateRecord		*flagRecord = new bz_FlagUpdateRecord;
	flagToAPIFlag(flag,flagRecord);

	for (int i = 0; i < curMaxPlayers; i++)
	{
		GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
		if (otherData && otherData->playerHandler)
			otherData->playerHandler->flagUpdate ( 1, &flagRecord );
	}
	delete(flagRecord);
}

void sendFlagCaptureMessage ( int playerIndex, int flagIndex, int teamCaptured )
{
  // send MsgCaptureFlag
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUShort(buf, uint16_t(flagIndex));
  buf = nboPackUShort(buf, uint16_t(teamCaptured));
  broadcastMessage(MsgCaptureFlag, (char*)buf-(char*)bufStart, bufStart,false);

  for (int i = 0; i < curMaxPlayers; i++)
  {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
	otherData->playerHandler->flagCaptured ( playerIndex, flagIndex, convertTeam((TeamColor)teamCaptured) );
  }
}

void sendRabbitUpdate ( int playerIndex, unsigned char mode )
{
  void *buf, *bufStart = getDirectMessageBuffer();

  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUByte(buf, mode);
  broadcastMessage(MsgNewRabbit, (char*)buf-(char*)bufStart, bufStart);
}

void sendMsgGMUpdate ( int player, ShotUpdate *shot )
{
  void *buf, *bufStart = getDirectMessageBuffer();

  buf = nboPackUByte(bufStart, player);
  buf = shot->pack(buf);
  broadcastMessage(MsgGMUpdate, (char*)buf-(char*)bufStart, bufStart);
}

void sendMsgWhatTimeIsIt ( NetHandler *handler, unsigned char tag, float time )
{
  /* Pack a message with the given time */
  void *bufStart;
  void *buf2 = bufStart = getDirectMessageBuffer();
  buf2 = nboPackUByte(bufStart,tag);
  buf2 = nboPackFloat(buf2,time);

  directMessage(handler, MsgWhatTimeIsIt, (char*)buf2-(char*)bufStart, bufStart);
}

void sendMsgTimeUpdate( int timeLimit )
{
  // start client's clock
  void *msg = getDirectMessageBuffer();
  nboPackInt(msg, timeLimit);
  broadcastMessage(MsgTimeUpdate, sizeof(int32_t), msg);
}

void sendMsgTanagabilityUpdate ( unsigned int object, unsigned char tang, int player )
{
  bool broadcast = player == AllPlayers;

  void *bufStart;
  void *buf2 = bufStart = getDirectMessageBuffer();
  buf2 = nboPackUInt(bufStart,object);
  buf2 = nboPackUByte(buf2,tang);

  if (broadcast)
    broadcastMessage(MsgTangibilityUpdate, (char*)buf2 - (char*)bufStart, bufStart,false);
  else
    directMessage(player, MsgTangibilityUpdate, (char*)buf2 - (char*)bufStart, bufStart);
}

void sendMsgTanagabilityReset ( void )
{
  broadcastMessage(MsgTangibilityReset, 0, NULL,false);
}

void sendMsgCanSpawn ( int player, bool canSpawn )
{
  char t= 0;
  if (canSpawn)
    t = 1;

  GameKeeper::Player* p = GameKeeper::Player::getPlayerByIndex(player);
  if (!p)
    return;

  if (!p->playerHandler)
  {
    void *bufStart;
    void *buf2 = bufStart = getDirectMessageBuffer();
    buf2 = nboPackUByte(buf2,t);

    directMessage(player, MsgAllowSpawn, (char*)buf2 - (char*)bufStart, bufStart);
  }
  else
    p->playerHandler->allowSpawn(canSpawn);
}

void sendMsgLimboMessage ( int player, const std::string  &text )
{
  void *bufStart;
  void *buf2 = bufStart = getDirectMessageBuffer();
  buf2 = nboPackStdString(buf2,text);
  directMessage(player, MsgLimboMessage, (char*)buf2 - (char*)bufStart, bufStart);

}


void sendSetTeam ( int playerIndex, int _team )
{
	void *buf, *bufStart = getDirectMessageBuffer();

	buf = nboPackUByte(bufStart, playerIndex);
	buf = nboPackUByte(buf, _team);
	broadcastMessage(MsgSetTeam, (char*)buf - (char*)bufStart, bufStart);
}

void sendEchoResponse (struct sockaddr_in *uaddr, unsigned char tag)
{
  uint16_t len = 1;
  
  uint16_t code = MsgEchoResponse;
  
  char echobuffer[5] = {0};
  void *ebuf = echobuffer;
  setGeneralMessageInfo(&ebuf, code, len);
  ebuf = nboPackUByte(ebuf, tag);
  sendto(NetHandler::getUdpSocket(), echobuffer, 5, 0, (struct sockaddr*)uaddr,
         sizeof(*uaddr));   //Low level - bad - if this could be encapsulated...
}

// utils to build new packets
void setGeneralMessageInfo ( void **buffer, uint16_t &code, uint16_t &len )
{
	*buffer = nboPackUShort(*buffer, len);
	*buffer = nboPackUShort(*buffer, code);
}

//messages sent TO the server
void getGeneralMessageInfo ( void **buffer, uint16_t &code, uint16_t &len )
{
  *buffer = nboUnpackUShort(*buffer, len);
  *buffer = nboUnpackUShort(*buffer, code);
}

PackVars::PackVars(void *buffer, NetHandler *_handler) : bufStart(buffer)
{
	buf = nboPackUShort(bufStart, 0);//placeholder
	handler = _handler;
	len = sizeof(uint16_t);
	count = 0;
}

PackVars::~PackVars()
{
	if (len > sizeof(uint16_t)) {
		nboPackUShort(bufStart, count);
		directMessage(handler, MsgSetVar, len, bufStart);
	}
}

// callback forwarder
void PackVars::packIt(const std::string &key, void *pv)
{
	reinterpret_cast<PackVars*>(pv)->sendPackVars(key);
}

void PackVars::sendPackVars(const std::string &key)
{
	std::string value = BZDB.get(key);
	int pairLen = key.length() + 1 + value.length() + 1;
	if ((pairLen + len) > (MaxPacketLen - 2*sizeof(uint16_t))) {
		nboPackUShort(bufStart, count);
		count = 0;
		directMessage(handler, MsgSetVar, len, bufStart);
		buf = nboPackUShort(bufStart, 0); //placeholder
		len = sizeof(uint16_t);
	}

	buf = nboPackUByte(buf, key.length());
	buf = nboPackString(buf, key.c_str(), key.length());
	buf = nboPackUByte(buf, value.length());
	buf = nboPackString(buf, value.c_str(), value.length());
	len += pairLen;
	count++;
}

// net utils
void broadcastMessage(uint16_t code, int len, const void *msg, bool alsoTty)
{
  void *bufStart = (char *)msg - 2*sizeof(uint16_t);
  void *buf = nboPackUShort(bufStart, uint16_t(len));
  nboPackUShort(buf, code);

  // send message to everyone
  int mask = NetHandler::clientBZFlag;
  if (alsoTty)
	  mask |= NetHandler::clientBZAdmin;
  pwriteBroadcast(bufStart, len + 4, mask);

  // record the packet
  if (Record::enabled())
    Record::addPacket(code, len, msg);
}

//utils
bool isUDPAttackMessage ( uint16_t &code )
{
    switch (code)
    {
      case MsgShotBegin:
      case MsgShotEnd:
      case MsgPlayerUpdate:
      case MsgPlayerUpdateSmall:
      case MsgGMUpdate:
      case MsgUDPLinkRequest:
      case MsgUDPLinkEstablished:
      case MsgHit:
      case MsgWhatTimeIsIt:
	      return false;
    }
    return true;
}

void playerStateToAPIState ( bz_PlayerUpdateState &apiState, const PlayerState &playerState )
{
	apiState.status = eAlive;
	if (playerState.status & PlayerState::DeadStatus)
		apiState.status = eDead;
	else if (playerState.status & PlayerState::Paused)
		apiState.status = ePaused;
	else if (playerState.status & PlayerState::Exploding)
		apiState.status = eExploding;
	else if (playerState.status & PlayerState::Teleporting)
		apiState.status = eTeleporting;
	else if (playerState.status & PlayerState::InBuilding)
		apiState.status = eInBuilding;

	apiState.inPhantomZone = (playerState.status & PlayerState::PhantomZoned) != 0;
	apiState.falling = (playerState.status & PlayerState::Falling) != 0;
	apiState.crossingWall = (playerState.status & PlayerState::CrossingWall) != 0;
	apiState.phydrv = (playerState.status & PlayerState::OnDriver) ? playerState.phydrv : -1;
	apiState.rotation = playerState.azimuth;
	apiState.angVel = playerState.angVel;
	memcpy(apiState.pos,playerState.pos,sizeof(float)*3);
	memcpy(apiState.velocity,playerState.velocity,sizeof(float)*3);
}

void APIStateToplayerState ( PlayerState &playerState, const bz_PlayerUpdateState &apiState )
{
	playerState.status = 0;
	switch(apiState.status)
	{
		case eAlive:
			playerState.status |= PlayerState::Alive;
			break;
		case eDead:
			playerState.status |= PlayerState::DeadStatus;
			break;
		case ePaused:
			playerState.status |= PlayerState::Paused;
			break;
		case eExploding:
			playerState.status |= PlayerState::Exploding;
			break;
		case eTeleporting:
			playerState.status |= PlayerState::Teleporting;
			break;

		case eInBuilding:
			playerState.status |= PlayerState::InBuilding;
			break;
	}

	if (apiState.inPhantomZone)
		playerState.status |=  PlayerState::PhantomZoned;

	if (apiState.falling)
		playerState.status |=  PlayerState::Falling;

	if (apiState.crossingWall)
		playerState.status |=  PlayerState::CrossingWall;

	if (apiState.phydrv != -1)
	{
		playerState.status |=  PlayerState::OnDriver;
		playerState.phydrv = apiState.phydrv;
	}

	playerState.azimuth = apiState.rotation;
	playerState.angVel = apiState.angVel;
	memcpy(playerState.pos,apiState.pos,sizeof(float)*3);
	memcpy(playerState.velocity,apiState.velocity,sizeof(float)*3);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
