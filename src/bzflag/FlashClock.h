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

#ifndef __FLASHCLOCK_H__
	#define __FLASHCLOCK_H__

/* common interface headers */
	#include "TimeKeeper.h"


/**
 * FlashClock
 *	keeps track of time for something that flashes
 */
class FlashClock
{
public:
	FlashClock();
	~FlashClock();

	void setClock( float time );
	void setClock( float time, float onTime, float offTime );

	bool isOn();

private:
	TimeKeeper startTime;
	float duration;
	float onDuration;
	float flashDuration;
};


#endif /* _FLASHCLOCK_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
