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


// bzflag common header
#include "TextUtils.h"



ircControl::ircControl() {

  //mmm, defaults.

  configured = false;
  connected = false;

  server = "irc.freenode.net" //subject to change if we use our own IRCd 
  port = 6667;

  //TODO: pick a default control channel and set it up here

}


bool ircControl::loadConfigFile(std::string filename) {

  std::string errmsg;

  //open conf file

  std::ifstream ircconf;
  ircconf.open(filename.c_str());

  if (!ircconf.is_open()) {

    errmsg = "Failed to load IRC configuration file \"";
    errmsg += filename;
    errmsg += "\"\n; IRC will not be used."; 
    logDebugMessage(1, errmsg.c_str());

    return false;

  }

  //read the conf file
  std::string curline;
  std::vector<std::string> curlineargs;

  int lineNum = 1;

  while (getline(ircconf, curline)) {

    //snip the comment out of the line if it exists
    std::string::size_type index = curline.find('#');
    if (index != std::string::npos) {

      curline = curline.substr(0, index);

    }

    //split the line into arguments
    curlineargs = TextUtils::tokenize(curline, std::string("-"), 0, true);

    //there should be nothing other than whitespace before the first hyphen... check.
    std::string trimmedGarbage = TextUtils::no_whitespace(curlineargs->at(0));

    if (trimmedGarbage != "") {
      errmsg = "Unrecognized garbage \"" + trimmedGarbage + "\" in IRC config at line " + lineNum + ", ignoring";
      logDebugMessage(2, errmsg.c_str());
    }

    for (unsigned int x = 1; x < curlineargs->size(); x++) {

kl.trim(curlineargs->at(x));

      //not worth bothering if it's whitespace, obviously
      if (curlineargs->at(x) != "") { 

	std::vector<std::string> params = TextUtils::tokenize(curlineargs->at(x), std::string(" "), 0, true);

	//erase blank entries
	for (std::vector<std::string>::iterator x = params.begin(); x != params.end(); x++) 
	  if (*x == "") 
	    params.erase(x);

	//important processing stuff goes here
	//TODO: add IRC configuration stuffs.

      }
    }

      lineNum++;
  }

  //TODO: do some checks- make sure there's a nick, at least.

}

bool ircControl::init(){

  //register events

  //connect

  //send init commands and wait

  //join channels

}

bool ircControl::terminate(){

//issue a /quit and destroy everything

}

bool ircControl::update(){

  if (!client->process()) {
    //there seems to be a connection problem- do something
  }

}

bool ircControl::process (IRCClient &ircClient, teIRCEventType eventType, trBaseEventInfo &info) {

  //meat and potatoes of the implementation goes here

}


