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

#ifndef __AUTOALLOWTIMER_H__
#define __AUTOALLOWTIMER_H__

/* common header */
#include "common.h"

/* common interface headers */
#include "TimeKeeper.h"

#include "WorldEventManager.h"

#define _MAX_WORLD_SHOTS 30

class AutoAllowTimerTickHandler : public bz_EventHandler
{
public:
	AutoAllowTimerTickHandler();
	virtual ~AutoAllowTimerTickHandler();

	virtual void process(bz_EventData *eventData);
	virtual bool autoDelete(void);

};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
