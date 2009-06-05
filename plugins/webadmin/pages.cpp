/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// webadmin.cpp : Defines the entry point for the DLL application.
//

#include "pages.h"
#include <fstream>
#include <cstring>
#include <algorithm>

void Mainpage::process ( const std::string &pagename, const HTTPRequest &request, HTTPReply &reply )
{
//   std::string s1, s2, error;
//   if (request.request != ePost)
//     return;
//
//   std::vector<std::string> v;
//
//   // kick/ban players
//   if (request.getParam("players", v))
//   {
//     bool notify = request.getParam("notify", s1);
//
//     request.getParam("reason", s2);
//     std::vector<std::string>::iterator i;
//
//     if (request.getParam("kick", s1))
//     {
//       for (i = v.begin(); i != v.end(); i++)
// 	bz_kickUser(atoi(i->c_str()), s2.c_str(), notify);
//     }
//     else if (request.getParam("ipban", v))
//     {
//       request.getParam("duration", s1);
//       int duration = atoi(s1.c_str());
//       for (i = v.begin(); i != v.end(); i++)
//       {
// 	int playerID = atoi(i->c_str());
// 	bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerID);
// 	const char *playerIP = bz_getPlayerIPAddress(playerID);
// 	bz_IPBanUser(playerID, playerIP, duration, s2.c_str());
//       }
//     }
//     else if (request.getParam("idban", v))
//     {
//       request.getParam("duration", s1);
//       int duration = atoi(s1.c_str());
//       for (i = v.begin(); i != v.end(); i++)
//       {
// 	int playerID = atoi(i->c_str());
// 	bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerID);
// 	const char *callsign = bz_getPlayerCallsign(playerID);
// 	bz_IPBanUser(playerID, callsign, duration, s2.c_str());
//       }
//     }
//   }
//   // update server vars
//   bz_APIStringList * stringList = bz_newStringList();
//   int listSize = bz_getBZDBVarList(stringList);
//   for (loopPos = 0; loopPos < listSize; loopPos++)
//   {
//     s1 = "var";
//     s1 += (*stringList)[loopPos].c_str();
//     if (request.getParam(s1, s2))
//       bz_setBZDBString((*stringList)[loopPos].c_str(), s2.c_str());
//   }
//
//   bz_deleteStringList(stringList);
//
//   if (!error.empty())
//     templateVars["error"] = error;
}

void Banlistpage::process ( const std::string &pagename, const HTTPRequest &request, HTTPReply &reply )
{
//   if (request.request != ePost)
//     return;
//   std::vector<std::string> banRemovals;
//   std::vector<std::string>::iterator i;
//   if (request.getParam("delip", banRemovals))
//     for(i = banRemovals.begin(); i != banRemovals.end(); i++)
//       bz_IPUnbanUser(i->c_str());
//   if (request.getParam("delid", banRemovals))
//     for(i = banRemovals.begin(); i != banRemovals.end(); i++)
//       bz_IDUnbanUser(i->c_str());
}

void HelpMsgpage::process ( const std::string &pagename, const HTTPRequest &request, HTTPReply &reply )
{

}


void GroupPage::process ( const std::string &pagename, const HTTPRequest &request, HTTPReply &reply )
{
//   std::string name, error;
//   if (request.getParam("name", name)) {
//     stringList = bz_getGroupPerms(name.c_str());
//     if (!stringList) {
//       error += "No such group: ";
//       error += name + ". ";
//     } else if (request.request == eGet) {
//       templateVars["groupname"] = name;
//       editing = true;
//       return;
//     } else if (request.request == ePost) {
//       bz_deleteStringList(stringList);
//       listSize = bzu_standardPerms().size();
//       std::string perm;
//       std::vector<std::string> perms;
//       request.getParam("perm", perms);
//       for (loopPos = 0; loopPos < listSize; loopPos++) {
// 	perm = bzu_standardPerms()[loopPos];
// 	if (find(perms.begin(), perms.end(), perm) != perms.end()) {
// 	  if (!bz_groupAllowPerm(name.c_str(), perm.c_str())) {
// 	    error += "Couldn't change permissions for group: ";
// 	    error += name + ". ";
// 	  }
// 	} // TODO: else remove permission
//       }
//     }
//   }
//   if (!error.empty())
//     templateVars["error"] = error;
}




// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
