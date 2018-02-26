// rabidRabbit.cpp : Defines the entry point for the DLL application.

#include "bzfsAPI.h"
#include <map>
#include <cmath>

class RabidRabbitHandler : public bz_CustomMapObjectHandler
{
public:
  virtual bool MapObject(bz_ApiString object, bz_CustomMapObjectInfo *data);
};

RabidRabbitHandler rabidrabbithandler;

class RabidRabbitEventHandler : public bz_Plugin
{
public:
  virtual const char* Name() {return "Rabid Rabbit";}
  virtual void Init(const char* config);
  virtual void Cleanup();

  virtual void Event(bz_EventData *eventData);

private:
  unsigned int currentKillZone;
  unsigned int rabbitNotifiedWrongZoneNum;
  bool rabbitNotifiedWrongZone;
};

BZ_PLUGIN(RabidRabbitEventHandler)


void RabidRabbitEventHandler::Init(const char* /*commandLineParameter*/)
{
  MaxWaitTime = 1.0f;
  bz_registerCustomMapObject("RABIDRABBITZONE",&rabidrabbithandler);
  Register(bz_eTickEvent);
  Register(bz_ePlayerDieEvent);

  currentKillZone = 0;
  rabbitNotifiedWrongZone = false;
  rabbitNotifiedWrongZoneNum = 0;
}

void RabidRabbitEventHandler::Cleanup(void)
{
  Flush();
  bz_removeCustomMapObject("RABIDRABBITZONE");
}

class RabidRabbitZone : public bz_CustomZoneObject
{
public:
  RabidRabbitZone()
  {
    zonekillhunter = false;
    WW = "";
    WWLifetime = 0;
    WWPosition[0] = 0;
    WWPosition[1] = 0;
    WWPosition[2] = 0;
    WWTilt = 0;
    WWDirection = 0;
    WWShotID = 0;
    WWRepeat = 0.5;
    WWFired = false;
    WWLastFired = 0;
  }

  bool zonekillhunter;

  bz_ApiString WW;
  float WWLifetime, WWPosition[3], WWTilt, WWDirection;
  double WWLastFired, WWRepeat;
  bool WWFired;
  int WWShotID;

  std::string playermessage;
  std::string servermessage;
};

std::vector <RabidRabbitZone> zoneList;

bool RabidRabbitHandler::MapObject(bz_ApiString object, bz_CustomMapObjectInfo *data)
{
  if (object != "RABIDRABBITZONE" || !data)
    return false;

  RabidRabbitZone newZone;
  newZone.handleDefaultOptions(data);

  // parse all the chunks
  for (unsigned int i = 0; i < data->data.size(); i++) {
    std::string line = data->data.get(i).c_str();

    bz_APIStringList *nubs = bz_newStringList();
    nubs->tokenize(line.c_str()," ",0,true);

    if (nubs->size() > 0) {
      std::string key = bz_toupper(nubs->get(0).c_str());

      if (key == "RRZONEWW" && nubs->size() > 10) {
	newZone.WW = nubs->get(1);
	newZone.WWLifetime = (float)atof(nubs->get(2).c_str());
	newZone.WWPosition[0] = (float)atof(nubs->get(3).c_str());
	newZone.WWPosition[1] = (float)atof(nubs->get(4).c_str());
	newZone.WWPosition[2] = (float)atof(nubs->get(5).c_str());
	newZone.WWTilt = (float)(atof(nubs->get(6).c_str()) * M_PI/180.0);	// convert degrees to radians
	newZone.WWDirection = (float)(atof(nubs->get(7).c_str()) * M_PI/180.0);	// convert degrees to radians
	newZone.WWShotID = (int)atoi(nubs->get(8).c_str());
	newZone.WWRepeat = (float)atof(nubs->get(10).c_str());
      } else if (key == "SERVERMESSAGE" && nubs->size() > 1) {
	newZone.servermessage = nubs->get(1).c_str();
      } else if (key == "ZONEKILLHUNTER") {
	if (nubs->size() > 1)
	  newZone.playermessage = nubs->get(1).c_str();
	newZone.zonekillhunter = true;
      }
    }
    bz_deleteStringList(nubs);
  }

  zoneList.push_back(newZone);
  return true;
}

void killAllHunters(std::string messagepass)
{
  bz_APIIntList *playerList = bz_newIntList();
  bz_getPlayerIndexList(playerList);


  for (unsigned int i = 0; i < playerList->size(); i++) {

    bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](i));

    if (player) {
      if (player->team != eRabbitTeam) {
	bz_killPlayer(player->playerID, true, BZ_SERVER);
	bz_sendTextMessage(BZ_SERVER, player->playerID, messagepass.c_str());
      }
      bz_freePlayerRecord(player);
    }
  }

  bz_deleteIntList(playerList);

  return;
}

void RabidRabbitEventHandler::Event(bz_EventData *eventData)
{

  if (eventData->eventType == bz_ePlayerDieEvent)
  {
    bz_PlayerDieEventData_V1 *DieData = (bz_PlayerDieEventData_V1*)eventData;

    if (bz_getBZDBBool("_rrCycleOnDeath") && DieData->team == eRabbitTeam) {
      unsigned int i = currentKillZone;
      if (i == (zoneList.size() - 1))
	currentKillZone = 0;
      else
	currentKillZone++;
    }
    return;
  }

  if ((eventData->eventType != bz_eTickEvent) || (zoneList.size() < 2))
    return;

  for (unsigned int i = 0; i < zoneList.size(); i++) {
    if (!zoneList[i].WWFired && currentKillZone == i) {
      float vector[3];
      bz_vectorFromRotations(zoneList[i].WWTilt, zoneList[i].WWDirection, vector);
      bz_fireServerShot(zoneList[i].WW.c_str(), zoneList[i].WWPosition, vector);
      zoneList[i].WWFired = true;
      zoneList[i].WWLastFired = bz_getCurrentTime();
    } else {
      if ((bz_getCurrentTime() - zoneList[i].WWLastFired) > zoneList[i].WWRepeat)
	zoneList[i].WWFired = false;
    }
  }

  bz_APIIntList *playerList = bz_newIntList();
  bz_getPlayerIndexList(playerList);

  for (unsigned int h = 0; h < playerList->size(); h++) {
    bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](h));

    if (player) {

      for (unsigned int i = 0; i < zoneList.size(); i++) {
	if (zoneList[i].pointInZone(player->lastKnownState.pos) &&
	    player->spawned && player->team == eRabbitTeam &&
	    currentKillZone != i && !rabbitNotifiedWrongZone) {
	  bz_sendTextMessage(BZ_SERVER,player->playerID,
			     "You are not in the current Rabid Rabbit zone - try another.");
	  rabbitNotifiedWrongZone = true;
	  rabbitNotifiedWrongZoneNum = i;
	}

	if (!zoneList[i].pointInZone(player->lastKnownState.pos) &&
	    player->spawned && player->team == eRabbitTeam &&
	    rabbitNotifiedWrongZone &&
	    rabbitNotifiedWrongZoneNum == i)
	  rabbitNotifiedWrongZone = false;

	if (zoneList[i].pointInZone(player->lastKnownState.pos) &&
	    player->spawned &&
	    player->team == eRabbitTeam &&
	    currentKillZone == i &&
	    bz_getTeamCount(eHunterTeam) > 0) {
	  killAllHunters(zoneList[i].servermessage);

	  rabbitNotifiedWrongZone = true;
	  rabbitNotifiedWrongZoneNum = i;

	  if (i == (zoneList.size() - 1))
	    currentKillZone = 0;
	  else
	    currentKillZone++;

	  rabbitNotifiedWrongZone = true;
	  rabbitNotifiedWrongZoneNum = i;
	}

	if (zoneList[i].pointInZone(player->lastKnownState.pos) &&
	    player->spawned &&
	    player->team != eRabbitTeam &&
	    zoneList[i].zonekillhunter) {
	  bz_killPlayer(player->playerID, true, BZ_SERVER);
	  bz_sendTextMessage (BZ_SERVER, player->playerID, zoneList[i].playermessage.c_str());
	}
      }
      bz_freePlayerRecord(player);
    }
  }

  bz_deleteIntList(playerList);
  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
