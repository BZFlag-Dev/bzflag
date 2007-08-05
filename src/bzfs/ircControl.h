/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef __IRCRELAY_H__
#define __IRCRELAY_H__

//global header
#include "common.h"


#include <map>
#include <string>

//libIRC
#include "libIRC.h"

//CAUTION: IN PROGRESS!

struct ircChannel {

  ircChannel();

  std::string name;

  bool joined;

  bool relayGameChat; 		//relay in-game chatter to this channel
  bool relayGameJoinsAndParts; 	//relay player joins and parts to this channel
  bool relayIRCChat; 		//relay IRC chat in this channel to the server's chat channel
  bool relayIRCJoinsAndParts; 	//relay IRC user joins and parts in the server's chat channel
  bool acceptCommands; 		//accept administrative commands from IRC users over this channel. (TODO: Implement authentication so this can be used)

};


class ircControl : public IRCBasicEventCallback {
 public:
  ircControl();

  bool loadConfigFile(std::string filename); //read a configuration file

  bool init(); //connect, join, etc

  bool terminate(bool forShutdown = true); //disconnect for server shutdown

  bool update(); //check for messages, etc

  virtual bool process (IRCClient &ircClient, teIRCEventType eventType, trBaseEventInfo &info);

  ~ircControl();

 private:

  IRCClient client;

  std::string server;
  int port;

  std::string nick; //TODO: Er, multiple nicks would probably be a good idea. Whoops...
  std::string pwd;

  bool configured;
  bool connected;

  ircChannel controlChannel; //mandatory bot control channel for the centralized system

  std::map<std::string, ircChannel> channels; //channel data

  std::vector<std::string> initCommands; //commands to execute upon connecting, before joining any channels

  



};

#else

class ircControl;

#endif


/* Local Variables: ***
 * mode: C++ ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */