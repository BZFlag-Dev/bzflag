/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	BZF_SYNCCLOCK_H
#define	BZF_SYNCCLOCK_H

#include "common.h"
#include "ServerLink.h"

#include "TimeKeeper.h"
#include <map>

#define _DEFAULT_SYNC_INTERVAL 60.0

class ServerSyncedClock
{
public:
	ServerSyncedClock( double interval = _DEFAULT_SYNC_INTERVAL );

	double GetServerSeconds( void );

	void update ( ServerLink *link );
	void timeMessage ( unsigned char tag, float time );

protected:
	std::map<int,double>	outstandingPings;
	unsigned char lastPingTag;

	double		  lastPingTime;
	double		  pingInterval;

	double serverTimeOffset;
};

extern ServerSyncedClock syncedClock;

#endif // BZF_SYNCCLOCK_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
