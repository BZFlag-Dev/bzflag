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

#include "ViewReader.h"
#include "ViewManager.h"
#include "View.h"
#include "ErrorHandler.h"
#include <stdio.h>

//
// ViewReader
//

ViewReader::ViewReader()
{
	// add known tag readers
	VIEWMGR->iterateReaders(onTagReaderCB, this);
}

ViewReader::~ViewReader()
{
	// clean up
	for (ItemReaders::iterator index = itemReaders.begin();
								index != itemReaders.end(); ++index)
		delete index->second;
	itemReaders.clear();
}

void					ViewReader::onTagReaderCB(
								const ViewTagReader* reader,
								const BzfString& tag, void* self)
{
	reinterpret_cast<ViewReader*>(self)->insert(tag, reader->clone());
}

void					ViewReader::insert(const BzfString& tag,
								ViewTagReader* reader)
{
	remove(tag);
	itemReaders.insert(std::make_pair(tag, reader));
}

void					ViewReader::remove(const BzfString& tag)
{
	ItemReaders::iterator index = itemReaders.find(tag);
	if (index != itemReaders.end()) {
		delete index->second;
		itemReaders.erase(index);
	}
}

bool					ViewReader::read(istream& s)
{
	assert(stack.size() == 0);

	// set initial state
	State state;
	state.view                 = NULL;
	state.reader               = NULL;
	state.levels               = 0;
	state.state.fontSize.pixel = 12.0f;
	state.state.fontName       = "helveticaBoldItalic";
	state.state.font           = OpenGLTexFont(state.state.fontName);
	stack.push_back(state);

	// prep config reader
	ConfigReader reader;
	reader.push(&openTopCB, &closeTopCB, NULL);

	// read stream
	const bool result = reader.read(s, this);

	// clean up
	assert(stack.size() >= 1);
	while (!stack.empty()) {
		if (stack.back().view != NULL)
			stack.back().view->unref();
		stack.pop_back();
	}
	for (NamedItems::iterator index = namedItems.begin();
								index != namedItems.end(); ++index)
		index->second->unref();
	namedItems.clear();

	return result;
}

bool					ViewReader::openTop(ConfigReader* reader,
								const BzfString& tag,
								const ConfigReader::Values& values)
{
	if (tag == "view") {
		ConfigReader::Values::const_iterator index = values.find("id");
		if (index == values.end()) {
			printError("%s: view must have an id",
								reader->getPosition().c_str());
			return false;
		}

		stack.push_back(stack.back());
		State& state = stack.back();
		state.view   = new View;
		viewName     = index->second;
		state.view->setState(state.state);
		reader->push(&openViewCB, &closeViewCB, &dataViewCB);

		// save named item
		if (namedItems.find(viewName) == namedItems.end()) {
			state.view->ref();
			namedItems.insert(std::make_pair(viewName, state.view));
		}
	}
	else {
		printError("%s: ignoring tag %s",
								reader->getPosition().c_str(),
								tag.c_str());
		reader->push(NULL, NULL, NULL);
	}
	return true;
}

bool					ViewReader::closeTop(const BzfString& tag)
{
	if (tag == "view") {
		assert(stack.back().view != NULL);
		VIEWMGR->add(viewName, stack.back().view);
		stack.back().view->unref();
		stack.pop_back();
		viewName = "";
	}
	return true;
}

bool					ViewReader::openView(ConfigReader* reader,
								const BzfString& tag,
								const ConfigReader::Values& values)
{
	// get useful stuff
	State& state         = stack.back();
	ViewState& viewState = state.state;

	if (tag == "font") {
		OpenGLTexFont font(viewState.font);
		BzfString name(viewState.fontName);

		ConfigReader::Values::const_iterator index;
		index = values.find("name");
		if (index != values.end()) {
			OpenGLTexFont tmp(index->second);
			if (!tmp.isValid()) {
				printError("%s: ignoring unknown font name: %s",
								reader->getPosition().c_str(),
								index->second.c_str());
			}
			else {
				font = tmp;
				name = index->second;
			}
		}

		// get size
		readSize(values, "size", viewState.fontSize,
								viewState.fontSize.pixel,
								viewState.fontSize.fraction);

		// save new state
		viewState.font     = font;
		viewState.fontName = name;
	}

	else if (tag == "align") {
		ConfigReader::Values::const_iterator index;
		index = values.find("h");
		if (index != values.end()) {
			const BzfString value = index->second;
			if (value == "left")
				viewState.hAlign = ViewState::Left;
			else if (value == "center")
				viewState.hAlign = ViewState::Center;
			else if (value == "right")
				viewState.hAlign = ViewState::Right;
			else
				printError("%s: ignoring invalid alignment ref: %s",
								reader->getPosition().c_str(),
								value.c_str());
		}
		index = values.find("v");
		if (index != values.end()) {
			const BzfString value = index->second;
			if (value == "bottom")
				viewState.vAlign = ViewState::Bottom;
			else if (value == "center")
				viewState.vAlign = ViewState::Center;
			else if (value == "top")
				viewState.vAlign = ViewState::Top;
			else
				printError("%s: ignoring invalid alignment ref: %s",
								reader->getPosition().c_str(),
								value.c_str());
		}
	}

	else if (tag == "color") {
		parseColor(viewState.color, values);
	}

	else if (tag == "texture") {
		ConfigReader::Values::const_iterator index;
		index = values.find("file");
		if (index != values.end()) {
			viewState.texture = OpenGLTexture(index->second,
								NULL, NULL, OpenGLTexture::Max);
		}
		else {
			viewState.texture = OpenGLTexture();
		}
	}

	else if (tag == "ref") {
		ConfigReader::Values::const_iterator index = values.find("id");
		if (index == values.end()) {
			printError("%s: ref must have an id",
								reader->getPosition().c_str());
			return false;
		}

		// lookup named item
		NamedItems::const_iterator index2 = namedItems.find(index->second);
		if (index2 == namedItems.end()) {
			printError("%s: ref of unknown id %s ignored",
								reader->getPosition().c_str(),
								index->second.c_str());
		}

		else {
			// add named item as a child
			assert(stack.back().view != NULL);
			stack.back().view->pushChild(index2->second);
		}
	}

	else {
		// not a built-in tag.  check for registered view types.
		ItemReaders::const_iterator index = itemReaders.find(tag);
		if (index != itemReaders.end()) {
			// it's a registered view type.  push new state.
			stack.push_back(stack.back());
			State& newState = stack.back();

			// create a new reader and set the state it should use
			newState.reader = index->second->clone();
			newState.reader->setState(&newState.state);

			// try opening the reader.
			newState.view = newState.reader->open(values);
			if (newState.view == NULL) {
				// failed to open
				printError("%s: failed to open %s",
								reader->getPosition().c_str(),
								tag.c_str());
				delete newState.reader;
				stack.pop_back();
				return false;
			}

			// set current state on view
			newState.view->setState(stack[stack.size() - 2].state);

			// save item if named (and name hasn't been used yet)
			ConfigReader::Values::const_iterator index = values.find("id");
			if (index != values.end() &&
		  namedItems.find(index->second) == namedItems.end()) {
				newState.view->ref();
				namedItems.insert(std::make_pair(index->second, newState.view));
			}

			return true;
		}

		// let top-of-stack reader have a crack at the tag
		else if (state.reader != NULL) {
			if (state.reader->push(tag, values)) {
				// reader accepted the tag
				++state.levels;
				return true;
			}
		}

		printError("%s: ignoring invalid tag: %s",
								reader->getPosition().c_str(),
								tag.c_str());
	}

	// ignore everything in a state tag (or an unknown tag)
	reader->push(NULL, NULL, NULL);

	return true;
}

bool					ViewReader::closeView(const BzfString& tag)
{
	assert(stack.size() > 1);
	State& state = stack.back();

	if (tag == "font" ||
		tag == "align" ||
		tag == "color" ||
		tag == "texture" ||
		tag == "ref") {
		// ignore the built-in tag close
	}

	else {
		assert(state.reader != NULL);
		if (state.levels > 0) {
			state.reader->pop(tag);
			--state.levels;
		}

		// closing a tag reader
		else {
			// close and destroy tag reader
			state.reader->close();
			delete state.reader;

			// add the view we just closed to its parent view
			View* newView = state.view;
			stack.pop_back();
			assert(stack.back().view != NULL);
			stack.back().view->pushChild(newView);
			newView->unref();
		}
	}

	return true;
}

bool					ViewReader::dataView(ConfigReader* reader,
								const BzfString& data)
{
	State& state = stack.back();
	if (state.reader != NULL) {
		if (state.reader->data(data))
			return true;

		// tag reader choked
		printError("%s: read error",
								reader->getPosition().c_str());
		return false;
	}

	// discard data outside of tag readers
	return true;
}

bool					ViewReader::readSize(
								const ConfigReader::Values& values,
								const BzfString& name,
								ViewSize& value,
								float pixel,
								float fraction)
{
	ConfigReader::Values::const_iterator index;
	index = values.find(name);
	if (index != values.end()) {
		float size;
		char type;
		if (sscanf(index->second.c_str(), "%f%c", &size, &type) == 2) {
			if (type == 'p') {
				value.pixel    = size;
				value.fraction = 0.0f;
				return false;
			}
			else if (type == '%') {
				value.pixel    = 0.0f;
				value.fraction = 0.01f * size;
				return false;
			}
			else if (type == 'x') {
				value.pixel    = 0.0f;
				value.fraction = size;
				return true;
			}
		}
	}

	value.pixel    = pixel;
	value.fraction = fraction;
	return false;
}

bool					ViewReader::parseColor(ViewColor& color,
								const ConfigReader::Values& values)
{
	ConfigReader::Values::const_iterator index;
	index = values.find("scale");
	if (index != values.end())
		parseColor(index->second, color.scale);
	index = values.find("shift");
	if (index != values.end())
		parseColor(index->second, color.shift);
	index = values.find("alpha");
	if (index != values.end())
		color.color[3] = atof(index->second.c_str());
	index = values.find("value");
	if (index != values.end()) {
		ViewColor::Source source = color.source;
		const BzfString value = index->second;
		if (value == "black")
			source = ViewColor::Black;
		else if (value == "white")
			source = ViewColor::White;
		else if (value == "team")
			source = ViewColor::MyTeam;
		else if (value == "red")
			source = ViewColor::Red;
		else if (value == "green")
			source = ViewColor::Green;
		else if (value == "blue")
			source = ViewColor::Blue;
		else if (value == "purple")
			source = ViewColor::Purple;
		else if (value == "rogue")
			source = ViewColor::Rogue;
		color.source = source;
	}
	return true;
}

bool					ViewReader::parseColor(
								const BzfString& value, float* color)
{
	float tmp[3];
	if (sscanf(value.c_str(), "%f %f %f", tmp+0, tmp+1, tmp+2) == 3) {
		color[0] = tmp[0];
		color[1] = tmp[1];
		color[2] = tmp[2];
		return true;
	}
	return false;
}

bool					ViewReader::openTopCB(ConfigReader* reader,
								const BzfString& tag,
								const ConfigReader::Values& values,
								void* self)
{
	return reinterpret_cast<ViewReader*>(self)->openTop(reader, tag, values);
}

bool					ViewReader::openViewCB(ConfigReader* reader,
								const BzfString& tag,
								const ConfigReader::Values& values,
								void* self)
{
	return reinterpret_cast<ViewReader*>(self)->openView(reader, tag, values);
}

bool					ViewReader::closeTopCB(ConfigReader*,
								const BzfString& tag,
								void* self)
{
	return reinterpret_cast<ViewReader*>(self)->closeTop(tag);
}

bool					ViewReader::closeViewCB(ConfigReader*,
								const BzfString& tag,
								void* self)
{
	return reinterpret_cast<ViewReader*>(self)->closeView(tag);
}

bool					ViewReader::dataViewCB(ConfigReader* reader,
								const BzfString& data,
								void* self)
{
	return reinterpret_cast<ViewReader*>(self)->dataView(reader, data);
}
