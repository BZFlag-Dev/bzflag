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

#if defined(WIN32)
#pragma warning(4:4503)
#endif

#include "MessageBuffer.h"
#include "BzfEvent.h"
#include <ctype.h>

//
// MessageBuffer
//

MessageBuffer::MessageBuffer(unsigned int _history) :
								history(_history),
								composeStopCallback(NULL),
								composeUserData(NULL),
								composing(false)
{
	// do nothing
}

MessageBuffer::~MessageBuffer()
{
	// do nothing
}

void					MessageBuffer::insert(
								const std::string& msg, const float* color)
{
	static const float defaultColor[] = { 1.0f, 1.0f, 1.0f };

	// clear out the oldest messages if full
	while (messages.size() >= history)
		messages.pop_back();

	// use default color if none specified
	if (color == NULL)
		color = defaultColor;

	// add message if not empty
	if (!msg.empty()) {
		Message message;
		message.time     = TimeKeeper::getCurrent();
		message.msg      = msg;
		message.color[0] = color[0];
		message.color[1] = color[1];
		message.color[2] = color[2];
		messages.push_front(message);
	}

	// notify callbacks
	CallbackInfo info;
	info.msg   = msg;
	info.color = color;
	callbacks.iterate(onCallback, &info);
}

void					MessageBuffer::clear()
{
	messages.clear();
}

unsigned int			MessageBuffer::size() const
{
	return messages.size();
}

void					MessageBuffer::iterate(float age,
								Callback callback, void* userData) const
{
	// get the current time
	const TimeKeeper time = TimeKeeper::getCurrent();

	// iterate over all messages, newest to oldest
	for (Messages::const_iterator index = messages.begin();
								index != messages.end(); ++index) {
		const Message& message = *index;

		// done if getting too old
		if (age > 0.0f && time - message.time > age)
			break;

		// invoke callback.  done if callback returns false.
		if (!(*callback)(message.msg, message.color, userData))
			break;
	}
}

void					MessageBuffer::addCallback(
								Callback callback, void* userData)
{
	callbacks.add(callback, userData);
}

void					MessageBuffer::removeCallback(
								Callback callback, void* userData)
{
	callbacks.remove(callback, userData);
}

bool					MessageBuffer::onCallback(
								Callback callback,
								void* userData,
								void* vinfo)
{
	CallbackInfo* info = reinterpret_cast<CallbackInfo*>(vinfo);
	callback(info->msg, info->color, userData);
	return true;
}

void					MessageBuffer::startComposing(
								const std::string& prompt,
								ComposeCallback callback,
								ComposeStopCallback stop,
								void* userData)
{
	// cancel existing composition
	if (composing)
		onCancelComposing(kCancel);

	// initialize
	composing           = true;
	composed            = "";
	composeCallback     = callback;
	composeStopCallback = stop;
	composeUserData     = userData;
	composePrompt       = prompt;
}

void					MessageBuffer::cancelComposing()
{
	onCancelComposing(kCancel);
}

void					MessageBuffer::onCancelComposing(StopReason reason)
{
	// save stop callback
	ComposeStopCallback oldStop = composeStopCallback;
	void* oldUserData           = composeUserData;

	// stop composing
	composing           = false;
	composed            = "";
	composeCallback     = NULL;
	composeStopCallback = NULL;
	composeUserData     = NULL;
	composePrompt       = "";

	// invoke stop callback
	if (oldStop != NULL)
		(*oldStop)(reason, oldUserData);
}

bool					MessageBuffer::isComposing() const
{
	return composing;
}

std::string				MessageBuffer::getComposePrompt() const
{
	return composePrompt;
}

std::string				MessageBuffer::getComposeMessage() const
{
	return composed;
}

bool					MessageBuffer::keyPress(const BzfKeyEvent& key)
{
	if (!composing)
		return false;

	char c = key.ascii;
	if (c == 0) switch (key.button) {
		// FIXME -- handle arrow keys for interior editing or history

		case BzfKeyEvent::Delete:
			c = '\b';
			break;

		default:
			return false;
	}

	// backspace
	if (c == '\b') {
		// do nothing if string is empty
		if (composed.empty())
			goto noRoom;

		// remove last character
		composed.resize(composed.size() - 1);
	}

	// done.  just eat these and let the key release do the work.
	else if (c == '\r' || c == 27) {
		return true;
	}

	// ignore non-printable characters except backspace
	else if (!isprint(c) && c != '\b') {
		return false;
	}

	// printable
	else {
		// convert all whitespace to space
		if (isspace(c))
			c = ' ';

		// append character
		composed.append(&c, 1);
	}

	return true;

noRoom:
	// ring bell?
	return true;
}

bool					MessageBuffer::keyRelease(const BzfKeyEvent& key)
{
	if (!composing)
		return false;

	char c = key.ascii;
	if (c == 0) switch (key.button) {
		// FIXME -- handle arrow keys for interior editing or history

		case BzfKeyEvent::Delete:
			c = '\b';
			break;

		default:
			return false;
	}

	// enter
	if (c == '\r') {
		if (composeCallback != NULL)
			(*composeCallback)(composed, composeUserData);
		onCancelComposing(kRun);
	}

	// cancel
	else if (c == 27) {
			onCancelComposing(kEscape);
	}

	// ignore non-printable characters except backspace
	else if (!isprint(c) && c != '\b') {
		return false;
	}

	return true;
}
