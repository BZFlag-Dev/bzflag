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
#include <stdio.h>

//
// function objects for parsing view
//

class ViewSetColor_t : public unary_function<std::string, float*> {
public:
	ViewSetColor_t(float* color_) : color(color_) { }
	float*				operator()(const std::string& arg) const
	{
		float tmp[3];
		if (sscanf(arg.c_str(), "%f %f %f", tmp+0, tmp+1, tmp+2) != 3)
			throw XMLNode::AttributeException("invalid color");
		color[0] = tmp[0];
		color[1] = tmp[1];
		color[2] = tmp[2];
		return color;
	}

private:
	float*				color;
};

inline ViewSetColor_t
viewSetColor(float* color)
{
	return ViewSetColor_t(color);
}


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
	// clean up named items
	for (NamedItems::iterator index = namedItems.begin();
								index != namedItems.end(); ++index)
		index->second->unref();
	namedItems.clear();

	// clean up readers
	for (ItemReaders::iterator index = itemReaders.begin();
								index != itemReaders.end(); ++index)
		delete index->second;
	itemReaders.clear();
}

void					ViewReader::onTagReaderCB(
								const ViewTagReader* reader,
								const std::string& tag, void* self)
{
	reinterpret_cast<ViewReader*>(self)->insert(tag, reader->clone());
}

void					ViewReader::insert(const std::string& tag,
								ViewTagReader* reader)
{
	remove(tag);
	itemReaders.insert(std::make_pair(tag, reader));
}

void					ViewReader::remove(const std::string& tag)
{
	ItemReaders::iterator index = itemReaders.find(tag);
	if (index != itemReaders.end()) {
		delete index->second;
		itemReaders.erase(index);
	}
}

ConfigFileReader*		ViewReader::clone()
{
	ViewReader* reader = new ViewReader;
	for (ItemReaders::iterator index = itemReaders.begin();
								index != itemReaders.end(); ++index)
		reader->insert(index->first, index->second->clone());
	return reader;
}

void					ViewReader::parse(XMLTree::iterator xml)
{
	assert(stack.size() == 0);

	// set initial state
	State state;
	state.levels               = 0;
	state.state.fontSize.pixel = 12.0f;
	state.state.fontName       = "helveticaBoldItalic";
	state.state.font           = OpenGLTexFont(state.state.fontName);
	stack.push_back(state);

	try {
		// start parsing
		parseChildren(xml, &ViewReader::parseViews, (void*)0);

		// clean up
		stack.pop_back();
		assert(stack.empty());
	}
	catch (...) {
		// clean up
		stack.clear();
		throw;
	}
}

void					ViewReader::parseViews(XMLTree::iterator xml, void*)
{
	// data is unexpected
	if (xml->type != XMLNode::Tag)
		throw XMLIOException(xml->position,
							"unexpected data outside any view");

	// must be a view
	if (xml->value != "view")
		throw XMLIOException(xml->position, string_util::format(
							"invalid tag `%s'", xml->value.c_str()));

	// get id parameter
	std::string id;
	if (!xml->getAttribute("id", id))
		throw XMLIOException(xml->position, "view must have an id attribute");

	// create view
	View* view = new View;
	try {
		assert(!stack.empty());

		// prep the new view
		view->setState(stack.back().state);

		// parse the view
		parseChildren(xml, &ViewReader::parseView, view);

		// save view
		saveNamedItem(id, view);
		VIEWMGR->add(id, view);

		// done with view
		view->unref();
	}
	catch (...) {
		view->unref();
		throw;
	}
}

void					ViewReader::parseView(
							XMLTree::iterator xml, View* view)
{
	assert(view != NULL);

	// data is unexpected
	if (xml->type != XMLNode::Tag)
		throw XMLIOException(xml->position,
							"unexpected data outside any view control");

	// check standard tags first
	if (parseStandardTags(xml))
		return;

	// check for references
	if (xml->value == "ref") {
		// ref node must be empty
		if (xml.begin() != xml.end())
			throw XMLIOException(xml->position, "`ref' must be empty");

		// get id
		std::string id;
		if (!xml->getAttribute("id", id))
			throw XMLIOException(xml->position,
								"`ref' must have an `id' attribute");

		// find the named item
		NamedItems::const_iterator index = namedItems.find(id);
		if (index == namedItems.end())
			throw XMLIOException(xml->position, string_util::format(
								"unknown item `%s'", id.c_str()));

		// add named item as a child
		view->pushChild(index->second);
		return;
	}

	// it's not a built-in tag.  check against registered view types.
	ItemReaders::const_iterator index = itemReaders.find(xml->value);
	if (index == itemReaders.end())
		throw XMLIOException(xml->position,
								string_util::format(
									"invalid view type `%s'",
									xml->value.c_str()));

	// it's a registered type
	ViewTagReader* reader = NULL;
	try {
		// push stack
		stack.push_back(stack.back());

		// clone the reader
		reader = index->second->clone();

		// create the child view and prepare it
		View* child = reader->open(xml);
		child->setState(stack.back().state);

		// let the reader parse.  recurse on anything it rejects.
		XMLTree::sibling_iterator scan = xml.begin();
		XMLTree::sibling_iterator end  = xml.end();
		for (; scan != end; ++scan)
			if (!reader->parse(scan))
				parseView(scan, child);

		// finish up
		reader->close();

		// add child
		view->pushChild(child);

		// save view if named
		std::string id;
		if (xml->getAttribute("id", id))
			saveNamedItem(id, child);

		// clean up
		stack.pop_back();
		delete reader;
	}
	catch (...) {
		stack.pop_back();
		delete reader;
		throw;
	}
}

bool					ViewReader::parseStandardTags(
							XMLTree::iterator xml)
{
	static const XMLParseEnumList<ViewState::Align> s_enumHAlign[] = {
		{ "left",   ViewState::Left },
		{ "center", ViewState::Center },
		{ "right",  ViewState::Right },
		{ NULL, ViewState::Left }
	};
	static const XMLParseEnumList<ViewState::Align> s_enumVAlign[] = {
		{ "bottom", ViewState::Bottom },
		{ "center", ViewState::Center },
		{ "top",    ViewState::Top },
		{ NULL, ViewState::Bottom }
	};
	static const XMLParseEnumList<ViewColor::Source> s_enumColor[] = {
		{ "black",  ViewColor::Black },
		{ "white",  ViewColor::White },
		{ "team",   ViewColor::MyTeam },
		{ "red",    ViewColor::Red },
		{ "green",  ViewColor::Green },
		{ "blue",   ViewColor::Blue },
		{ "purple", ViewColor::Purple },
		{ "rogue",  ViewColor::Rogue },
		{ "king",   ViewColor::King },
		{ NULL, ViewColor::Black }
	};

	// parse
	State& state         = stack.back();
	ViewState& viewState = state.state;
	if (xml->value == "font") {
		std::string name;
		if (xml->getAttribute("name", name)) {
			OpenGLTexFont font(name);
			if (!font.isValid())
				throw XMLIOException(xml->getAttributePosition("name"),
							string_util::format(
								"unknown font name `%s'", name.c_str()));
			viewState.font     = font;
			viewState.fontName = name;
		}
		xml->getAttribute("size", viewSetSize(viewState.fontSize));
	}

	else if (xml->value == "align") {
		xml->getAttribute("h", xmlParseEnum(s_enumHAlign,
								xmlSetVar(viewState.hAlign)));
		xml->getAttribute("v", xmlParseEnum(s_enumVAlign,
								xmlSetVar(viewState.vAlign)));
	}

	else if (xml->value == "color") {
		std::string value;
		xml->getAttribute("scale", viewSetColor(viewState.color.scale));
		xml->getAttribute("shift", viewSetColor(viewState.color.shift));
		xml->getAttribute("alpha", xmlStrToFloat(xmlCompose(xmlCompose(
								xmlSetVar(viewState.color.color[3]),
								xmlMax(0.0f)), xmlMin(1.0f))));
		xml->getAttribute("value", xmlParseEnum(s_enumColor,
								xmlSetVar(viewState.color.source)));
	}

	else if (xml->value == "texture") {
		std::string filename;
		if (xml->getAttribute("filename", filename))
			viewState.texture = OpenGLTexture(filename, NULL,
										NULL, OpenGLTexture::Max);
		else
			viewState.texture = OpenGLTexture();
	}
	else {
		return false;
	}

	// all standard tags expect to be empty
	// (this isn't a general requirement, it just happens to be true)
	if (xml.begin() != xml.end())
		throw XMLIOException(xml->position, string_util::format(
							"`%s' must be empty", xml->value.c_str()));

	return true;
}

void					ViewReader::saveNamedItem(
							const std::string& id, View* item)
{
	assert(item != NULL);

	// keep a ref to the item
	item->ref();

	// add or replace item in map
	NamedItems::iterator index = namedItems.find(id);
	if (index == namedItems.end()) {
		namedItems.insert(std::make_pair(id, item));
	}
	else {
		index->second->unref();
		index->second = item;
	}
}

// crs -- this must come after all uses of it to work around a VC++ 6.0 bug
template <class T>
void					ViewReader::parseChildren(
							XMLTree::iterator xml,
							void (ViewReader::*method)(XMLTree::iterator, T*),
							T* data)
{
	try {
		stack.push_back(stack.back());
		XMLTree::sibling_iterator scan = xml.begin();
		XMLTree::sibling_iterator end  = xml.end();
		for (; scan != end; ++scan)
			(this->*method)(scan, data);
		stack.pop_back();
	}
	catch (...) {
		stack.pop_back();
		throw;
	}
}
// ex: shiftwidth=4 tabstop=4
