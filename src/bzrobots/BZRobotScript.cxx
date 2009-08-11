/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
#include "BZRobotScript.h"

/* common implementation headers */
#include "bzsignal.h"
#include "TimeKeeper.h"

/* local implementation headers */
#include "BZRobotControl.h"
#include "ScriptLoaderFactory.h"

/* static entry point for threading */
static void *startBot(void *bot) {
  ((BZRobot *)(bot))->run();
  return NULL;
}

BZRobotScript::BZRobotScript()
{
  robot = NULL;
  botplayer = NULL;
  bzrobotcb = NULL;
  _loaded = false;
  _running = false;
  error = "Invalid script filename.";
}

BZRobotScript *BZRobotScript::loadFile(std::string filename)
{
  std::string::size_type extension_pos = filename.find_last_of(".");
  if (extension_pos == std::string::npos || extension_pos >= filename.length()) {
    return new BZRobotScript();
  }
  std::string extension = filename.substr(extension_pos + 1);
  
  if (!SCRIPTTOOLFACTORY.IsRegistered(extension)) {
    return new BZRobotScript();
  }

  BZRobotScript *scriptTool = SCRIPTTOOLFACTORY.scriptTool(extension);
  scriptTool->load(filename);

  return scriptTool;
}

void BZRobotScript::setPlayer(BZRobotPlayer *_botplayer)
{
  botplayer = _botplayer;
  bzrobotcb = BZRobotControl::CallbackSet(_botplayer);
}

bool BZRobotScript::hasPlayer()
{
  if(botplayer == NULL)
    return false;
  return true;
}

void BZRobotScript::start()
{
  if(!robot)
    robot = create();
  if(robot)
	robot->setCallbacks(bzrobotcb);
  if(botplayer)
    botplayer->setRobot(robot);

#ifndef _WIN32
  pthread_create( &rthread, NULL, startBot, robot);
#else
  rthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) startBot, robot, 0, 0);
#endif // _WIN32

  _running = true;
}

void BZRobotScript::stop()
{
#ifndef _WIN32
  pthread_join(rthread, NULL);
  pthread_detach(rthread);
#else
  WaitForSingleObject(rthread, INFINITE);
  CloseHandle(rthread);
#endif // _WIN32

  _running = false;

  if(robot)
	robot->setCallbacks(NULL);
  if(botplayer)
    botplayer->setRobot(NULL);
  if(robot)
    destroy(robot);
  robot = NULL;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
