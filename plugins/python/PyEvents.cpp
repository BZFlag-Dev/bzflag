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

#include "PyEvents.h"
#include "PyBZFlag.h"

namespace Python
{

const char *
event_to_name (int event)
{
  switch (event) {
  case bz_eNullEvent:		return "Null";
  case bz_eCaptureEvent:	return "Capture";
  case bz_ePlayerDieEvent:	return "PlayerDie";
  case bz_ePlayerSpawnEvent:	return "PlayerSpawn";
  case bz_eZoneEntryEvent:	return "ZoneEntry";
  case bz_eZoneExitEvent:	return "ZoneExit";
  case bz_ePlayerJoinEvent:	return "PlayerJoin";
  case bz_ePlayerPartEvent:	return "PlayerPart";
  case bz_eChatMessageEvent:	return "ChatMessage";
  case bz_eUnknownSlashCommand:	return "UnknownSlashCommand";
  case bz_eGetPlayerSpawnPosEvent: return "GetPlayerSpawnPosition";
  case bz_eGetAutoTeamEvent:	return "GetAutoTeam";
  case bz_eAllowPlayer:		return "AllowPlayer";
  case bz_eTickEvent:		return "Tick";
  case bz_eGenerateWorldEvent:	return "GenerateWorld";
  case bz_eGetPlayerInfoEvent:	return "GetPlayerInfo";
  case bz_eAllowSpawn:		return "AllowSpawn";
  case bz_eListServerUpdateEvent: return "ListServerUpdate";
  case bz_eBanEvent: return "Ban";
  case bz_eHostBanEvent: return "HostBan";
  }
  return NULL;
}

void
Handler::emit (PyObject *arglist, int event)
{
  PyObject *listeners = parent->GetListeners (event);

  if (listeners == NULL || !PyList_Check(listeners)) {
    // FIXME - throw error
    fprintf (stderr, "%s listeners is not a list!\n", event_to_name (event));
    return;
  }

  // Call out
  int size = PyList_Size (listeners);
  for (int i = 0; i < size; i++) {
    PyObject *handler = PyList_GetItem (listeners, i);
    if (!PyCallable_Check (handler)) {
      // FIXME - throw error
      fprintf (stderr, "%s listener is not callable\n", event_to_name (event));
      Py_DECREF (arglist);
      return;
    }
    PyErr_Clear ();
    PyEval_CallObject (handler, arglist);
    if (PyErr_Occurred ()) {
      PyErr_Print ();
      return;
    }
  }
}

void
CaptureHandler::process (bz_EventData *eventData)
{
  bz_CTFCaptureEventData *ced = (bz_CTFCaptureEventData *) eventData;

  PyObject *arglist = Py_BuildValue ("(iii(fff)fd)",
      ced->teamCapped,
      ced->teamCapping,
      ced->playerCapping,
      ced->pos[0],
      ced->pos[1],
      ced->pos[2],
      ced->rot,
      ced->time);
  emit (arglist, bz_eCaptureEvent);
  Py_DECREF (arglist);
}

void
DieHandler::process (bz_EventData *eventData)
{
  bz_PlayerDieEventData *pded = (bz_PlayerDieEventData *) eventData;

  PyObject *arglist = Py_BuildValue ("(iiiis(fff)fd)",
      pded->playerID,
      pded->team,
      pded->killerID,
      pded->killerTeam,
      pded->flagKilledWith.c_str(),
      pded->pos[0],
      pded->pos[1],
      pded->pos[2],
      pded->rot,
      pded->time);
  emit (arglist, bz_ePlayerDieEvent);
  Py_DECREF (arglist);
}

void
SpawnHandler::process (bz_EventData *eventData)
{
  bz_PlayerSpawnEventData *psed = (bz_PlayerSpawnEventData *) eventData;

  PyObject *arglist = Py_BuildValue ("(ii(fff)fd)",
      psed->playerID,
      psed->team,
      psed->pos[0],
      psed->pos[1],
      psed->pos[2],
      psed->rot,
      psed->time);
  emit (arglist, bz_ePlayerSpawnEvent);
  Py_DECREF (arglist);
}

void
ZoneEntryHandler::process (bz_EventData *eventData)
{
  emit (NULL /* FIXME? */, bz_eZoneEntryEvent);
}

void
ZoneExitHandler::process (bz_EventData *eventData)
{
  emit (NULL /* FIXME? */, bz_eZoneExitEvent);
}

void
JoinHandler::process (bz_EventData *eventData)
{
  bz_PlayerJoinPartEventData *pjped = (bz_PlayerJoinPartEventData*) eventData;
  parent->AddPlayer (pjped->playerID);

  PyObject *arglist = Py_BuildValue ("(iissd)",
      pjped->playerID,
      pjped->team,
      pjped->callsign.c_str (),
      pjped->reason.c_str (),
      pjped->time);

  emit (arglist, bz_ePlayerJoinEvent);
  Py_DECREF (arglist);
}

void
PartHandler::process (bz_EventData *eventData)
{
  bz_PlayerJoinPartEventData *pjped = (bz_PlayerJoinPartEventData*) eventData;
  parent->RemovePlayer (pjped->playerID);

  PyObject *arglist = Py_BuildValue ("(iissd)",
      pjped->playerID,
      pjped->team,
      pjped->callsign.c_str (),
      pjped->reason.c_str (),
      pjped->time);

  emit (arglist, bz_ePlayerPartEvent);
  Py_DECREF (arglist);
}

void
ChatHandler::process (bz_EventData *eventData)
{
  bz_ChatEventData *ced = (bz_ChatEventData *) eventData;

  PyObject *arglist = Py_BuildValue ("(iisd)",
      ced->from,
      ced->to,
      ced->message.c_str (),
      ced->time);
  emit (arglist, bz_eChatMessageEvent);
  Py_DECREF (arglist);
}

void
UnknownSlashHandler::process (bz_EventData *eventData)
{
  bz_UnknownSlashCommandEventData *usced = (bz_UnknownSlashCommandEventData *) eventData;

  PyObject *arglist = Py_BuildValue ("(iosd)",
      usced->from,
      usced->handled ? Py_True : Py_False,
      usced->message.c_str (),
      usced->time);
  emit (arglist, bz_eUnknownSlashCommand);
  Py_DECREF (arglist);
}

void
GetSpawnPosHandler::process (bz_EventData *eventData)
{
  bz_GetPlayerSpawnPosEventData *gpsped = (bz_GetPlayerSpawnPosEventData *) eventData;

  // FIXME - this probably has some way to return data to the main program
  PyObject *arglist = Py_BuildValue ("(iio(fff)fd)",
      gpsped->playerID,
      gpsped->team,
      gpsped->handled ? Py_True : Py_False,
      gpsped->pos[0],
      gpsped->pos[1],
      gpsped->pos[2],
      gpsped->rot,
      gpsped->time);
  emit (arglist, bz_eGetPlayerSpawnPosEvent);
  Py_DECREF (arglist);
}

void
GetAutoTeamHandler::process (bz_EventData *eventData)
{
  emit (NULL, bz_eGetAutoTeamEvent);
}

void
AllowPlayerHandler::process (bz_EventData *eventData)
{
  bz_AllowPlayerEventData *aped = (bz_AllowPlayerEventData *) eventData;

  // FIXME - this probably has some way to return data to the main program
  PyObject *arglist = Py_BuildValue ("(isssod)",
      aped->playerID,
      aped->callsign.c_str (),
      aped->ipAddress.c_str (),
      aped->reason.c_str (),
      aped->allow ? Py_True : Py_False,
      aped->time);
  emit (arglist, bz_eAllowPlayer);
  Py_DECREF (arglist);
}

void
GenerateWorldHandler::process (bz_EventData *eventData)
{
  bz_GenerateWorldEventData *gwed = (bz_GenerateWorldEventData *) eventData;

  PyObject *arglist = Py_BuildValue ("(ood)",
      gwed->handled ? Py_True : Py_False,
      gwed->ctf ? Py_True : Py_False,
      gwed->time);
  emit (arglist, bz_eGenerateWorldEvent);
  // FIXME - dunno why this is crashing
  //Py_DECREF (arglist);
}

void
GetPlayerInfoHandler::process (bz_EventData *eventData)
{
  bz_GetPlayerInfoEventData *gpied = (bz_GetPlayerInfoEventData *) eventData;

  PyObject *arglist = Py_BuildValue ("(issioood)",
      gpied->playerID,
      gpied->callsign.c_str (),
      gpied->ipAddress.c_str (),
      gpied->team,
      gpied->admin ? Py_True : Py_False,
      gpied->verified ? Py_True : Py_False,
      gpied->registered ? Py_True : Py_False,
      gpied->time);
  emit (arglist, bz_eGetPlayerInfoEvent);
  Py_DECREF (arglist);
}

void
TickHandler::process (bz_EventData *eventData)
{
  bz_TickEventData *ted = (bz_TickEventData*) eventData;

  PyObject *arglist = Py_BuildValue ("(d)", ted->time);
  emit (arglist, bz_eTickEvent);
  Py_DECREF (arglist);
}


void
AllowSpawnHandler::process (bz_EventData *eventData)
{
  bz_AllowSpawnData *ased = (bz_AllowSpawnData*) eventData;
  PyObject *arglist = Py_BuildValue ("(iiod)",
      ased->playerID,
      ased->team,
      ased->handled ? Py_True : Py_False,
      ased->time);
  emit (arglist, bz_eAllowSpawn);
  // FIXME - return
  Py_DECREF (arglist);
}

void
ListServerUpdateHandler::process (bz_EventData *eventData)
{
  bz_ListServerUpdateEvent *lsued = (bz_ListServerUpdateEvent*) eventData;
}

void
BanEventHandler::process (bz_EventData *eventData)
{
	bz_BanEventData  *bed = (bz_BanEventData*) eventData;

	PyObject *arglist = Py_BuildValue ("(iiiss)",
      bed->bannerID,
      bed->banneeID,
      bed->duration,
      bed->ipAddress.c_str (),
      bed->reason.c_str ());
	emit (arglist, bz_eBanEvent);
	Py_DECREF (arglist);
}

void
HostBanEventHandler::process (bz_EventData *eventData)
{
	bz_HostBanEventData  *hbed = (bz_HostBanEventData*) eventData;

	PyObject *arglist = Py_BuildValue ("(iiss)",
      hbed->bannerID,
      hbed->duration,
      hbed->hostPattern.c_str (),
      hbed->reason.c_str ());
	emit (arglist, bz_eHostBanEvent);
	Py_DECREF (arglist);
}

};

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

