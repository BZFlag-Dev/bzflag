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

#ifndef BZF_MESSAGE_BUFFER_H
#define BZF_MESSAGE_BUFFER_H

#include "common.h"
#include <string>
#include "CallbackList.h"
#include "TimeKeeper.h"
#include <list>
#include <map>

class BzfKeyEvent;

class MessageBuffer {
public:
	enum StopReason {
		kCancel,		// startComposing() or cancelComposing() was called
		kRun,			// command was executed (user pressed enter)
		kEscape			// user pressed escape
	};

	typedef bool (*Callback)(const std::string& msg,
								const float* color, void* userData);
	typedef void (*ComposeCallback)(const std::string& msg, void* userData);
	typedef void (*ComposeStopCallback)(StopReason, void* userData);

	MessageBuffer(unsigned int historySize = 100);
	virtual ~MessageBuffer();

	// add a message.  color may be NULL to use a default color.
	void				insert(const std::string&, const float* color = NULL);

	// remove all messages
	void				clear();

	// get the number of messages currently stored
	unsigned int		size() const;

	// invoke a callback for every message that's newer than the given
	// age.  if age is <= 0 then return all messages.  stops iterating
	// when a callback returns false.  messages are returned in order
	// from newest to oldest.
	void				iterate(float age, Callback, void* userData) const;

	// add/remove a callback to invoke when a message is added.
	// return value of callback is ignored.
	void				addCallback(Callback, void* userData);
	void				removeCallback(Callback, void* userData);

	// start/cancel interactive message composition.  this object
	// doesn't display the message being composed, it just composes it.
	void				startComposing(const std::string& prompt,
							ComposeCallback onRun,
							ComposeStopCallback onStop, void* userData);
	void				cancelComposing();
	bool				isComposing() const;
	std::string			getComposePrompt() const;
	std::string			getComposeMessage() const;

	// these return true if composing and the key was used for the
	// message, otherwise they return false.
	bool				keyPress(const BzfKeyEvent&);
	bool				keyRelease(const BzfKeyEvent&);

private:
	void				onCancelComposing(StopReason);

	struct CallbackInfo {
	public:
		std::string		msg;
		const float*	color;
	};
	static bool			onCallback(Callback, void*, void*);

private:
	struct Message {
	public:
		TimeKeeper		time;
		std::string		msg;
		float			color[3];
	};
	typedef std::list<Message> Messages;

	unsigned int		history;
	Messages			messages;
	CallbackList<Callback>	callbacks;
	ComposeCallback		composeCallback;
	ComposeStopCallback	composeStopCallback;
	void*				composeUserData;
	std::string			composePrompt;
	bool				composing;
	std::string			composed;
};

#endif
// ex: shiftwidth=4 tabstop=4
