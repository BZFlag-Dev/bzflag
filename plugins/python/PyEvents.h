/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <Python.h>
#include "bzfsAPI.h"

#ifndef __PYTHON_EVENTS_H__
#define __PYTHON_EVENTS_H__

namespace Python
{

class BZFlag;

class PythonHandler : public bz_EventHandler
{
public:
	virtual void process (bz_EventData *eventData);
	BZFlag *parent;

protected:
	void emit (PyObject *arglist, int event);
};

#define PY_HANDLER(x) class x : public PythonHandler { public: virtual void process (bz_EventData *eventData); };
PY_HANDLER(CaptureHandler)
PY_HANDLER(DieHandler)
PY_HANDLER(SpawnHandler)
PY_HANDLER(ZoneEntryHandler)
PY_HANDLER(ZoneExitHandler)
PY_HANDLER(JoinHandler)
PY_HANDLER(PartHandler)
PY_HANDLER(ChatHandler)
PY_HANDLER(UnknownSlashHandler)
PY_HANDLER(GetSpawnPosHandler)
PY_HANDLER(GetAutoTeamHandler)
PY_HANDLER(AllowPlayerHandler)
PY_HANDLER(TickHandler)
PY_HANDLER(GenerateWorldHandler)
PY_HANDLER(GetPlayerInfoHandler)
#undef PY_HANDLER

};

#endif
