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

// bzflag global header
#include "bzfsMessages.h"
#include <assert.h>

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

				bool hide = (flag.flag.type->flagTeam == ::NoTeam) && (flag.player == -1);
				
				flagRecord->index = flagIndex;

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
			bz_PlayerUpdateRecord	playerRecord;

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

			playerData->playerHandler->playerUpdate(&playerRecord);
		}
		else
		{
			if (sendPlayerUpdateDirect(playerData->netHandler,otherData) < 0)
				break;
		}
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

void sendHandycapInfoUpdate (int playerID )
{
	GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerID);
	if (!playerData)
		return;

	GameKeeper::Player *otherData;
	if (clOptions->gameStyle & HandicapGameStyle) 
	{
		if (playerData->playerHandler)
		{
			std::vector<bz_HandycapUpdateRecord*>	handyList;
			for (int i = 0; i < curMaxPlayers; i++)
			{
				otherData = GameKeeper::Player::getPlayerByIndex(i);
				if (otherData)
				{
					bz_HandycapUpdateRecord *handyData = new bz_HandycapUpdateRecord;
					handyData->player = i;
					handyData->handycap = otherData->score.getHandicap();
					handyList.push_back(handyData);
				}

				bz_HandycapUpdateRecord** handyPtrList = (bz_HandycapUpdateRecord**)malloc(sizeof(bz_HandycapUpdateRecord*)*handyList.size());
				for ( int i = 0; i < (int)handyList.size(); i++)
					handyPtrList[i] = handyList[i];

				playerData->playerHandler->handycapUpdate((int)handyList.size(),handyPtrList);

				free(handyPtrList);
				for ( int i = 0; i < (int)handyList.size(); i++)
					delete(handyList[i]);
			}
		}
		else
		{
			int numHandicaps = 0;

			// Send handicap for all players
			void *bufStart = getDirectMessageBuffer();
			void *buf = nboPackUByte(bufStart, numHandicaps);
			for (int i = 0; i < curMaxPlayers; i++)
			{
				if (i != playerID)
				{
					otherData = GameKeeper::Player::getPlayerByIndex(i);
					if (otherData)
					{
						numHandicaps++;
						buf = nboPackUByte(buf, i);
						buf = nboPackShort(buf, otherData->score.getHandicap());
					}
				}
			}
			nboPackUByte(bufStart, numHandicaps);
			broadcastMessage(MsgHandicap, (char*)buf-(char*)bufStart, bufStart);
		}
	}
}

void sendAdminInfoMessage ( int aboutPlayer, int toPlayer, bool record )
{
	GameKeeper::Player *aboutPlayerData = NULL;
	GameKeeper::Player *toPlayerData = NULL;

	aboutPlayerData = GameKeeper::Player::getPlayerByIndex(aboutPlayer);
	if (!aboutPlayerData)
		return;

	if (!record)
	{
		toPlayerData = GameKeeper::Player::getPlayerByIndex(toPlayer);
		if (!toPlayerData)
			return;

		if (toPlayerData->playerHandler)
		{
			if (!aboutPlayerData->netHandler)
				toPlayerData->playerHandler->playeIPUpdate (aboutPlayer,"local.player");
			else
				toPlayerData->playerHandler->playeIPUpdate (aboutPlayer,aboutPlayerData->netHandler->getTargetIP());
		}
	}
	
	if (toPlayer || record)
	{
		void *buf, *bufStart = getDirectMessageBuffer();
		buf = nboPackUByte(bufStart, 1);
		buf = aboutPlayerData->packAdminInfo(buf);
		if (!toPlayerData->playerHandler)
			directMessage(toPlayer, MsgAdminInfo,(char*)buf - (char*)bufStart, bufStart);
		
		if (record)
			Record::addPacket(MsgAdminInfo,(char*)buf - (char*)bufStart, bufStart, HiddenPacket);
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
	for (int t = 0; t < CtfTeams; t++) {
		buf = nboPackUShort(buf, t);
		buf = team[t].team.pack(buf);
	}
	return directMessage(handler, MsgTeamUpdate,
		(char*)buf - (char*)bufStart, bufStart);
}

void sendWorldChunk(NetHandler *handler, uint32_t ptr)
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

//messages sent TO the server
void getGeneralMessageInfo ( void **buffer, uint16_t &code, uint16_t &len )
{
	*buffer = nboUnpackUShort(*buffer, len);
	*buffer = nboUnpackUShort(*buffer, code);
}

GameKeeper::Player *getPlayerMessageInfo ( void **buffer, uint16_t &code, int &playerID )
{
	switch (code)
	{
		case MsgEnter:
		case MsgExit:
		case MsgAlive:
		case MsgKilled:
		case MsgGrabFlag:
		case MsgDropFlag:
		case MsgCaptureFlag:
		case MsgShotEnd:
		case MsgHit:
		case MsgTeleport:
		case MsgMessage:
		case MsgTransferFlag:
		case MsgPause:
		case MsgAutoPilot:
		case MsgLagPing:
		case MsgKrbPrincipal:
		case MsgKrbTicket:
		case MsgNewRabbit:
		case MsgPlayerUpdate:
		case MsgPlayerUpdateSmall: 
			uint8_t id;
			*buffer  = nboUnpackUByte(*buffer, id);
			playerID = id;
			return GameKeeper::Player::getPlayerByIndex(playerID);
	}
	return NULL;
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

//utils
bool isUDPAtackMessage ( uint16_t &code )
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
			return false;
	}
	return true;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
