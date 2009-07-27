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

#include <common.h>
#include "HttpHandler.h"
#include "ConfigMgr.h"
#include "Log.h"
#include <TextUtils.h>
#include "UserStorage.h"

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
  std::string::size_type off = 0, poz, del;
  while(1) {
    del = request.find('&', off);
    std::string elem = del == request.npos ? request.substr(off) : request.substr(off, del - off);

    poz = elem.find("*2"); if(poz != elem.npos) elem.replace(poz, 2, "&");
    poz = elem.find("*1"); if(poz != elem.npos) elem.replace(poz, 2, "*");

    ret.push_back(elem);
    if(del == request.npos) break;
    off = del + 1;
  };
  return ret;
}

void HttpHandler::request_callback(
  struct mg_connection *conn, const struct mg_request_info *request_info, void *user_data)
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
    // TODO: not thread safe
    BzRegErrors err = sUserStore.registerUser(UserInfo(tokens[1], tokens[2], tokens[3]));

    mg_printf(conn, "%d", (int)err);
  } else if(tokens[0] == "intersectGroups") {
    if(tokens.size() < 3) return;
    std::list<std::string> list;
    for(int i = 2; i < tokens.size(); i++)
      list.push_back(tokens[i]);
    
    list = sUserStore.intersectGroupList(tokens[1], list);
    for(std::list<std::string>::iterator itr = list.begin(); itr != list.end(); ++itr)
      mg_printf(conn, ":%s", itr->c_str());
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
