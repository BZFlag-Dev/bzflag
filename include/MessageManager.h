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

#ifndef BZF_MESSAGE_MANAGER_H
#define BZF_MESSAGE_MANAGER_H

#include "MessageBuffer.h"

#define MSGMGR (MessageManager::getInstance())

class MessageManager {
public:
	virtual ~MessageManager();

	// create a message buffer
	MessageBuffer*		create(const std::string& bufferName,
								unsigned int bufferLength);

	// get a message buffer.  returns NULL if no such buffer.
	MessageBuffer*		get(const std::string& bufferName) const;

	// add a message.  color may be NULL to use a default color.
	// does nothing if no such buffer.
	void				insert(const std::string& bufferName,
							const std::string& msg,
							const float* color = NULL);

	// handle key events.  these return true if composing and the
	// key was used for the message, otherwise they return false.
	// each buffer is tried until one handles the key.
	bool				keyPress(const BzfKeyEvent&);
	bool				keyRelease(const BzfKeyEvent&);

	// get the singleton instance
	static MessageManager* getInstance();

private:
	MessageManager();

private:
	typedef std::map<std::string, MessageBuffer*> BufferMap;

	BufferMap			buffers;
	static MessageManager* mgr;
};

#endif
// ex: shiftwidth=4 tabstop=4
