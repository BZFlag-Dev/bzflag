/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
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

#include <string>
#include "common.h"
#include "BzfEvent.h"

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
			SendRecipient,
			Jump,
			Binoculars,
                        Score,
                        Labels,
			FlagHelp,
			TimeForward,
			TimeBackward,
			Pause,
			Destruct,
			Quit,
			ScrollBackward,
			ScrollForward,
			SlowKeyboardMotion,
			ToggleRadarFlags,
			ToggleMainFlags,
			ChooseSilence,
			ServerCommand,
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
    bool		isMappedTo(Key, const BzfKeyEvent&) const;

    static std::string	getKeyEventString(const BzfKeyEvent&);
    static std::string	getKeyName(Key);
    static Key		lookupKeyName(const std::string&);
    static bool	translateStringToEvent(const std::string&, BzfKeyEvent&);

  private:
    BzfKeyEvent		map1[LastKey];
    BzfKeyEvent		map2[LastKey];
    static const BzfKeyEvent	defaults1[];
    static const BzfKeyEvent	defaults2[];
    static const char*		keyName[];
    static const char*		eventNames[];
};

#endif // BZF_KEYMAP_H
// ex: shiftwidth=2 tabstop=8
