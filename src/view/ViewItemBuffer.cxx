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

#include "ViewItemBuffer.h"
#include "bzfgl.h"

static const GLenum		bufferMap[] = { GL_BACK, GL_BACK_LEFT, GL_BACK_RIGHT };

//
// ViewItemBuffer
//

ViewItemBuffer::Buffer	ViewItemBuffer::current = Both;

ViewItemBuffer::ViewItemBuffer() : buffer(Both)
{
	// do nothing
}

ViewItemBuffer::~ViewItemBuffer()
{
	// do nothing
}

void					ViewItemBuffer::setBuffer(Buffer _buffer)
{
	buffer = _buffer;
}

bool					ViewItemBuffer::onPreRender(
								float, float, float, float)
{
	saved = current;
	if (current != buffer) {
		current = buffer;
		glDrawBuffer(bufferMap[current]);
	}

	return true;
}

void					ViewItemBuffer::onPostRender(
								float, float, float, float)
{
	if (current != saved) {
		current = saved;
		glDrawBuffer(bufferMap[current]);
	}
}


//
// ViewItemBufferReader
//

ViewItemBufferReader::ViewItemBufferReader() : item(NULL)
{
	// do nothing
}

ViewItemBufferReader::~ViewItemBufferReader()
{
	if (item != NULL)
		item->unref();
}

ViewTagReader*			ViewItemBufferReader::clone() const
{
	return new ViewItemBufferReader;
}

View*					ViewItemBufferReader::open(
								const ConfigReader::Values& values)
{
	assert(item == NULL);

	// get buffer
	ViewItemBuffer::Buffer buffer = ViewItemBuffer::Both;
	ConfigReader::Values::const_iterator index;
	index = values.find("buffer");
	if (index != values.end()) {
		if (index->second == "left")
			buffer = ViewItemBuffer::Left;
		else if (index->second == "right")
			buffer = ViewItemBuffer::Right;
		else if (index->second == "both")
			buffer = ViewItemBuffer::Both;
		else
			return NULL;
	}

	// create item
	item = new ViewItemBuffer;
	item->setBuffer(buffer);

	return item;
}

void					ViewItemBufferReader::close()
{
	assert(item != NULL);
	item = NULL;
}
