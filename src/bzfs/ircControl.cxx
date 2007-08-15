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

/* class interface header */
#include "ircControl.h"


#include <fstream>

#include "libIRC.h"


// bzflag common header
#include "TextUtils.h"

#include "bzfio.h"
#include "bzfs.h"

ircChannel::ircChannel(){

  joined;

  relayGameChat = false; 
  relayGameJoinsAndParts = false;
  relayIRCChat = false;
  relayIRCJoinsAndParts = false; 
  acceptCommands = false; 

}

ircControl::ircControl() {

  //mmm, defaults.

  configured = false;
  connected = false;

  server = "irc.freenode.net"; //subject to change if we use our own IRCd 
  port = 6667;

  //TODO: pick a default control channel and set it up here

}

//FIXME: Some of this text wrangling could probably be added to TextUtils... 
bool ircControl::loadConfigFile(std::string filename) {

  logDebugMessage(4, "starting IRC configuration\n");

  std::string errmsg;

  //open conf file

  std::ifstream ircconf;
  ircconf.open(filename.c_str());

  if (!ircconf.is_open()) {

    errmsg = "Failed to load IRC configuration file \"";
    errmsg += filename + "\"\n; IRC will not be used.";
    logDebugMessage(1, errmsg.c_str());

    return false;

  }

  //read the conf file
  std::string curline;
  std::vector<std::string> curlineargs;

  int lineNum = 1;

  while (getline(ircconf, curline)) {


    //snip the comment out of the line if it exists
    //FIXME: um, oops, DUH. IRC channels use #, so using it as a comment character is a BAD idea.
    //Commented out until I choose another character 
    /*
    std::string::size_type index = curline.find('#');
    if (index != std::string::npos) {

      curline = curline.substr(0, index);

    } */

    //split the line into arguments
    curlineargs = TextUtils::tokenize(curline, std::string("-"), 0, true);

    //there should be nothing other than whitespace before the first hyphen... check.
    std::string trimmedGarbage = TextUtils::no_whitespace(curlineargs.at(0));

    if (trimmedGarbage != "") {
      errmsg = "Unrecognized garbage \"";
      errmsg += trimmedGarbage;
      errmsg += "\" in IRC config at line ";
      errmsg += lineNum;
      errmsg += ", ignoring";
      logDebugMessage(2, errmsg.c_str());
    }

    for (unsigned int x = 1; x < curlineargs.size(); x++) {

      //trim whitespace from the ends
      //Find the last non-whitespace char...
      int last;
      for (last = curlineargs.at(x).length() - 1; last >= 0; last--)
	if (!TextUtils::isWhitespace(curlineargs.at(x)[last])) 
	  break;

      if (last >= 0) { //if last gets to -1, the string is all whitespace (or empty). No reason to continue processing.
	
	//...otherwise, time to do the front...
	int first;
	for (first = 0; first < curlineargs.at(x).length(); first++) 
	  if (!TextUtils::isWhitespace(curlineargs.at(x)[first]))
	    break;

	//trim the string...
	curlineargs.at(x) = curlineargs.at(x).substr(first, last - first + 1);

	//okay, split the arg into params
	std::vector<std::string> params = TextUtils::tokenize(curlineargs.at(x), std::string(" "), 0, true);

	//erase blank entries
	//for (std::vector<std::string>::iterator x = params.begin(); x != params.end(); x++) 
	//  if (*x == "") 
	//    params.erase(x);

	if (params.size() == 0) //sanity check- an empty param should have been caught already
	  continue;

	//Now we start processing the arguments (finally!) Ordered by likely frequency in the configuration.
	if (TextUtils::compare_nocase(params.at(0), std::string("channel")) == 0 && params.size() >= 2) {

	  if (channels.count(params.at(1)) > 0) //make sure we don't have this channel already
	    continue;

	  ircChannel newchan;

	  newchan.name = params.at(1);

	  //loop through the remaining params and configure this channel accordingly
	  for (int i = 2; i < params.size(); i++) {
	    if (TextUtils::compare_nocase(params.at(i), std::string("relayGameChat")) == 0) {
	      newchan.relayGameChat = true;

	    } else if (TextUtils::compare_nocase(params.at(i), std::string("relayGameJoinsAndParts")) == 0) {
	      newchan.relayGameJoinsAndParts = true;

	    } else if (TextUtils::compare_nocase(params.at(i), std::string("relayIRCChat")) == 0) {
	      newchan.relayIRCChat = true;

	    } else if (TextUtils::compare_nocase(params.at(i), std::string("relayIRCJoinsAndParts")) == 0) {
	      newchan.relayIRCJoinsAndParts = true;

	    } else if (TextUtils::compare_nocase(params.at(i), std::string("acceptCommands")) == 0) {
	      newchan.acceptCommands = true;

	    } else { 
	      errmsg = "Unrecognized parameter \"";
	      errmsg += params.at(i);
	      errmsg += "\" for channel \"";
	      errmsg += params.at(1);
	      errmsg += "\" on line ";
	      errmsg += lineNum;
	      errmsg += ", ignoring.\n";
	      logDebugMessage(2, errmsg.c_str());
	    }
	  }

	  //our channel is configured. Add it to the system.
	  channels[newchan.name] = newchan;

	} else if (TextUtils::compare_nocase(params.at(0), std::string("initCommand")) == 0 && params.size() == 2) {
	  initCommands.push_back(params.at(1));

	} else if (TextUtils::compare_nocase(params.at(0), std::string("nick")) == 0 && params.size() == 2) {
	  nick = params.at(1);

	} else if (TextUtils::compare_nocase(params.at(0), std::string("pwd")) == 0 && params.size() == 2) {
	  pwd = params.at(1);

	} else if (TextUtils::compare_nocase(params.at(0), std::string("controlChannel")) == 0 && params.size() == 2) {
	  controlChannel.name = params.at(1);

	} else if (TextUtils::compare_nocase(params.at(0), std::string("server")) == 0 && params.size() == 2) {
	  server = params.at(1);

	} else if (TextUtils::compare_nocase(params.at(0), std::string("port")) == 0 && params.size() == 2) {
	  port = atoi(params.at(1).c_str());

	} else {
	  errmsg = "Unrecognized argument \"";
	  errmsg += curlineargs.at(x);
	  errmsg += "\" on line ";
	  errmsg += lineNum;
	  errmsg += ", ignoring.\n";
	  logDebugMessage(2, errmsg.c_str());
	}
      }
    }

      lineNum++;
  }

  if (nick == "") {
    errmsg = "No nick specified. IRC will not be used.";
    logDebugMessage(1, errmsg.c_str());

    return false;
  }

  configured = true;
  return true; 
}

bool ircControl::init(){

  logDebugMessage(4, "starting IRC init\n");

  std::string errmsg;

  //make sure it's configured... sanity check
  if (!configured) {
    logDebugMessage (4, "IRC will not load (not configured)\n");
    return false; //we don't need to explain- the conf loader will do that when it fails
  }

  //register events
  bool allworked = true;
  allworked = allworked && client.registerEventHandler(eIRCNoticeEvent, this);
  allworked = allworked && client.registerEventHandler(eIRCNickNameError, this);
  allworked = allworked && client.registerEventHandler(eIRCNickNameChange, this);
  //client.registerEventHandler(eIRCWelcomeEvent, this);
  allworked = allworked && client.registerEventHandler(eIRCEndMOTDEvent, this);
  allworked = allworked && client.registerEventHandler(eIRCChannelJoinEvent, this);
  allworked = allworked && client.registerEventHandler(eIRCChannelPartEvent, this);
  allworked = allworked && client.registerEventHandler(eIRCChannelBanEvent, this);
  allworked = allworked && client.registerEventHandler(eIRCChannelMessageEvent, this);
  allworked = allworked && client.registerEventHandler(eIRCPrivateMessageEvent, this);
  //client.registerEventHandler(eIRCTopicEvent, this);
  allworked = allworked && client.registerEventHandler(eIRCUserJoinEvent, this);
  allworked = allworked && client.registerEventHandler(eIRCUserPartEvent, this);
  allworked = allworked && client.registerEventHandler(eIRCUserKickedEvent, this);
  //client.registerEventHandler(eIRCTopicChangeEvent, this);
  allworked = allworked && client.registerEventHandler(eIRCChanInfoCompleteEvent, this);
  allworked = allworked && client.registerEventHandler(eIRCChannelModeSet, this);
  allworked = allworked && client.registerEventHandler(eIRCChannelUserModeSet, this);
  allworked = allworked && client.registerEventHandler(eIRCUserModeSet, this);
  allworked = allworked && client.registerEventHandler(eIRCQuitEvent, this);

  if (!allworked)
    logDebugMessage (0, "Some of the events failed to register.");

  //connect
  if (!client.connect(server, port)) {
    errmsg = "IRC Connection failed.";
    logDebugMessage(1, errmsg.c_str());
    return false;
  }

  return true;

}

bool ircControl::terminate(bool forShutdown) {

  if (!connected)
    return false; //can't disconnect if we're not connected...

  //issue a /quit and destroy everything
  if (forShutdown)
    return client.disconnect("BZFS: Shutting down");
  else
    return client.disconnect("BZFS: Premature IRC disconnection!");
}

bool ircControl::update(){

  if (!client.process()) {
    //there seems to be a connection problem- do something
  }

}

bool ircControl::process (IRCClient &ircClient, teIRCEventType eventType, trBaseEventInfo &info) {

  std::string errmsg;

  //debug
  logDebugMessage(0, "IRC: Process called");

  //meat and potatoes of the implementation goes here

  /*
  if (eventType == eIRCNoticeEvent) {

  } else if (eventType == eIRCNickNameError) {

  } else if (eventType == eIRCNickNameChange) {

  } else if (eventType == eIRCWelcomeEvent) {

  } else */ if (eventType == eIRCEndMOTDEvent) {
    //this will finish off the connection stuff, like commands and channel joins.

    //send init commands and wait
    //TODO: Implement this... 

    //join channels
    for (std::map<std::string, ircChannel>::iterator itr = channels.begin(); itr != channels.end(); itr++) {
      if (client.join(itr->second.name)) {
        itr->second.joined = true;
      } else {
        errmsg = "Unable to join channel \"";
        errmsg += itr->second.name + "\"\n";
        logDebugMessage(1, errmsg.c_str());
      }
    }
  } /* else if (eventType == eIRCChannelJoinEvent) {

  } else if (eventType == eIRCChannelPartEvent) {

  } else if (eventType == eIRCChannelBanEvent) {

  } */ else if (eventType == eIRCChannelMessageEvent) {

    trMessageEventInfo* chanMsgInfo = (trMessageEventInfo*)&info;

    if (chanMsgInfo->source == controlChannel.name) {
      //we're dealing with the bot control channel. Act accordingly.
      //TODO: Check for a player query


      return true;
    }

    if (channels.count(chanMsgInfo->source) == 1) { //make sure we're actually supposed to be in this channel
      if (channels[chanMsgInfo->source].relayIRCChat) {
        //forward the chat msg to the game if we're supposed to.
        std::string msg = "[IRC]";
        msg += chanMsgInfo->from + ": " + chanMsgInfo->message;
        sendMessage(ServerPlayer, AllPlayers, msg.c_str());
      }
    }

  } else if (eventType == eIRCPrivateMessageEvent) {

    trMessageEventInfo* pmInfo = (trMessageEventInfo*)&info;

    //we are probably getting a command, or a return for a query. 

  } /* else if (eventType == eIRCTopicEvent) {

  } else if (eventType == eIRCUserJoinEvent) {

  } else if (eventType == eIRCUserPartEvent) {

  } else if (eventType == eIRCUserKickedEvent) {

  } else if (eventType == eIRCTopicChangeEvent) {

  } else if (eventType == eIRCChanInfoCompleteEvent) {

  } else if (eventType == eIRCChannelModeSet) {

  } else if (eventType == eIRCChannelUserModeSet) {

  } else if (eventType == eIRCUserModeSet) {

  } */ else if (eventType == eIRCQuitEvent) {

    logDebugMessage(4, "BORK! We've been disconnected!");

  }

  return true;

}

void ircControl::handleGameChat(std::string playername, std::string message) {

  for (std::map<std::string, ircChannel>::iterator itr = channels.begin(); itr != channels.end(); itr++) {
    if (itr->second.relayGameChat) {
      //send the chat msg to this channel
      std::string msg = playername;
      msg += ": ";
      msg += message;
      client.sendMessage(itr->second.name, msg);
    }
  }
}


