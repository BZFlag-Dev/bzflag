/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
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

#ifndef BZF_KEYMANAGER_H
#define BZF_KEYMANAGER_H

#include "common.h"
#include "BzfEvent.h"
#include "BzfString.h"
#include "CallbackList.h"
#include <map>

#define KEYMGR (KeyManager::getInstance())

class KeyManager {
public:
	typedef void (*IterateCallback)(const BzfString& name, bool press,
							const BzfString& cmd, void* userData);
	typedef IterateCallback ChangeCallback;

						KeyManager();
						~KeyManager();

	// bind/unbind a command to/from a key event press or release
	void				bind(const BzfKeyEvent&,
								bool press, const BzfString& cmd);
	void				unbind(const BzfKeyEvent&, bool press);

	// get the command for a key event press or release
	BzfString			get(const BzfKeyEvent&, bool press) const;

	// convert a key event to/from a string
	BzfString			keyEventToString(const BzfKeyEvent&) const;
	bool				stringToKeyEvent(const BzfString&, BzfKeyEvent&) const;

	// invoke callback for each bound key
	void				iterate(IterateCallback callback, void* userData);

	// add/remove a callback to invoke when a key binding is added,
	// removed, or changed.
	void				addCallback(ChangeCallback, void* userData);
	void				removeCallback(ChangeCallback, void* userData);

	static KeyManager*	getInstance();

private:
	void				notify(const BzfKeyEvent&,
							bool press, const BzfString& cmd);

	struct CallbackInfo {
	public:
		BzfString		name;
		bool			press;
		BzfString		cmd;
	};
	static bool			onCallback(ChangeCallback, void*, void*);

private:
	class KeyEventLess {
	public:
		bool			operator()(const BzfKeyEvent&,
							const BzfKeyEvent&) const;
	};

	typedef std::map<BzfKeyEvent, BzfString, KeyEventLess> EventToCommandMap;
	typedef std::map<BzfString, BzfKeyEvent> StringToEventMap;

	EventToCommandMap	pressEventToCommand;
	EventToCommandMap	releaseEventToCommand;
	StringToEventMap	stringToEvent;
	CallbackList<ChangeCallback>	callbacks;
	static KeyManager*	instance;
	static const char*	buttonNames[];
	static const char*	asciiNames[][2];
};

#endif // BZF_KEYMANAGER_H
