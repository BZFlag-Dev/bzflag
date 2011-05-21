/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// commonItems.cpp : Defines the entry point for the DLL application.
//

#include "commonItems.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <string>

UserInfo* userInfo = NULL;
ServerError* serverError = NULL;
GameInfo* gameInfo = NULL;

void initCommonItems(Templateiser& ts) {
  userInfo = new UserInfo(ts);
  serverError = new ServerError(ts);
  gameInfo = new GameInfo(ts);
}

void freeCommonItems(void) {
  delete(userInfo);
  delete(serverError);
  delete(gameInfo);

  userInfo = NULL;
  serverError = NULL;
  gameInfo = NULL;
}


std::vector<NewPageCallback*> newPageCallbacks;

void addNewPageCallback(NewPageCallback* callback) {
  if (callback) {
    newPageCallbacks.push_back(callback);
  }
}

void removeNewPageCallback(NewPageCallback* callback) {
  std::vector<NewPageCallback*> ::iterator itr = newPageCallbacks.begin();
  while (itr != newPageCallbacks.end()) {
    if (*itr == callback) {
      newPageCallbacks.erase(itr);
      return;
    }
    itr++;
  }
}

void callNewPageCallbacks(const std::string& pagename, const HTTPRequest& request) {
  std::vector<NewPageCallback*> ::iterator itr = newPageCallbacks.begin();
  while (itr != newPageCallbacks.end()) {
    (*itr++)->newPage(pagename, request);
  }
}

//----------------UserInfo
UserInfo::UserInfo(Templateiser& ts) {
  sessionAuth = NULL;
  sessionID = -1;
  ts.addKey("username", this);

  ts.addIF("hasperm", this);

}

void UserInfo::keyCallback(std::string& data, const std::string& /*key*/) {
  data += userName;
}

bool UserInfo::ifCallback(const std::string& key) {
  if (sessionAuth && key == "hasperm" && templateParam.size()) {
    return sessionAuth->getSessionPermision(sessionID, templateParam);
  }

  return false;
}

//--------------Error
ServerError::ServerError(Templateiser& ts) {
  ts.addKey("error", this);
  ts.addIF("error", this);
}

void ServerError::keyCallback(std::string& data, const std::string& /*key*/) {
  data += errorMessage;
}

bool ServerError::ifCallback(const std::string& /*key*/) {
  return errorMessage.size() != 0;
}

//--------------GameInfo

GameInfo::GameInfo(Templateiser& ts) {
  ts.addKey("MapFile", this);
  ts.addKey("RedTeamScore", this);
  ts.addKey("RedTeamWins", this);
  ts.addKey("RedTeamLosses", this);
  ts.addKey("GreenTeamScore", this);
  ts.addKey("GreenTeamWins", this);
  ts.addKey("GreenTeamLosses", this);
  ts.addKey("BlueTeamScore", this);
  ts.addKey("BlueTeamWins", this);
  ts.addKey("BlueTeamLosses", this);
  ts.addKey("PurpleTeamScore", this);
  ts.addKey("PurpleTeamWins", this);
  ts.addKey("PurpleTeamLosses", this);

  ts.addKey("KBIn", this);
  ts.addKey("KBOut", this);

  bz_registerEvent(bz_eGetWorldEvent, this);
  bz_registerEvent(bz_eNetDataReceiveEvent, this);
  bz_registerEvent(bz_eNetDataSendEvent, this);

  startTime = bz_getCurrentTime();
  bytesIn = 0;
  bytesOut = 0;

  mapFile = "Unknown";
}

GameInfo::~GameInfo(void) {
  bz_removeEvent(bz_eGetWorldEvent, this);
  bz_removeEvent(bz_eNetDataReceiveEvent, this);
  bz_removeEvent(bz_eNetDataSendEvent, this);
}


void GameInfo::keyCallback(std::string& data, const std::string& key) {
  double now = bz_getCurrentTime();

  if (key == "mapfile") {
    data += mapFile;
  }
  else if (key == "redteamscore") {
    data += format("%d", bz_getTeamScore(eRedTeam));
  }
  else if (key == "redteamwins") {
    data += format("%d", bz_getTeamWins(eRedTeam));
  }
  else if (key == "redteamlosses") {
    data += format("%d", bz_getTeamLosses(eRedTeam));
  }
  else if (key == "greenteamscore") {
    data += format("%d", bz_getTeamScore(eGreenTeam));
  }
  else if (key == "greenteamwins") {
    data += format("%d", bz_getTeamWins(eGreenTeam));
  }
  else if (key == "greenteamlosses") {
    data += format("%d", bz_getTeamLosses(eGreenTeam));
  }
  else if (key == "blueteamscore") {
    data += format("%d", bz_getTeamScore(eBlueTeam));
  }
  else if (key == "blueteamwins") {
    data += format("%d", bz_getTeamWins(eBlueTeam));
  }
  else if (key == "blueteamlosses") {
    data += format("%d", bz_getTeamLosses(eBlueTeam));
  }
  else if (key == "purpleteamscore") {
    data += format("%d", bz_getTeamScore(ePurpleTeam));
  }
  else if (key == "purpleteamwins") {
    data += format("%d", bz_getTeamWins(ePurpleTeam));
  }
  else if (key == "purpleteamlosses") {
    data += format("%d", bz_getTeamLosses(ePurpleTeam));
  }
  else if (key == "kbin") {
    data += format("%f", (double)bytesIn / 1024.0);
  }
  else if (key == "kbout") {
    data += format("%f", (double)bytesOut / 1024.0);
  }
}

void GameInfo::process(bz_EventData* eventData) {
  switch (eventData->eventType) {
    case bz_eGetWorldEvent: {
      bz_GetWorldEventData_V1* data = (bz_GetWorldEventData_V1*)eventData;
      mapFile = data->worldFile.c_str();
      if (!mapFile.size()) {
        if (data->worldBlob) {
          mapFile = "Custom";
        }
        else {
          mapFile = "Random";
        }
      }
      break;
    }

    case bz_eNetDataReceiveEvent:
      bytesIn += ((bz_NetTransferEventData_V1*)eventData)->iSize;
      break;

    case bz_eNetDataSendEvent:
      bytesOut += ((bz_NetTransferEventData_V1*)eventData)->iSize;
      break;
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
