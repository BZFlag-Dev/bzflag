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

#include "ViewItemMessages.h"
#include "MessageBuffer.h"
#include "MessageManager.h"
#include "bzfgl.h"
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

//
// ViewItemMessages
//

ViewItemMessages::ViewItemMessages(MessageBuffer* _msgBuffer) :
								msgBuffer(_msgBuffer),
								timeout(0.0f),
								input(false),
								shadow(false)
{
	// do nothing
}

ViewItemMessages::~ViewItemMessages()
{
	// do nothing
}

void					ViewItemMessages::setTimeout(float _timeout)
{
	timeout = _timeout;
}

void					ViewItemMessages::showInput(bool _input)
{
	input = _input;
}

void					ViewItemMessages::setShadow(bool _shadow)
{
	shadow = _shadow;
}

bool					ViewItemMessages::onPreRender(
								float, float, float, float)
{
	return true;
}

void					ViewItemMessages::onPostRender(
								float, float, float w, float h)
{
	// get line spacing
	float spacing = getState().font.getSpacing();

	// how many lines can we fit?
	unsigned int maxLines = (int)floorf(h / spacing);

	// prepare callback info to count the lines
	unsigned int numLines = maxLines;
	CallbackInfo cbState;
	cbState.self     = this;
	cbState.numLines = &numLines;
	cbState.w        = w;

	// how many lines are there?  clamp to max.
	if (numLines > 0 && input && msgBuffer->isComposing())
		--numLines;
	if (numLines > 0)
		msgBuffer->iterate(timeout, &countLine, &cbState);

	// compute actual number of lines
	numLines = maxLines - numLines;

	// prepare callback info for drawing
	cbState.x = 0.0f;
	switch (getState().vAlign) {
		default:
			cbState.y = spacing - getState().font.getAscent();
			break;

		case ViewState::Top:
			cbState.y = (maxLines - numLines + 1) * spacing - getState().font.getAscent();
			break;
	}

	// draw
	OpenGLGState::resetState();
	if (input && msgBuffer->isComposing()) {
		BzfString msg = msgBuffer->getComposePrompt();
		msg += msgBuffer->getComposeMessage();
		msg += "_";
		drawLine(msg, w, getState().color.color, &cbState);
	}
	if (numLines > 0)
		msgBuffer->iterate(timeout, &drawLineCB, &cbState);
}

bool					ViewItemMessages::countLine(
								const BzfString& msg,
								const float*, void* _cbState)
{
	CallbackInfo* cbState = reinterpret_cast<CallbackInfo*>(_cbState);

	// count lines after word breaking
	unsigned int numLines;
	BzfString buffer = cbState->self->wordBreak(
								cbState->self->getState().font,
								cbState->w, msg, &numLines);

	// decrement line count
	if (*(cbState->numLines) <= numLines) {
		*(cbState->numLines) = 0;
		return false;
	}
	else {
		*(cbState->numLines) -= numLines;
		return true;
	}
}

bool					ViewItemMessages::drawLine(
								const BzfString& msg,
								float w,
								const float* color,
								CallbackInfo* cbState)
{
	const OpenGLTexFont& font = getState().font;

	// do word breaking and count lines
	unsigned int numLines;
	BzfString buffer = wordBreak(font, w, msg, &numLines);

	float x = cbState->x;
	switch (getState().hAlign) {
		case ViewState::Center:
			x += 0.5f * (cbState->w - font.getWidth(buffer));
			if (shadow)
				x -= 0.5f;
			break;

		case ViewState::Right:
			x += cbState->w - font.getWidth(buffer);
			if (shadow)
				x -= 1.0f;
			break;
	}

	// move position for multiple lines
	cbState->y += (numLines - 1) * font.getSpacing();

	// draw
	if (shadow) {
		glColor3f(0.0f, 0.0f, 0.0f);
		font.draw(buffer, x, cbState->y - 1.0f);
		font.draw(buffer, x + 1.0f, cbState->y);
		font.draw(buffer, x + 1.0f, cbState->y - 1.0f);
	}
	glColor3fv(color);
	font.draw(buffer, x, cbState->y);

	// move position
	cbState->y += font.getSpacing();

	// numLines less lines
	const bool more = (*(cbState->numLines) > numLines);
	*(cbState->numLines) -= numLines;
	return more;
}

bool					ViewItemMessages::drawLineCB(
								const BzfString& msg,
								const float* color, void* _cbState)
{
	CallbackInfo* cbState = reinterpret_cast<CallbackInfo*>(_cbState);
	return cbState->self->drawLine(msg, cbState->w, color, cbState);
}

BzfString				ViewItemMessages::wordBreak(
								const OpenGLTexFont& font,
								float w,
								const BzfString& msg,
								unsigned int* numLines)
{
	BzfString split;
	*numLines = 0;
	const char* begin = msg.c_str();
	const char* end = begin + msg.size();
	const char* scan = strchr(begin, '\n');
	if (scan == NULL)
		scan = end;
	do {
		// check that portion fits on line
		int n = font.getLengthInWidth(w, begin, scan - begin);
		while (n < scan - begin) {
			// doesn't fit.  back up until we find a space.
			int n0 = n;
			while (n > 0 && !isspace(begin[n]))
				--n;

			// if we found a space then back up until we find a non-space
			while (n > 0 && isspace(begin[n - 1]))
				--n;

			// if we couldn't find a place to break then break inside a word
			if (n == 0)
				n = n0;

			// append the line
			split.append(begin, n);
			split += "\n";
			++(*numLines);

			// skip ahead
			begin += n;

			// skip leading whitespace
			while (begin != scan && isspace(*begin))
				++begin;

			// check next portion
			n = font.getLengthInWidth(w, begin, scan - begin);
		}

		// add remainder of line
		if (scan == end) {
			split.append(begin, scan - begin);
			++(*numLines);
		}
		else {
			split.append(begin, scan - begin + 1);
			++(*numLines);
			begin = scan + 1;
			scan  = strchr(begin, '\n');
			if (scan == NULL)
				scan = end;
		}
	} while (scan != end);

	return split;
}


//
// ViewItemMessagesReader
//

ViewItemMessagesReader::ViewItemMessagesReader() :
								item(NULL)
{
	// do nothing
}

ViewItemMessagesReader::~ViewItemMessagesReader()
{
	if (item != NULL)
		item->unref();
}

ViewTagReader* 			ViewItemMessagesReader::clone() const
{
	return new ViewItemMessagesReader;
}

View*					ViewItemMessagesReader::open(
								const ConfigReader::Values& values)
{
	assert(item == NULL);

	// get name parameter
	ConfigReader::Values::const_iterator index = values.find("buffer");
	if (index == values.end()) {
		// must have a name
		return NULL;
	}
	BzfString name = index->second;
	MessageBuffer* buffer = MSGMGR->get(name);
	if (buffer == NULL) {
		// unknown buffer
		return NULL;
	}

	// other parameters
	float timeout = 0.0;
	bool input = false, shadow = false;
	index = values.find("timeout");
	if (index != values.end())
		sscanf(index->second.c_str(), "%f", &timeout);
	index = values.find("input");
	if (index != values.end())
		input = (index->second == "yes");
	index = values.find("shadow");
	if (index != values.end())
		shadow = (index->second == "yes");

	// create item
	item = new ViewItemMessages(buffer);
	if (timeout >= 0.0f)
		item->setTimeout(timeout);
	item->showInput(input);
	item->setShadow(shadow);

	return item;
}

void					ViewItemMessagesReader::close()
{
	assert(item != NULL);
	item = NULL;
}
