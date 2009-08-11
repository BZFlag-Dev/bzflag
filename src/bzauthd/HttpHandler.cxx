/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "HttpHandler.h"
#include "ConfigMgr.h"
#include "Log.h"
#include "UserStorage.h"
#include <mongoose.h>
#include <network.h>
#include <curl/curl.h>
#include "TokenMgr.h"

INSTANTIATE_SINGLETON(HttpHandler)

HttpHandler::HttpHandler()
{
  ctx = NULL;
}

HttpHandler::~HttpHandler()
{
  if(ctx) mg_stop(ctx);
}

bool HttpHandler::initialize()
{
  if(!(ctx = mg_start())) return false;
  if(1 != mg_set_option(ctx, "ports", (const char*)sConfig.getStringValue(CONFIG_HTTP_PORT))) return false;

  mg_set_uri_callback(ctx, "/", &request_callback, (void *)NULL);
  sLog.outLog("HttpHandler: initialized");
  return true;
}

std::vector<std::string> split_request(const std::string &request)
{
  std::vector<std::string> ret;
  std::string::size_type off = 0, del;
  while(1) {
    del = request.find('&', off);
    std::string elem = del == request.npos ? request.substr(off) : request.substr(off, del - off);

    char *unescaped = curl_easy_unescape(NULL, elem.c_str(), (int)elem.length(), NULL);
    ret.push_back(unescaped);
    curl_free(unescaped);

    if(del == request.npos) break;
    off = del + 1;
  };
  return ret;
}

void HttpHandler::request_callback(
  struct mg_connection *conn, const struct mg_request_info *request_info, void * /*user_data*/)
{
  // only from localhost (127.0.0.1)
  if(request_info->remote_ip != 0x7F000001) {
    sLog.outError("request dropped from ip %d", request_info->remote_ip);
    return;
  }

  char *req = request_info->query_string;
  if(!req) return;
  sLog.outDebug("got request %s", req);

  // apparently it isn't received at all if it's too short so
  mg_printf(conn, "message:");

  std::vector<std::string> tokens = split_request(req);
  if(tokens.empty())
    return;

  if(tokens[0] == "register") {
    if(tokens.size() < 4) return;
    mg_printf(conn, "%d", (int)sUserStore.registerUser(UserInfo(tokens[1], tokens[2], tokens[3])));

  } else if(tokens[0] == "intersectGroups") {
    if(tokens.size() < 3) return;
    if(tokens[2].size() != 2) return;
    std::list<std::string> list;
    bool all = false;
    if(tokens[2][0] == '0') {
      if(tokens.size() < 4) return;
      for(int i = 3; i < (int)tokens.size(); i++)
        list.push_back(tokens[i]);
    } else
      all = true;
    
    list = sUserStore.intersectGroupList(tokens[1], list, all, tokens[2][1] == '1');
    for(std::list<std::string>::iterator itr = list.begin(); itr != list.end(); ++itr)
      mg_printf(conn, ":%s", itr->c_str());

  } else if(tokens[0] == "gettoken") {
    if(tokens.size() < 4) return;
    // TODO: token[3] = ip
    uint32_t uid = sUserStore.authUser(UserInfo(tokens[1], tokens[2], ""));
    if(uid) {
      uint32_t token = sTokenMgr.newToken(tokens[1], uid, 0);
      mg_printf(conn, "%d", (int)token);
    }

  } else if(tokens[0] == "checktoken") {
    if(tokens.size() < 4) return;
    // TODO: tokens[2] = ip
    if(sUserStore.isRegistered(tokens[1])) {    
      uint32_t token;
      sscanf(tokens[3].c_str(), "%d", (int*)&token);
      uint32_t bzid = sTokenMgr.checkToken(token, tokens[1], 0);
      if(bzid)
        mg_printf(conn, "3,%d", bzid);
      else
        mg_printf(conn, "2,0");
    } else
      mg_printf(conn, "1,0");

  } else if(tokens[0] == "chinf") {
    if(tokens.size() < 5) return;
    mg_printf(conn, "%d", (int)sUserStore.changeUserInfo(tokens[1], UserInfo(tokens[2], tokens[3], tokens[4])) );

  } else if(tokens[0] == "addtogroup") {
    if(tokens.size() < 3) return;
    mg_printf(conn, "%d", sUserStore.addToGroup(tokens[1], tokens[2], 
      sUserStore.getUserDN(tokens[1]), sUserStore.getGroupDN(tokens[2]) ) );

  } else if(tokens[0] == "activate") {
    if(tokens.size() < 4) return;
    mg_printf(conn, "%d", sUserStore.activateUser(UserInfo(tokens[1], "", tokens[2]), tokens[3]));

  } else if(tokens[0] == "resetpass") {
    if(tokens.size() < 3) return;
    mg_printf(conn, "%d", sUserStore.resetPassword(UserInfo(tokens[1], "", tokens[2])));

  } else if(tokens[0] == "resendactmail") {
    if(tokens.size() < 3) return;
    mg_printf(conn, "%d", sUserStore.resendActivation(UserInfo(tokens[1], "", tokens[2])));

  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
