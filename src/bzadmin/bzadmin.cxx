/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _MSC_VER
#pragma warning( 4: 4786)
#endif

#include <stdio.h>
#include <iostream>
#include <map>
#include <string>

#include "BZAdminClient.h"
#include "BZAdminUI.h"
#include "OptionParser.h"
#include "UIMap.h"

// causes persistent rebuilding to obtain build versioning
#include "version.h"

int debugLevel = 0;


#ifdef _WIN32
void Player::setDeadReckoning()
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
		  std::cerr << "Invalid WinSock version.";
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
		  "CALLSIGN@HOST[:PORT] [COMMAND] [COMMAND] ...");
  const std::string uiOption("ui");
  const std::string uiMsg = "choose a user interface";
  op.registerVariable(uiOption, uiName, uiUsage, uiMsg);
  op.registerVector("show", visibleMsgs, "[-show msgtype{,msgtype}*]",
		      "tell bzadmin to show these message types");
  op.registerVector("hide", invisibleMsgs, "[-hide msgtype{,msgtype}*]",
		      "tell bzadmin not to show these message types");
  if (!op.parse(argc, argv))
    return 1;
  
  // check that we have callsign and host in the right format and extract them
  int atPos;
  std::string name = "", host = "";
  if (!(op.getParameters().size() > 0 &&
	(atPos = op.getParameters()[0].find('@')) > 0)) {
    // input callsign and host interactively
    std::cout << "No callsign@host specified.  Please input them" << std::endl;
    std::cout << "Callsign: ";
    std::getline(std::cin, name);
    if (name.size() <= 1) {
      std::cerr << "You must specify a callsign.  Exiting." << std::endl;
      return 1;
    }
    std::cout << "Server to connect to: ";
    std::getline(std::cin, host);
    if (host.size() <= 1) {
      std::cerr << "You must specify a host name to connect to.  Exiting." << std::endl;
      return 1;
    }
  } else { // callsign/host on command line
    name = op.getParameters()[0].substr(0, atPos);
    host = op.getParameters()[0].substr(atPos + 1);
  }
  
  int port = ServerPort;
  int cPos = host.find(':');
  if (cPos != -1) {
    port = atoi(host.substr(cPos + 1).c_str());
    host = host.substr(0, cPos);
  }

  // check that the ui is valid
  uiIter = UIMap::instance().find(uiName);
  if (uiIter == UIMap::instance().end()) {
    std::cerr<<"There is no interface called \""<<uiName<<"\"."<<std::endl;
    return 1;
  }

  // try to connect
  BZAdminClient client(name, host, port);
  if (!client.isValid())
    return 1;
  unsigned int i;
  for (i = 0; i < visibleMsgs.size(); ++i)
    client.showMessageType(visibleMsgs[i]);
  for (i = 0; i < invisibleMsgs.size(); ++i)
    client.ignoreMessageType(invisibleMsgs[i]);

  // if we got commands as arguments, send them and exit
  if (op.getParameters().size() > 1) {
    for (unsigned int i = 1; i < op.getParameters().size(); ++i) {
      if (op.getParameters()[i] == "/quit") {
        client.waitForServer();
        return 0;
      }
      client.sendMessage(op.getParameters()[i], AllPlayers);
    }
  }

  // create UI and run the main loop
  BZAdminUI*  ui = uiIter->second(client);
  client.setUI(ui);
  client.runLoop();
  delete ui;

  return 0;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
