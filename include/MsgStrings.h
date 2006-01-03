/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __MSG_STRINGS_H__
#define __MSG_STRINGS_H__


// common headers
#include "common.h"

// system headers
#include <string>
#include <vector>
#include <map>

typedef struct {
  int level;
  std::string text;
  std::string color;
} MsgString;

typedef std::vector<MsgString> MsgStringList;

namespace MsgStrings {

  // returns the name of the code  (doesn't need init())
  const char *strMsgCode(uint16_t code);

  void init();
  void reset();			// clean up all tracked state
  void useDNS(bool);		// look up hostnames based on IPs?
  void showEmail(bool);		// show player emails next to names?
  void colorize(bool);		// use ANSI color codes?
  void trackState(bool);	// track game state?

  int knownPacketTypes();

  // Messages from the server to the client
  MsgStringList msgFromServer(uint16_t len, uint16_t code, const void *data);

  // Messages from the client to the server  (currently unimplemented)
  MsgStringList msgFromClient(uint16_t len, uint16_t code, const void *data);
}

#endif // __MSG_STRINGS_H__


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
