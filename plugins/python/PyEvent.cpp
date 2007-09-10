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

#include "bzfsAPI.h"
#include "PyEvent.h"

namespace Python
{

Event::Event ()
{
  module = Py_InitModule3 ("BZFlag.Event", NULL, NULL);

  // This module just contains all those fiddly little event IDs
  PyModule_AddIntConstant (module, "Null",			bz_eNullEvent);
  PyModule_AddIntConstant (module, "Capture",			bz_eCaptureEvent);
  PyModule_AddIntConstant (module, "PlayerDie",			bz_ePlayerDieEvent);
  PyModule_AddIntConstant (module, "PlayerSpawn",		bz_ePlayerSpawnEvent);
  PyModule_AddIntConstant (module, "ZoneEntry",			bz_eZoneEntryEvent);
  PyModule_AddIntConstant (module, "ZoneExit",			bz_eZoneExitEvent);
  PyModule_AddIntConstant (module, "PlayerJoin",		bz_ePlayerJoinEvent);
  PyModule_AddIntConstant (module, "PlayerPart",		bz_ePlayerPartEvent);
  PyModule_AddIntConstant (module, "ChatMessage",		bz_eChatMessageEvent);
  PyModule_AddIntConstant (module, "UnknownSlashCommand",	bz_eUnknownSlashCommand);
  PyModule_AddIntConstant (module, "GetPlayerSpawnPosition",	bz_eGetPlayerSpawnPosEvent);
  PyModule_AddIntConstant (module, "GetAutoTeam",		bz_eGetAutoTeamEvent);
  PyModule_AddIntConstant (module, "AllowPlayer",		bz_eAllowPlayer);
  PyModule_AddIntConstant (module, "Tick",			bz_eTickEvent);
  PyModule_AddIntConstant (module, "GenerateWorld",		bz_eGenerateWorldEvent);
  PyModule_AddIntConstant (module, "GetPlayerInfo",		bz_eGetPlayerInfoEvent);
}

PyObject *
Event::GetSubModule ()
{
  return module;
}

}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

