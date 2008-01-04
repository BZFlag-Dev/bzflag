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

/* interface header */
#include "Frontend.h"

/* system implementation headers */
#ifdef _WIN32
#  include <io.h>
#else
#  include <unistd.h>
#endif

/* common implementation headers */
#include "bzsignal.h"
#include "TimeKeeper.h"

/* local implementation headers */
#include "RCMessageFactory.h"
#include "TestRobot.h"
#include "ScriptLoaderFactory.h"
#include "Logger.h"


bool Frontend::run(std::string filename, const char *host, int port)
{
#ifndef _USE_FAKE_NET
  pid_t pid = fork();
  if (pid < 0)
    return false;
  else if (pid > 0)
    return true;
#endif

  fclose(stdin); // Shouldn't mess around with that here ;-)
  bzSignal(SIGINT, SIG_DFL);

  Frontend frontend;
  if (!frontend.connect(host, port)) {
    FRONTENDLOGGER << "Frontend failed to connect! Bailing! (" << frontend.getError() << ")" << std::endl;
    return false;
  }

  FRONTENDLOGGER << "Frontend initialized, " << host << ":" << port << std::endl;
  frontend.start(filename);
  FRONTENDLOGGER << "Frontend disconnected / failed! (" << frontend.getError() << ")" << std::endl;
  return false;
}


Frontend::Frontend() :sentStuff(false)
{
  RCMessageFactory<RCReply>::initialize();
  RCMessageFactory<RCEvent>::initialize();
}


bool Frontend::connect(const char *host, int port)
{
  link = new RCLinkFrontend();
  RCREPLY.setLink(link);
  return link->connect(host, port);
}


void Frontend::start(std::string filename)
{
  std::string::size_type extension_pos = filename.find_last_of(".");
  if (extension_pos == std::string::npos || extension_pos >= filename.length()) {
    error = "Could not find a valid extension in filename '" + filename + "'.";
    return;
  }
  std::string extension = filename.substr(extension_pos + 1);

  if (!SCRIPTLOADER.IsRegistered(extension)) {
    error = "Could not find a proper ScriptLoader for extension '" + extension + "'.";
    return;
  }

  scriptLoader = SCRIPTLOADER.scriptLoader(extension);
  if (!scriptLoader->load(filename)) {
    error = "Could not load script: " + scriptLoader->getError();
    return;
  }

  robot = scriptLoader->create();
  if (robot == NULL) {
    error = "Could not instantiate robot via ScriptLoader for file '" + filename + "'.";
    return;
  }

  while (link->getStatus() != RCLink::Connected) {
    link->update();
    TimeKeeper::sleep(0.5);
  }

  FRONTENDLOGGER << "Loaded script " << filename << ", starting the bot! :-)" << std::endl;
  robot->setLink(link);
  robot->run();

  error = "Bot finished executing, shouldn't really happen.";
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
