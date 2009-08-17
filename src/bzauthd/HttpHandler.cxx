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

struct HttpMemberInfoCallback : public GroupMemberCallback
{
  bool first_group;
  mg_connection *conn;
  HttpMemberInfoCallback(mg_connection *_conn, MemberFilter &filter)
  {
    conn = _conn;
    first_group = true;
    sUserStore.getMembershipInfo(filter, *this);
  }

  void got_group(char *uid, char* ou, char* grp)
  {
    mg_printf(conn, "%s%s %s %s", (first_group ? "" : ":"), uid, ou, grp);
    first_group = false;
  }

  void got_perm(uint32_t perm_val, char* val, char* arg)
  {
    mg_printf(conn, ",%s%s%s", val, (arg ? " " : ""), (arg ? arg : ""));
  }
};

struct HttpGroupInfoCallback : public GroupInfoCallback
{
  mg_connection *conn;
  bool first_group;
  HttpGroupInfoCallback(mg_connection *_conn, GroupFilter &filter)
  {
    conn = _conn;
    first_group = true;
    sUserStore.getGroupInfo(filter, *this);
  }

  void got_group(char* ou, char* grp, uint32_t state)
  {
    mg_printf(conn, "%s%s %s %d", (first_group ? "" : ":"), ou, grp, (int)state);
    first_group = false;
  }

  void got_perm(char* val, char* arg)
  {
    mg_printf(conn, ",%s%s%s", val, (arg ? " " : ""), (arg ? arg : ""));
  }
};

struct HttpOrgGroupsCallback : public FindGroupsCallback
{
  mg_connection *conn;
  bool first_group;
  HttpOrgGroupsCallback(mg_connection *_conn, GroupFilter &filter)
  {
    conn = _conn;
    first_group = true;
    sUserStore.getGroups(filter, *this);
  }

  void got_group(char* ou, char* grp)
  {
    mg_printf(conn, "%s%s %s", (first_group ? "" : ","), ou, grp);
    first_group = false;
  }
};

struct HttpOrgsOwnedByCallback : public OrgCallback
{
  mg_connection *conn;
  bool first_org;
  HttpOrgsOwnedByCallback(mg_connection *_conn, OrgFilter &filter) 
  {
    conn = _conn;
    first_org = true;
    sUserStore.getOrgs(filter, *this);
  }

  void got_org(char *ou)
  {
    mg_printf(conn, "%s%s", (first_org ? "" : ","), ou);
    first_org = false;
  }
};

struct HttpMemberCountCallback : public MemberCountCallback
{
  mg_connection *conn;
  bool first_group;
  HttpMemberCountCallback(mg_connection *_conn, MemberFilter &filter)
  {
    conn = _conn;
    first_group = true;
    sUserStore.getMemberCount(filter, *this);
  }

  void got_count(const char *ou, const char *grp, uint32_t count)
  {
    mg_printf(conn, "%s%s %s %d", (first_group ? "" : ","), ou, grp, (int)count);
    first_group = false;
  }
};

struct HttpUserNameCallback : public UserNameCallback
{
  mg_connection *conn;
  bool first_name;
  HttpUserNameCallback(mg_connection *_conn, UserNameFilter &filter)
  {
    conn = _conn;
    first_name = true;
    sUserStore.getUserNames(filter, *this);
  }

  void got_userName(const char *uid, const char *name)
  {
    mg_printf(conn, "%s%s %s", (first_name ? "" : ","), uid, name);
    first_name = false;
  }
};


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
    if(tokens[2].size() != 1) return;
    std::list<GroupId> list;
    bool all = false;
    if(tokens[2][0] == '0') {
      if(tokens.size() < 4) return;
      for(int i = 3; i < (int)tokens.size(); i++)
        list.push_back(GroupId(tokens[i]));
    } else
      all = true;
    
    list = sUserStore.intersectGroupList(tokens[1], list, all);
    for(std::list<GroupId>::iterator itr = list.begin(); itr != list.end(); ++itr)
      mg_printf(conn, ":%s.%s", itr->ou.c_str(), itr->grp.c_str());

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
    mg_printf(conn, "%d", sUserStore.addToGroup(tokens[1], GroupId(tokens[2])));

  } else if(tokens[0] == "activate") {
    if(tokens.size() < 4) return;
    mg_printf(conn, "%d", sUserStore.activateUser(UserInfo(tokens[1], "", tokens[2]), tokens[3]));

  } else if(tokens[0] == "resetpass") {
    if(tokens.size() < 3) return;
    mg_printf(conn, "%d", sUserStore.resetPassword(UserInfo(tokens[1], "", tokens[2])));

  } else if(tokens[0] == "resendactmail") {
    if(tokens.size() < 3) return;
    mg_printf(conn, "%d", sUserStore.resendActivation(UserInfo(tokens[1], "", tokens[2])));

  } else if(tokens[0] == "createorg") {
    if(tokens.size() < 2) return;

    mg_printf(conn, "%d", sUserStore.createOrganization(tokens[1]));

  } else if(tokens[0] == "creategroup") {
    if(tokens.size() < 2) return;

  } else if(tokens[0] == "getmemberinfo") {
    if(tokens.size() < 2) return;

    MemberFilter filter; filter.add_uid(tokens[1].c_str());
    HttpMemberInfoCallback cb(conn, filter);

  } else if(tokens[0] == "getgroupinfo") {
    if(tokens.size() < 3 || tokens.size() % 2 != 1) return;
    GroupFilter filter;
    for(int i = 1; i < (int)tokens.size(); i+=2)
      filter.add_org_group(tokens[i].c_str(), tokens[i+1].c_str());

    HttpGroupInfoCallback cb(conn, filter);

  } else if(tokens[0] == "getorggroups") {
    if(tokens.size() < 2) return;
    GroupFilter filter;
    for(int i = 1; i < (int)tokens.size(); i++)
      filter.add_org(tokens[i].c_str());

    HttpOrgGroupsCallback cb(conn, filter);

  } else if(tokens[0] == "getorgsownedby") {
    if(tokens.size() < 2) return;
    OrgFilter filter; filter.add_owner(tokens[1].c_str());

    HttpOrgsOwnedByCallback cb(conn, filter); 
    
  } else if(tokens[0] == "totalgroups") {
    mg_printf(conn, "%d", sUserStore.getTotalGroups());

  } else if(tokens[0] == "totalorgs") {
    mg_printf(conn, "%d", sUserStore.getTotalOrgs());

  } else if(tokens[0] == "getmembercount") {
    if(tokens.size() < 3 || tokens.size() % 2 != 1) return;
    MemberFilter filter;
    for(int i = 1; i < (int)tokens.size(); i+=2)
      filter.add_org_group(tokens[i].c_str(), tokens[i+1].c_str());

    HttpMemberCountCallback cb(conn, filter);

  } else if(tokens[0] == "getgroupmembers") {
    if(tokens.size() < 3) return;
    MemberFilter filter;
    for(int i = 1; i < (int)tokens.size(); i+=2)
      filter.add_org_group(tokens[i].c_str(), tokens[i+1].c_str());

    HttpMemberInfoCallback cb(conn, filter);

  } else if(tokens[0] == "getusernames") {
    if(tokens.size() < 2) return;
    UserNameFilter filter;
    for(int i = 1; i < (int)tokens.size(); i+=2)
      filter.add_uid(tokens[i].c_str());

    HttpUserNameCallback cb(conn, filter);

  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
