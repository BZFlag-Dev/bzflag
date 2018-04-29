/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "bzfsAPI.h"
#include <map>
#include <cmath>

class WWZEventHandler : public bz_Plugin, bz_CustomMapObjectHandler
{
public:
  virtual const char* Name() {return "World Weapon Zones";}
  virtual void Init(const char* config);
  virtual void Cleanup();
  virtual void Event(bz_EventData *eventData);

  virtual bool MapObject (bz_ApiString object, bz_CustomMapObjectInfo *data);
};

BZ_PLUGIN(WWZEventHandler)

void WWZEventHandler::Init (const char* /*commandLineParameter*/)
{
  // Register our custom world object
  bz_registerCustomMapObject("WWZONE", this);

  // Register our events
  Register(bz_eTickEvent);

  // Set an initial minimum wait time of 0.5 seconds
  MaxWaitTime = 0.5f;
}

void WWZEventHandler::Cleanup (void)
{
  // Remove our events
  Flush();

  // Remove our custom world object
  bz_removeCustomMapObject("WWZONE");
}

class WWZPlyrInfo
{
public:
  WWZPlyrInfo()
  {
    wwzplyrID = -1;
    wwzPlyrInTime = 0;
  }

  int wwzplyrID;
  double wwzPlyrInTime;
};

WWZPlyrInfo wwzNewPlyr;

class WWZone : public bz_CustomZoneObject
{
public:
  WWZone() : bz_CustomZoneObject()
  {
    zoneWeapon = "";
    zoneWeaponLifetime = 0;
    zoneWeaponPosition[0] = 0;
    zoneWeaponPosition[1] = 0;
    zoneWeaponPosition[2] = 0;
    zoneWeaponTilt = 0;
    zoneWeaponDirection = 0;
    zoneWeaponShotID = 0;
    zoneWeaponRepeat = false;
    zoneWeaponFired = false;
    zoneWeaponLastFired = 0;
    zoneWeaponMinFireTime = 0.5;
    zoneWeaponTimeDelay = 0;
    zoneWeaponInfoMessage = false;
    zoneWeaponSentMessage = false;
    wwzPlyrList.clear();
  }

  std::vector <WWZPlyrInfo> wwzPlyrList;

  bz_ApiString zoneWeapon;
  float zoneWeaponLifetime, zoneWeaponPosition[3], zoneWeaponTilt, zoneWeaponDirection;
  double zoneWeaponMinFireTime, zoneWeaponTimeDelay, zoneWeaponLastFired;
  bool zoneWeaponRepeat, zoneWeaponInfoMessage, zoneWeaponFired, zoneWeaponSentMessage;
  int zoneWeaponShotID;

  std::string playermessage;
  std::string servermessage;
};

std::vector <WWZone> zoneList;

bool WWZEventHandler::MapObject (bz_ApiString object, bz_CustomMapObjectInfo *data)
{
  if (object != "WWZONE" || !data)
    return false;

  WWZone newZone;
  newZone.handleDefaultOptions(data);

  // parse all the chunks
  for (unsigned int i = 0; i < data->data.size(); i++) {
    std::string line = data->data.get(i).c_str();

    bz_APIStringList *nubs = bz_newStringList();
    nubs->tokenize(line.c_str(), " ", 0, true);

    if (nubs->size() > 0) {
      std::string key = bz_toupper(nubs->get(0).c_str());

      if (key == "ZONEWEAPON" && nubs->size() > 9) {
	newZone.zoneWeapon = nubs->get(1);
	newZone.zoneWeaponLifetime = (float)atof(nubs->get(2).c_str());
	newZone.zoneWeaponPosition[0] = (float)atof(nubs->get(3).c_str());
	newZone.zoneWeaponPosition[1] = (float)atof(nubs->get(4).c_str());
	newZone.zoneWeaponPosition[2] = (float)atof(nubs->get(5).c_str());
	newZone.zoneWeaponTilt = (float)(atof(nubs->get(6).c_str()) * M_PI/180.0);	// convert degrees to radians
	newZone.zoneWeaponDirection = (float)(atof(nubs->get(7).c_str()) * M_PI/180.0);	// convert degrees to radians
	newZone.zoneWeaponShotID = (int)atoi(nubs->get(8).c_str());
      } else if (key == "REPEAT" && nubs->size() == 1) {
	newZone.zoneWeaponRepeat = true;
      } else if (key == "REPEAT" && nubs->size() > 1) {
	newZone.zoneWeaponRepeat = true;
	newZone.zoneWeaponMinFireTime = (double)atof(nubs->get(1).c_str());
	if (newZone.zoneWeaponMinFireTime < MaxWaitTime && newZone.zoneWeaponMinFireTime >= 0.1)
	  MaxWaitTime = (float)newZone.zoneWeaponMinFireTime - 0.05f;	// tick faster than ww calls
      } else if (key == "TIMEDELAY" && nubs->size() > 1) {
	newZone.zoneWeaponTimeDelay = (double)atof(nubs->get(1).c_str());
	if (newZone.zoneWeaponTimeDelay < 0)
	  newZone.zoneWeaponTimeDelay = 0;
      } else if (key == "PLAYERMESSAGE" && nubs->size() > 1) {
	newZone.playermessage = nubs->get(1).c_str();
      } else if (key == "SERVERMESSAGE" && nubs->size() > 1) {
	newZone.servermessage = nubs->get(1).c_str();
      } else if (key == "INFOMESSAGE") {
	newZone.zoneWeaponInfoMessage = true;
      }
    }
    bz_deleteStringList(nubs);
  }
  zoneList.push_back(newZone);
  return true;
}

inline bool wasHere(int zoneNum, int plyrNum)
{
  for (unsigned int j = 0; j < zoneList[zoneNum].wwzPlyrList.size(); j++) {
    if (plyrNum == zoneList[zoneNum].wwzPlyrList[j].wwzplyrID)
      return true;
  }

  wwzNewPlyr.wwzplyrID = plyrNum;
  wwzNewPlyr.wwzPlyrInTime = bz_getCurrentTime();

  zoneList[zoneNum].wwzPlyrList.push_back(wwzNewPlyr);
  zoneList[zoneNum].zoneWeaponSentMessage = false;
  zoneList[zoneNum].zoneWeaponFired = false;

  return false;
}

inline void notHere(int zoneNum, int plyrNum)
{
  for (unsigned int j = 0; j < zoneList[zoneNum].wwzPlyrList.size(); j++) {
    if (plyrNum == zoneList[zoneNum].wwzPlyrList[j].wwzplyrID) {
      zoneList[zoneNum].wwzPlyrList.erase(zoneList[zoneNum].wwzPlyrList.begin() + j);
      zoneList[zoneNum].zoneWeaponFired = false;
      zoneList[zoneNum].zoneWeaponSentMessage = false;
      return;
    }
  }
}

inline bool OKToFire(int zoneNum, int plyrNum)
{
  for (unsigned int j = 0; j < zoneList[zoneNum].wwzPlyrList.size(); j++) {
    if (zoneList[zoneNum].wwzPlyrList[j].wwzplyrID == plyrNum) {
      if ((bz_getCurrentTime() - zoneList[zoneNum].wwzPlyrList[j].wwzPlyrInTime) > zoneList[zoneNum].zoneWeaponTimeDelay && !zoneList[zoneNum].zoneWeaponFired) {
	zoneList[zoneNum].wwzPlyrList[j].wwzPlyrInTime = bz_getCurrentTime();
	return true;
      }
    }
  }

  return false;
}

void WWZEventHandler::Event (bz_EventData *eventData)
{
  if (eventData->eventType != bz_eTickEvent)
    return;

  bz_APIIntList *playerList = bz_newIntList();
  bz_getPlayerIndexList (playerList);

  for (unsigned int h = 0; h < playerList->size(); h++) {
    bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](h));

    if (player) {
      for (unsigned int i = 0; i < zoneList.size(); i++) {
	if (zoneList[i].pointInZone(player->lastKnownState.pos) && player->spawned) {
	  if (wasHere(i, player->playerID) && OKToFire(i, player->playerID) && !zoneList[i].zoneWeaponFired) {
	    float vector[3];
	    bz_vectorFromRotations(zoneList[i].zoneWeaponTilt, zoneList[i].zoneWeaponDirection, vector);
	    bz_fireServerShot(zoneList[i].zoneWeapon.c_str(), zoneList[i].zoneWeaponPosition, vector);
	    zoneList[i].zoneWeaponFired = true;
	    zoneList[i].zoneWeaponLastFired = bz_getCurrentTime();
	  } else {
	    if ((bz_getCurrentTime() - zoneList[i].zoneWeaponLastFired) > zoneList[i].zoneWeaponMinFireTime && zoneList[i].zoneWeaponRepeat)
	      zoneList[i].zoneWeaponFired = false;
	  }

	  if (!zoneList[i].zoneWeaponSentMessage && zoneList[i].zoneWeaponFired) {
	    if (!zoneList[i].playermessage.empty())
	      bz_sendTextMessage(BZ_SERVER, player->playerID, zoneList[i].playermessage.c_str());
	    if (!zoneList[i].servermessage.empty())
	      bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, zoneList[i].servermessage.c_str());
	    if (zoneList[i].zoneWeaponInfoMessage)
	      bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "%s triggered by %s.", zoneList[i].zoneWeapon.c_str(), player->callsign.c_str());

	    zoneList[i].zoneWeaponSentMessage = true;
	  }
	} else {
	  notHere(i, player->playerID);
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
