/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * keyboard mapping stuff
 */

#ifndef	BZF_KEYMAP_H
#define	BZF_KEYMAP_H

#include "common.h"
#include "BzfEvent.h"
#include "BzfString.h"

class BzfKeyMap {
  public:
    enum Key {
			FireShot,
			DropFlag,
			Identify,
			ShortRange,
			MediumRange,
			LongRange,
			SendAll,
			SendTeam,
			SendNemesis,
			Jump,
			Binoculars,
			Score,
			FlagHelp,
			TimeForward,
			TimeBackward,
			Pause,
			Quit,
			ScrollBackward,
			ScrollForward,
                        SlowKeyboardMotion,
			LastKey
    };

			BzfKeyMap();
			~BzfKeyMap();

    void		resetAll();
    void		reset(Key);
    void		clear(Key);
    void		set(Key, const BzfKeyEvent&);
    void		unset(Key, const BzfKeyEvent&);
    const BzfKeyEvent&	get(Key) const;
    const BzfKeyEvent&	getAlternate(Key) const;
    Key			isMapped(char) const;
    Key			isMapped(BzfKeyEvent::Button) const;
    Key			isMapped(const BzfKeyEvent&) const;
    boolean		isMappedTo(Key, const BzfKeyEvent&) const;

    static BzfString	getKeyEventString(const BzfKeyEvent&);
    static BzfString	getKeyName(Key);
    static Key		lookupKeyName(const BzfString&);
    static boolean	translateStringToEvent(const BzfString&, BzfKeyEvent&);

  private:
    BzfKeyEvent		map1[LastKey];
    BzfKeyEvent		map2[LastKey];
    static const BzfKeyEvent	defaults1[];
    static const BzfKeyEvent	defaults2[];
    static const char*		keyName[];
    static const char*		eventNames[];
};

#endif // BZF_KEYMAP_H
