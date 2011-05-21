/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
#include "RobotScript.h"

/* common implementation headers */
#include "bzsignal.h"
#include "BzTime.h"

/* local implementation headers */
#include "RobotControl.h"
#include "ScriptLoaderFactory.h"

/* static entry point for threading */
static void* startBot(void* bot) {
  ((BZRobots::Robot*)bot)->run();
  return NULL;
}

RobotScript::RobotScript() {
  robot = NULL;
  botplayer = NULL;
  bzrobotcb = NULL;
  _loaded = false;
  _running = false;
  error = "Invalid script filename.";
}

RobotScript* RobotScript::loadFile(std::string filename) {
  std::string::size_type extension_pos = filename.find_last_of(".");
  if (extension_pos == std::string::npos || extension_pos >= filename.length()) {
    return new RobotScript();
  }
  std::string extension = filename.substr(extension_pos + 1);

  if (!SCRIPTTOOLFACTORY.IsRegistered(extension)) {
    return new RobotScript();
  }

  RobotScript* scriptTool = SCRIPTTOOLFACTORY.scriptTool(extension);
  scriptTool->load(filename);

  return scriptTool;
}

void RobotScript::setPlayer(BZRobotPlayer* _botplayer) {
  botplayer = _botplayer;
  botplayer->setRobot(robot);
  bzrobotcb = RobotControl::CallbackSet(_botplayer);
  if (robot) {
    robot->setCallbacks(bzrobotcb);
  }
}

bool RobotScript::hasPlayer() {
  return (botplayer != NULL);
}

void RobotScript::start() {
  if (!robot) {
    robot = create();
  }
  if (robot) {
    robot->setCallbacks(bzrobotcb);
  }
  if (botplayer) {
    botplayer->setRobot(robot);
  }

#ifndef _WIN32
  pthread_create(&rthread, NULL, startBot, robot);
#else
  rthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) startBot, robot, 0, 0);
#endif // _WIN32

  _running = true;
}

void RobotScript::stop() {
#ifndef _WIN32
  pthread_join(rthread, NULL);
  pthread_detach(rthread);
#else
  WaitForSingleObject(rthread, INFINITE);
  CloseHandle(rthread);
#endif // _WIN32

  _running = false;

  if (robot) {
    robot->setCallbacks(NULL);
  }
  if (botplayer) {
    botplayer->setRobot(NULL);
  }
  if (robot) {
    destroy(robot);
  }

  robot = NULL;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
