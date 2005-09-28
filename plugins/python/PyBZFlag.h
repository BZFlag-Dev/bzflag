/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "PyBZDB.h"
#include "PyEvent.h"
#include "PyEvents.h"
#include "PyTeam.h"
#include <Python.h>
#include <map>

#ifndef __PYTHON_BZFLAG_H__
#define __PYTHON_BZFLAG_H__

namespace Python
{

class BZFlag
{
public:
  BZFlag ();

  PyObject *GetListeners (int event);
  void AddPlayer (int id);
  void RemovePlayer (int id);

protected:
  void RegisterEvent (Handler *handler, bz_eEventType event);

private:
  Event *event_sub;
  Team  *team_sub;

  PyObject *event_listeners;
  PyObject *players;
  PyObject *bzdb;

  PyObject *module;

  CaptureHandler	capture_handler;
  DieHandler		die_handler;
  SpawnHandler		spawn_handler;
  ZoneEntryHandler	zone_entry_handler;
  ZoneExitHandler	zone_exit_handler;
  JoinHandler		join_handler;
  PartHandler		part_handler;
  ChatHandler		chat_handler;
  UnknownSlashHandler	unknownslash_handler;
  GetSpawnPosHandler	getspawnpos_handler;
  GetAutoTeamHandler	getautoteam_handler;
  AllowPlayerHandler	allowplayer_handler;
  TickHandler		tick_handler;
  GenerateWorldHandler	generateworld_handler;
  GetPlayerInfoHandler	getplayerinfo_handler;
};

};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

