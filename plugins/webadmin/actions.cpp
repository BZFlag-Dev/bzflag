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

// actions.cpp : Defines the entry point for the DLL application.
//

#include "actions.h"
#include "loops.h"
#include "commonItems.h"

#include <fstream>
#include <cstring>
#include <algorithm>

#include <map>
#include <vector>
#include <string>

bz_BasePlayerRecord* getPlayerFromGUID(const std::string GUID) {
  std::vector<std::string> parts = tokenize(GUID, std::string("_"), 2, false);
  if (parts.size() < 2) {
    return NULL;
  }

  int slot = atoi(parts[0].c_str());

  bz_BasePlayerRecord* rec = bz_getPlayerByIndex(slot);
  if (!rec) {
    return NULL;
  }

  if (rec->callsign == parts[1]) {
    return rec;
  }

  bz_freePlayerRecord(rec);
  return NULL;
}

bool UpdateBZDBVars::process(std::string& inputPage, const HTTPRequest& request, HTTPReply& reply) {
  std::map<std::string, std::vector<std::string> >::const_iterator itr = request.parameters.begin();

  if (!userInfo->hasPerm("setVar")) {
    serverError->errorMessage = "Update BZDB Var: Invalid Permission";
    return false;
  }

  if (!inputPage.size()) {
    inputPage = "Vars";
  }

  while (itr != request.parameters.end()) {
    const std::string& key = itr->first;
    if (itr->second.size()) {
      // vars only use the first param with the name.
      const std::string val = url_decode(itr->second[0]);
      if (strncmp(key.c_str(), "var", 3) == 0) {
        // it's a var,
        if (varChanged(key.c_str() + 3, val.c_str())) {
          bz_updateBZDBString(key.c_str() + 3, val.c_str());
        }
      }
    }
    itr++;
  }
  return false;
}


bool UpdateBZDBVars::varChanged(const char* key , const char* val) {
  if (!bz_BZDBItemExists(key)) {
    return false;
  }
  return bz_getBZDBString(key) != val;
}

bool SendChatMessage::process(std::string& inputPage, const HTTPRequest& request, HTTPReply& reply) {
  if (!userInfo->hasPerm("say")) {
    serverError->errorMessage = "Send Chat Message: Invalid Permission";
    return false;
  }

  std::string message;

  if (request.getParam("message", message) && message.size()) {
    bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, message.c_str());
  }
  return false;
}

bool SaveLogFile::process(std::string& inputPage, const HTTPRequest& request, HTTPReply& reply) {
  if (!logLoop) {
    return false;
  }

  if (!userInfo->hasPerm("viewReports") || !userInfo->hasPerm("playerList")) {
    serverError->errorMessage = "Save Log File: Invalid Permission";
    return false;
  }

  std::string logFile;
  logLoop->getLogAsFile(logFile);
  if (!logFile.size()) {
    return false;
  }

  reply.body = logFile;
  reply.docType = HTTPReply::eText;
  reply.headers["Content-Disposition"] = "attachment; filename=\"logs.txt\"";
  return true;
}

bool ClearLogFile::process(std::string& inputPage, const HTTPRequest& request, HTTPReply& reply) {
  if (!logLoop) {
    return false;
  }

  if (!userInfo->hasPerm("viewReports") || !userInfo->hasPerm("playerList")) {
    serverError->errorMessage = "Clear Log File: Invalid Permission";
    return false;
  }

  if (serverError) {
    serverError->errorMessage = "Log File Cleared";
  }

  logLoop->clearLog();
  return false;
}

bool KickUser::process(std::string& inputPage, const HTTPRequest& request, HTTPReply& reply) {
  if (!userInfo->hasPerm("kick")) {
    serverError->errorMessage = "Kick: Invalid Permission";
    return false;
  }

  std::string GUID;

  if (request.getParam("guid", GUID) && GUID.size()) {
    bz_BasePlayerRecord* player = getPlayerFromGUID(GUID);
    if (!player) {
      serverError->errorMessage = "Kick: Invalid User";
    }
    else {
      int playerID = player->playerID;
      bz_freePlayerRecord(player);
      std::string reason;
      if (!request.getParam("kickreason", reason) || !reason.size()) {
        reason = "Webkicked by " + userInfo->userName;
      }
      bz_kickUser(playerID, reason.c_str(), true);

      serverError->errorMessage = "Kick: Sucsessful";
    }
  }

  return false;
}

bool RemoveBan::process(std::string& inputPage, const HTTPRequest& request, HTTPReply& reply) {
  if (!userInfo->hasPerm("unban")) {
    serverError->errorMessage = "RemoveBan: Invalid Permission";
    return false;
  }

  std::string banID;
  std::string banList;

  if (request.getParam("banid", banID) && banID.size() && request.getParam("bantype", banList) && banList.size()) {
    bz_eBanListType banType = eIPList;
    if (compare_nocase(banList, std::string("ipban")) == 0) {
      banType = eIPList;
    }
    else if (compare_nocase(banList, std::string("idban")) == 0) {
      banType = eIDList;
    }
    else if (compare_nocase(banList, std::string("hostban")) == 0) {
      banType = eHostList;
    }
    else {
      return false;
    }

    unsigned int id = (unsigned int)atoi(banID.c_str());
    if (id >= bz_getBanListSize(banType)) {
      return false;
    }

    if (bz_getBanItemIsFromMaster(banType, id)) {
      return false;
    }

    switch (banType) {
      case eIPList:
        bz_IPUnbanUser(bz_getBanItem(banType, id));
        break;

      case eIDList:
        bz_IDUnbanUser(bz_getBanItem(banType, id));
        break;

      case eHostList:
        bz_HostUnbanUser(bz_getBanItem(banType, id));
        break;

    }
  }
  return false;
}

bool AddBan::process(std::string& inputPage, const HTTPRequest& request, HTTPReply& reply) {
  if (!userInfo->hasPerm("ban")) {
    serverError->errorMessage = "AddBan: Invalid Permission";
    return false;
  }

  std::string banList;

  if (request.getParam("bantype", banList) && banList.size()) {
    bz_eBanListType banType = eIPList;
    if (compare_nocase(banList, std::string("ipban")) == 0) {
      banType = eIPList;
    }
    else if (compare_nocase(banList, std::string("idban")) == 0) {
      banType = eIDList;
    }
    else if (compare_nocase(banList, std::string("hostban")) == 0) {
      banType = eHostList;
    }
    else {
      return false;
    }

    std::string val, reason, source, len;

    if (!request.getParam("banvalue", val) || !val.size()) {
      return false;
    }

    request.getParam("banreason", reason);
    request.getParam("banlen", len);

    source = userInfo->userName + "(Webadmin)";
    if (!len.size()) {
      len = "-1";
    }
    if (!reason.size()) {
      reason = "WebBan";
    }

    switch (banType) {
      case eIPList:
        bz_IPBanUser(val.c_str(), source.c_str(), atoi(len.c_str()), reason.c_str());
        break;

      case eIDList:
        bz_IDBanUser(val.c_str(), source.c_str(), atoi(len.c_str()), reason.c_str());
        break;

      case eHostList:
        bz_HostBanUser(val.c_str(), source.c_str(), atoi(len.c_str()), reason.c_str());
        break;
    }
  }
  return false;
}

bool RemoveReport::process(std::string& inputPage, const HTTPRequest& request, HTTPReply& reply) {
  if (!userInfo->hasPerm("viewreports")) {
    serverError->errorMessage = "RemoveReport: Invalid Permission";
    return false;
  }

  std::string reportID;

  if (request.getParam("reportid", reportID) && reportID.size()) {
    bz_clearReport((unsigned int)atoi(reportID.c_str()));
  }
  return false;
}

bool AddReport::process(std::string& inputPage, const HTTPRequest& request, HTTPReply& reply) {
  if (!userInfo->hasPerm("talk")) {
    serverError->errorMessage = "RemoveReport: Invalid Permission";
    return false;
  }

  std::string reportBody;

  if (request.getParam("reportbody", reportBody) && reportBody.size()) {
    bz_fileReport(reportBody.c_str(), userInfo->userName.c_str());
  }
  return false;
}
// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
