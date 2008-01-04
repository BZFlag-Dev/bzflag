/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _MSC_VER
#pragma warning( 4: 4786)
#endif

#include "common.h"

/* system headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <map>
#include <string>

/* common headers */
#include "TimeKeeper.h"

/* local headers */
#include "BZAdminClient.h"
#include "BZAdminUI.h"
#include "OptionParser.h"
#include "UIMap.h"

// causes persistent rebuilding to obtain build versioning
#include "version.h"


#ifdef _WIN32
void Player::setDeadReckoning( float timestamp )
{
}
#endif
/** @file
    This is the main file for bzadmin, the bzflag text client.
*/


int main(int argc, char** argv) {

#ifdef _WIN32
  // startup winsock
  {
    static const int major = 2, minor = 2;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(major, minor), &wsaData)) {
      std::cerr << "Could not initialise WinSock.";
      return 1;
    }
    if (LOBYTE(wsaData.wVersion) != major ||
	HIBYTE(wsaData.wVersion) != minor) {
      std::cerr << "Invalid WinSock version (got " << (int) LOBYTE(wsaData.wVersion) <<
	'.' << (int) HIBYTE(wsaData.wVersion) << ", expected" << major << '.' << minor << ')';
      WSACleanup();
      return 1;
    }
  }
#endif
  // command line options
  std::string uiName("curses");
  std::vector<std::string> visibleMsgs;
  std::vector<std::string> invisibleMsgs;

  // no curses, use stdboth as default instead
  const UIMap& interfaces = UIMap::instance();
  if (interfaces.find("curses") == interfaces.end())
    uiName = "stdboth";

  // build a usage string with all interfaces
  UIMap::const_iterator uiIter;
  std::string uiUsage;
  for (uiIter = interfaces.begin(); uiIter != interfaces.end(); ++uiIter)
    uiUsage += uiIter->first + '|';
  uiUsage = std::string("[-ui ") + uiUsage.substr(0, uiUsage.size() - 1) + ']';

  // register and parse command line arguments
  OptionParser op(std::string("bzadmin ") + getAppVersion(),
		  "CALLSIGN[:password]@HOST[:PORT] [COMMAND] [COMMAND] ...");
  const std::string uiOption("ui");
  const std::string uiMsg = "choose a user interface";
  op.registerVariable(uiOption, uiName, uiUsage, uiMsg);
  op.registerVariable("list", startupInfo.listServerURL, "[-list <list-server-url>]", "specify a list server to use");
  op.registerVector("show", visibleMsgs, "[-show msgtype{,msgtype}*]",
		      "tell bzadmin to show these message types");
  op.registerVector("hide", invisibleMsgs, "[-hide msgtype{,msgtype}*]",
		      "tell bzadmin not to show these message types");
  if (!op.parse(argc, argv))
    return 1;

  // check that the ui is valid
  uiIter = UIMap::instance().find(uiName);
  if (uiIter == UIMap::instance().end()) {
    std::cerr<<"There is no interface called \""<<uiName<<"\"."<<std::endl;
    return 1;
  }

  // check that we have callsign and host in the right format and extract them
  {
    int atPos;
    std::string callsign = "", password = "", serverName = "";
    if (!(op.getParameters().size() > 0 &&
	  (atPos = op.getParameters()[0].find('@')) > 0)) {
      // input callsign and host interactively
      std::cout << "No callsign@host specified.  Please input them" << std::endl;
      std::cout << "Callsign: ";
      std::getline(std::cin, callsign);
      if (callsign.size() <= 1) {
	std::cerr << "You must specify a callsign.  Exiting." << std::endl;
	return 1;
      }
      std::cout << "Password (optional): ";
      std::getline(std::cin, password);
      if (password.size() <= 1) {
	std::cerr << "Not using central login" << std::endl;
      }
      std::cout << "Server[:port] to connect to: ";
      std::getline(std::cin, serverName);
      if (serverName.size() <= 1) {
	std::cerr << "You must specify a host name to connect to.  Exiting." << std::endl;
	return 1;
      }
    } else { // callsign:password@host:port on command line
      callsign = op.getParameters()[0].substr(0, atPos);
      int pPos = callsign.find(':');
      if (pPos != -1) {
	password = callsign.substr(pPos + 1).c_str();
	callsign = callsign.substr(0, pPos);
      }
      serverName = op.getParameters()[0].substr(atPos + 1);
    }
    startupInfo.serverPort = ServerPort;
    int cPos = serverName.find(':');
    if (cPos != -1) {
      startupInfo.serverPort = atoi(serverName.substr(cPos + 1).c_str());
      serverName = serverName.substr(0, cPos);
    }
    strncpy(startupInfo.callsign, callsign.c_str(), sizeof(startupInfo.callsign) - 1);
    strncpy(startupInfo.password, password.c_str(), sizeof(startupInfo.password) - 1);
    strncpy(startupInfo.serverName, serverName.c_str(), sizeof(startupInfo.serverName) - 1);
  }
  std::cerr << "Connecting to " <<
    startupInfo.callsign << "@" <<
    startupInfo.serverName << ":" <<
    startupInfo.serverPort;
  if (strlen(startupInfo.password)) {
    std::cerr << " using central login";
  }
  std::cerr << std::endl;

  // try to connect
  BZAdminClient client;
  if (!client.isValid())
    return 1;

  unsigned int i;
  for (i = 0; i < visibleMsgs.size(); ++i)
    client.showMessageType(visibleMsgs[i]);
  for (i = 0; i < invisibleMsgs.size(); ++i)
    client.ignoreMessageType(invisibleMsgs[i]);

  // if we got commands as arguments, send them
  if (op.getParameters().size() > 1) {
    // if we have a token wait a bit for global login
    // FIXME: should "know" when we are logged in (or fail) and only wait that long.
    if (startupInfo.token[0] != 0)
      TimeKeeper::sleep(5.0);
    for (unsigned int j = 1; j < op.getParameters().size(); ++j) {
      const std::string& cmd = op.getParameters()[j];
      if (cmd == "/quit") {
	client.waitForServer();
	return 0;
      }
      else if (strncasecmp(cmd.c_str(), "/sleep", 6) == 0) {
	const char* start = cmd.c_str() + 6;
	char* endptr;
	double sleepTime = strtod(start, &endptr);
	if (endptr != start) {
	  TimeKeeper::sleep(sleepTime);
	}
      }
      else {
	client.sendMessage(cmd, AllPlayers);
      }
    }
  }

  /* try to set the processor affinity to prevent TimeKeeper from
   * reporting negative times
   */
  TimeKeeper::setProcessorAffinity();

  // create UI and run the main loop
  BZAdminUI* ui = uiIter->second(client);
  client.setUI(ui);
  client.runLoop();
  delete ui;

  return 0;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
