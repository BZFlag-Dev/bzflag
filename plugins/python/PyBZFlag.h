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

#include "PyEvent.h"
#include "PyTeam.h"
#include <Python.h>
#include <map>

#ifndef __PYTHON_BZFLAG_H__
#define __PYTHON_BZFLAG_H__

namespace Python
{

class BZFlag;

class CaptureHandler : public bz_EventHandler
{
public:
	virtual void process (bz_EventData *eventData);
	BZFlag *parent;
};

class TickHandler : public bz_EventHandler
{
public:
	virtual void process (bz_EventData *eventData);
	BZFlag *parent;
};

class JoinHandler : public bz_EventHandler
{
public:
	virtual void process (bz_EventData *eventData);
	BZFlag *parent;
};

class PartHandler : public bz_EventHandler
{
public:
	virtual void process (bz_EventData *eventData);
	BZFlag *parent;
};

class BZFlag
{
public:
	static BZFlag *GetInstance ();
	static int References;
	static void DeRef ();

	PyObject *GetListeners (int event);
	void AddPlayer (int id);
	void RemovePlayer (int id);

protected:
	BZFlag ();
private:
	static BZFlag *instance;
	Event *event_sub;
	Team  *team_sub;
	PyObject *event_listeners;
	PyObject *players;
	PyObject *module;

	CaptureHandler capture_handler;
	TickHandler    tick_handler;
	JoinHandler    join_handler;
	PartHandler    part_handler;
};

};

#endif
