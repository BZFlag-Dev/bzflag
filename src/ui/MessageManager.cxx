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

#include "MessageManager.h"

//
// MessageManager
//

MessageManager*			MessageManager::mgr = NULL;

MessageManager::MessageManager()
{
	// do nothing
}

MessageManager::~MessageManager()
{
	// destroy buffers
	for (BufferMap::iterator index = buffers.begin();
								index != buffers.end(); ++index)
		delete index->second;

	mgr = NULL;
}

MessageManager* 	MessageManager::getInstance()
{
	if (mgr == NULL)
		mgr = new MessageManager;
	return mgr;
}

MessageBuffer*			MessageManager::create(
								const std::string& bufferName,
								unsigned int bufferLength)
{
	BufferMap::const_iterator index = buffers.find(bufferName);
	if (index == buffers.end()) {
		MessageBuffer* buffer = new MessageBuffer(bufferLength);
		buffers.insert(std::make_pair(bufferName, buffer));
		return buffer;
	}
	else {
		return index->second;
	}
}

MessageBuffer*			MessageManager::get(const std::string& bufferName) const
{
	BufferMap::const_iterator index = buffers.find(bufferName);
	if (index == buffers.end())
		return NULL;
	else
		return index->second;
}

void					MessageManager::insert(const std::string& bufferName,
								const std::string& msg, const float* color)
{
	MessageBuffer* buffer = get(bufferName);
	if (buffer != NULL)
		buffer->insert(msg, color);
}

bool					MessageManager::keyPress(const BzfKeyEvent& key)
{
	for (BufferMap::iterator index = buffers.begin();
								index != buffers.end(); ++index)
		if (index->second->keyPress(key))
			return true;
	return false;
}

bool					MessageManager::keyRelease(const BzfKeyEvent& key)
{
	for (BufferMap::iterator index = buffers.begin();
								index != buffers.end(); ++index)
		if (index->second->keyRelease(key))
			return true;
	return false;
}
