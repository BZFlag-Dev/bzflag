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

#ifndef	__FRONTEND_H__
#define	__FRONTEND_H__

#include "common.h"

/* system interface headers */
#include <string>

/* local interface headers */
#include "RCLinkFrontend.h"
#include "ScriptLoader.h"
#include "BZAdvancedRobot.h"


/**
 * Remote Control Frontend: Class to encapsulate the frontend.
 */
class Frontend
{
  RCLinkFrontend *link;
  bool sentStuff;
  Frontend();
  std::string error;
  ScriptLoader *scriptLoader;
  BZAdvancedRobot *robot;
  
public:
  static bool run(std::string filename, const char *host, int port); 
  
  bool connect(const char *host, int port);
  void start(std::string filename);
  const std::string &getError() const { return error; }
};

#else
class Frontend;
#endif /* __FRONTEND_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
