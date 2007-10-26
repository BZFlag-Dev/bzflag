// rabidRabbit.cpp : Defines the entry point for the DLL application.

#include "bzfsAPI.h"
#include <string>
#include <vector>
#include <map>
#include <math.h>

BZ_GET_PLUGIN_VERSION class RabidRabbitHandler:public bz_CustomMapObjectHandler {
public:
  virtual bool handle(bz_ApiString object, bz_CustomMapObjectInfo * data);
};

RabidRabbitHandler rabidrabbithandler;

class RabidRabbitEventHandler:public bz_EventHandler {
public:
  virtual void process(bz_EventData * eventData);
};

RabidRabbitEventHandler rabidrabbiteventHandler;

class RabidRabbitDieEventHandler:public bz_EventHandler {
public:
  virtual void process(bz_EventData * eventData);
};

RabidRabbitDieEventHandler rabidrabbitdieeventhandler;

BZF_PLUGIN_CALL int
bz_Load(const char * /*commandLineParameter */ )
{
  bz_debugMessage(4, "rabidRabbit plugin loaded");
  bz_registerCustomMapObject("RABIDRABBITZONE", &rabidrabbithandler);
  bz_registerCustomMapObject("RRSOUNDOFF", &rabidrabbithandler);
  bz_registerCustomMapObject("RRCYCLEONDIE", &rabidrabbithandler);
  bz_registerEvent(bz_eTickEvent, &rabidrabbiteventHandler);
  bz_registerEvent(bz_ePlayerDieEvent, &rabidrabbitdieeventhandler);
  return 0;
}

BZF_PLUGIN_CALL int
bz_Unload(void)
{
  bz_removeEvent(bz_eTickEvent, &rabidrabbiteventHandler);
  bz_removeEvent(bz_ePlayerDieEvent, &rabidrabbitdieeventhandler);
  bz_removeCustomMapObject("RABIDRABBITZONE");
  bz_removeCustomMapObject("RRSOUNDOFF");
  bz_removeCustomMapObject("RRCYCLEONDIE");
  bz_debugMessage(4, "rabidRabbit plugin unloaded");
  return 0;
}

class RRZoneInfo {
public:
  RRZoneInfo() {
    currentKillZone = 0;
    rabbitNotifiedWrongZone = false;
    rabbitNotifiedWrongZoneNum = 0;
    soundEnabled = true;
    cycleOnDie = false;
  }
  int currentKillZone, rabbitNotifiedWrongZoneNum;
  bool rabbitNotifiedWrongZone, soundEnabled, cycleOnDie;
};

RRZoneInfo rrzoneinfo;

class RabidRabbitZone {
public:
  RabidRabbitZone() {
    zonekillhunter = false;
    box = false;
    xMax = xMin = yMax = yMin = zMax = zMin = rad = 0;
    WW = "";
    WWLifetime = 0;
    WWPosition[0] = 0;
    WWPosition[1] = 0;
    WWPosition[2] = 0;
    WWTilt = 0;
    WWDirection = 0;
    WWShotID = 0;
    WWDT = 0;
    WWRepeat = 0.5;
    WWFired = false;
    WWLastFired = 0;
    pi = 3.14159265358979323846;
  } bool zonekillhunter;
  bool box;
  float xMax, xMin, yMax, yMin, zMax, zMin;
  float rad;

  bz_ApiString WW;
  float WWLifetime, WWPosition[3], WWTilt, WWDirection, WWDT;
  double pi, WWLastFired, WWRepeat;
  bool WWFired;
  int WWShotID;

  std::string playermessage;
  std::string servermessage;

  bool pointIn(float pos[3]) {
    if (box) {
      if (pos[0] > xMax || pos[0] < xMin)
	return false;

      if (pos[1] > yMax || pos[1] < yMin)
	return false;

      if (pos[2] > zMax || pos[2] < zMin)
	return false;
    } else {
      float vec[3];
      vec[0] = pos[0] - xMax;
      vec[1] = pos[1] - yMax;
      vec[2] = pos[2] - zMax;

      float dist = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
      if (dist > rad)
	return false;

      if (pos[2] > zMax || pos[2] < zMin)
	return false;
    }
    return true;
  }
};

std::vector < RabidRabbitZone > zoneList;

bool RabidRabbitHandler::handle(bz_ApiString object, bz_CustomMapObjectInfo * data)
{
  if (object == "RRSOUNDOFF")
    rrzoneinfo.soundEnabled = false;

  if (object == "RRCYCLEONDIE")
    rrzoneinfo.cycleOnDie = true;

  if (object != "RABIDRABBITZONE" || !data)
    return false;

  RabidRabbitZone newZone;

  // parse all the chunks
  for (unsigned int i = 0; i < data->data.size(); i++) {
    std::string line = data->data.get(i).c_str();

    bz_APIStringList *nubs = bz_newStringList();
    nubs->tokenize(line.c_str(), " ", 0, true);

    if (nubs->size() > 0) {
      std::string key = bz_toupper(nubs->get(0).c_str());

      if (key == "BBOX" && nubs->size() > 6) {
	newZone.box = true;
	newZone.xMin = (float) atof(nubs->get(1).c_str());
	newZone.xMax = (float) atof(nubs->get(2).c_str());
	newZone.yMin = (float) atof(nubs->get(3).c_str());
	newZone.yMax = (float) atof(nubs->get(4).c_str());
	newZone.zMin = (float) atof(nubs->get(5).c_str());
	newZone.zMax = (float) atof(nubs->get(6).c_str());
      } else if (key == "CYLINDER" && nubs->size() > 5) {
	newZone.box = false;
	newZone.rad = (float) atof(nubs->get(5).c_str());
	newZone.xMax = (float) atof(nubs->get(1).c_str());
	newZone.yMax = (float) atof(nubs->get(2).c_str());
	newZone.zMin = (float) atof(nubs->get(3).c_str());
	newZone.zMax = (float) atof(nubs->get(4).c_str());
      } else if (key == "RRZONEWW" && nubs->size() > 10) {
	newZone.WW = nubs->get(1);
	newZone.WWLifetime = (float) atof(nubs->get(2).c_str());
	newZone.WWPosition[0] = (float) atof(nubs->get(3).c_str());
	newZone.WWPosition[1] = (float) atof(nubs->get(4).c_str());
	newZone.WWPosition[2] = (float) atof(nubs->get(5).c_str());
	newZone.WWTilt = (float) atof(nubs->get(6).c_str());
	newZone.WWTilt = (newZone.WWTilt / 360) * (2 * (float) newZone.pi);
	newZone.WWDirection = (float) atof(nubs->get(7).c_str());
	newZone.WWDirection = (newZone.WWDirection / 360) * (2 * (float) newZone.pi);
	newZone.WWShotID = (int) atoi(nubs->get(8).c_str());
	newZone.WWDT = (float) atof(nubs->get(9).c_str());
	newZone.WWRepeat = (float) atof(nubs->get(10).c_str());
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
  bz_setMaxWaitTime((float) 0.1);
  return true;
}

void
killAllHunters(std::string messagepass)
{
  bz_APIIntList *playerList = bz_newIntList();
  bz_getPlayerIndexList(playerList);


  for (unsigned int i = 0; i < playerList->size(); i++) {

    bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](i));

    if (player) {
      if (player->team != eRabbitTeam) {
	bz_killPlayer(player->playerID, true, BZ_SERVER);
	bz_sendTextMessage(BZ_SERVER, player->playerID, messagepass.c_str());
	if (rrzoneinfo.soundEnabled)
	  bz_sendPlayCustomLocalSound(player->playerID, "flag_lost");
      }
      if (player->team == eRabbitTeam && rrzoneinfo.soundEnabled && bz_getTeamCount(eHunterTeam) > 0)
	bz_sendPlayCustomLocalSound(player->playerID, "flag_won");

    }

    bz_freePlayerRecord(player);
  }

  bz_deleteIntList(playerList);

  return;
}


void
RabidRabbitEventHandler::process(bz_EventData * eventData)
{
  if ((eventData->eventType != bz_eTickEvent) || (zoneList.size() < 2))
    return;

  for (unsigned int i = 0; i < zoneList.size(); i++) {
    if (!zoneList[i].WWFired && rrzoneinfo.currentKillZone == i) {
      bz_fireWorldWep(zoneList[i].WW.c_str(), zoneList[i].WWLifetime, zoneList[i].WWPosition, zoneList[i].WWTilt,
		      zoneList[i].WWDirection, zoneList[i].WWShotID, zoneList[i].WWDT);
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
	if (zoneList[i].pointIn(player->currentState.pos) && player->spawned && player->team == eRabbitTeam
	    && rrzoneinfo.currentKillZone != i && !rrzoneinfo.rabbitNotifiedWrongZone) {
	  bz_sendTextMessage(BZ_SERVER, player->playerID, "You are not in the current Rabid Rabbit zone - try another.");
	  rrzoneinfo.rabbitNotifiedWrongZone = true;
	  rrzoneinfo.rabbitNotifiedWrongZoneNum = i;
	}

	if (!zoneList[i].pointIn(player->currentState.pos) && player->spawned && player->team == eRabbitTeam
	    && rrzoneinfo.rabbitNotifiedWrongZone && rrzoneinfo.rabbitNotifiedWrongZoneNum == i)
	  rrzoneinfo.rabbitNotifiedWrongZone = false;

	if (zoneList[i].pointIn(player->currentState.pos) && player->spawned && player->team == eRabbitTeam
	    && rrzoneinfo.currentKillZone == i && bz_getTeamCount(eHunterTeam) > 0) {
	  killAllHunters(zoneList[i].servermessage);

	  rrzoneinfo.rabbitNotifiedWrongZone = true;
	  rrzoneinfo.rabbitNotifiedWrongZoneNum = i;

	  if (i == (zoneList.size() - 1))
	    rrzoneinfo.currentKillZone = 0;
	  else
	    rrzoneinfo.currentKillZone++;

	  rrzoneinfo.rabbitNotifiedWrongZone = true;
	  rrzoneinfo.rabbitNotifiedWrongZoneNum = i;
	}

	if (zoneList[i].pointIn(player->currentState.pos) && player->spawned && player->team != eRabbitTeam
	    && zoneList[i].zonekillhunter) {
	  bz_killPlayer(player->playerID, true, BZ_SERVER);
	  bz_sendTextMessage(BZ_SERVER, player->playerID, zoneList[i].playermessage.c_str());
	}
      }
    }
    bz_freePlayerRecord(player);
  }

  bz_deleteIntList(playerList);
  return;
}

void
RabidRabbitDieEventHandler::process(bz_EventData * eventData)
{
  if (eventData->eventType != bz_ePlayerDieEvent)
    return;

  bz_PlayerDieEventData_V1 *DieData = (bz_PlayerDieEventData_V1 *) eventData;

  if (rrzoneinfo.cycleOnDie && DieData->team == eRabbitTeam) {
    int i = rrzoneinfo.currentKillZone;
    if (i == (zoneList.size() - 1))
      rrzoneinfo.currentKillZone = 0;
    else
      rrzoneinfo.currentKillZone++;
  }

  return;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
