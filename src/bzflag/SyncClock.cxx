/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

// get our interface
#include "SyncClock.h"

ServerSyncedClock syncedClock;

ServerSyncedClock::ServerSyncedClock( double interval )
{
	lastPingTag = 0;

	pingInterval = interval;
	lastPingTime = -interval*2;
	serverTimeOffset = 0;
}

void ServerSyncedClock::update ( ServerLink *link )
{
	double current = TimeKeeper::getCurrent().getSeconds();
	if ( (lastPingTime < 0) || ((current - lastPingTime) >= pingInterval) )
	{
		// send a pingout
		if (!link)
		{
			// make sure that when we DO have a link we will update first thing
			lastPingTime = -1;
			return;
		}

		link->sendWhatTimeIsIt(++lastPingTag);
		outstandingPings[lastPingTag] = current;

		lastPingTime = current;
	}
}

void ServerSyncedClock::timeMessage ( unsigned char tag, double time )
{
	double sentTime = -1;
	double current = TimeKeeper::getCurrent().getSeconds();

	if (outstandingPings.find(tag) == outstandingPings.end())
		return;
	sentTime = outstandingPings[tag];
	outstandingPings.erase(outstandingPings.find(tag));

	if ( sentTime > current )	// Einstein says this can't happen no negitive pings
		return;

	double halfPing = (current-sentTime) * 0.5;

	double lagCompedServerTime = time + halfPing;

	serverTimeOffset = lagCompedServerTime - current;
}

double ServerSyncedClock::GetServerSeconds( void )
{
	return TimeKeeper::getCurrent().getSeconds() + serverTimeOffset;
}
