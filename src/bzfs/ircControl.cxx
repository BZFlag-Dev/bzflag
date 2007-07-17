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

// bzflag common header




ircControl::ircControl() {

  configured = false;
  connected = false;

}


bool ircControl::loadConfigFile(std::string filename){

  //load and parse a config file

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


