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

#include "MenuReader.h"
#include "MenuControls.h"
#include "MenuManager.h"
#include "StateDatabase.h"
#include <stdio.h>

//
// function objects for parsing menus
//

template <class Object>
class MenuSetSizeMethod_t : public std::unary_function<std::string, void> {
public:
	typedef void (Object::*Member)(float, float);
	MenuSetSizeMethod_t(Object* object_, Member member_) :
							object(object_), member(member_) { }
	void				operator()(const std::string& arg) const
	{
		char type;
		float value;
		if (sscanf(arg.c_str(), "%f%c", &value, &type) == 2 &&
			(type == 'p' || type == '%')) {
			if (type == 'p')
				(object->*member)(value, 0.0f);
			else
				(object->*member)(0, 0.01f * value);
		}
		else {
			throw XMLNode::AttributeException("invalid value");
		}
	}

private:
	Object*				object;
	Member				member;
};

template <class Object>
inline MenuSetSizeMethod_t<Object>
menuSetSizeMethod(Object* object, void (Object::*member)(float, float))
{
	return MenuSetSizeMethod_t<Object>(object, member);
}


//
// MenuReader
//

MenuReader::MenuReader()
{
	// do nothing
}

MenuReader::~MenuReader()
{
	// do nothing
}

ConfigFileReader*		MenuReader::clone()
{
	return new MenuReader;
}

void					MenuReader::parse(XMLTree::iterator xml)
{
	assert(stateStack.size() == 0);

	// set initial state
	State state;
	state.menu      = NULL;
	state.align     = Menu::Center;
	state.xIsPixels = false;
	state.yIsPixels = false;
	state.x         = 0.5f;
	state.y         = 0.1f;
	state.fontName  = "helveticaBoldItalic";
	state.font      = OpenGLTexFont(state.fontName);
	state.color[0]  = 1.0f;
	state.color[1]  = 1.0f;
	state.color[2]  = 1.0f;
	stateStack.push_back(state);

	try {
		// start parsing
		parseChildren(xml, &MenuReader::parseMenus, (void*)0);

		// clean up
		stateStack.pop_back();
		assert(stateStack.empty());
	}
	catch (...) {
		stateStack.clear();
		throw;
	}
}

void					MenuReader::parseMenus(XMLTree::iterator xml, void*)
{
	// data is unexpected
	if (xml->type != XMLNode::Tag)
		throw XMLIOException(xml->position,
							"unexpected data outside any menu");

	// handle global tags
	if (parseGlobalTags(xml))
		return;

	// must be a menu
	if (xml->value != "menu")
		throw XMLIOException(xml->position, string_util::format(
							"invalid tag `%s'", xml->value.c_str()));

	// get id parameter
	std::string id;
	if (!xml->getAttribute("id", id))
		throw XMLIOException(xml->position, "menu must have an id attribute");

	// create menu
	Menu* menu = new Menu;

	try {
		// get other parameters
		xml->getAttribute("open",  xmlSetMethod(menu, &Menu::setOpenCommand));
		xml->getAttribute("close", xmlSetMethod(menu, &Menu::setCloseCommand));

		// start on new menu
		yPixel        = 0.0f;
		yFraction     = 0.0f;
		yMarkPixel    = 0.0f;
		yMarkFraction = 0.0f;

		// parse the menu
		stateStack.back().menu = menu;
		parseChildren(xml, &MenuReader::parseMenu, (void*)0);
		stateStack.back().menu = NULL;

		// add it
		MENUMGR->insert(id, menu);
	}
	catch (...) {
		delete menu;
		throw;
	}
}

void					MenuReader::parseMenu(
							XMLTree::iterator xml, void*)
{
	// data is unexpected
	if (xml->type != XMLNode::Tag)
		throw XMLIOException(xml->position,
							"unexpected data outside any menu control");

	// check standard tags first
	if (parseStandardTags(xml))
		return;

	// check controls
	MenuControl* control = NULL;
	try {
		if (xml->value == "combo") {
			MenuCombo* combo = new MenuCombo;
			control          = combo;

			// set parameters
			xml->getAttribute("name", xmlSetMethod(combo, &MenuCombo::setName));
			xml->getAttribute("default", xmlSetMethod(combo, &MenuCombo::setDefault));

			// do children
			parseChildren(xml, &MenuReader::parseCombo, combo);
		}

		else if (xml->value == "edit") {
			MenuEdit* edit = new MenuEdit;
			control        = edit;

			// set parameters
			xml->getAttribute("name", xmlSetMethod(edit, &MenuEdit::setName));
			xml->getAttribute("maxlen", xmlStrToInt(xmlCompose(xmlSetMethod(edit,
										&MenuEdit::setMaxLength), xmlMax(1))));
			std::string value;
			if (xml->getAttribute("type", value))
				if (value == "int")
					edit->setNumeric(true);
		}

		else if (xml->value == "button") {
			MenuButton* button = new MenuButton;
			control            = button;

			// set parameters
			xml->getAttribute("action", xmlSetMethod(button, &MenuButton::setAction));
		}

		else if (xml->value == "static") {
			MenuLabel* label = new MenuLabel;
			control          = label;

			// set parameters
			xml->getAttribute("name", xmlSetMethod(label, &MenuLabel::setName));
			xml->getAttribute("texture", xmlSetMethod(label, &MenuLabel::setTexture));
		}

		else if (xml->value == "text") {
			MenuText* text = new MenuText;
			control        = text;

			// set parameters
			xml->getAttribute("lines", xmlStrToInt(xmlCompose(xmlSetMethod(text,
										&MenuText::setLines), xmlMax(1))));
			xml->getAttribute("speed", xmlStrToFloat(xmlSetMethod(text,
										&MenuText::setScrollSpeed)));
			xml->getAttribute("width", menuSetSizeMethod(text, &MenuText::setWidth));

			// collect text data
			parseChildren(xml, &MenuReader::parseText, text);
		}

		else if (xml->value == "list") {
			MenuList* list = new MenuList;
			control        = list;

			// set parameters
			xml->getAttribute("width", menuSetSizeMethod(list, &MenuList::setWidth));
			xml->getAttribute("lines", xmlStrToInt(xmlCompose(xmlSetMethod(list,
										&MenuList::setLines), xmlMax(1))));
			xml->getAttribute("columns", xmlStrToInt(xmlCompose(xmlSetMethod(list,
										&MenuList::setColumns), xmlMax(1))));
			xml->getAttribute("src", xmlSetMethod(list, &MenuList::setSourceName));
			xml->getAttribute("dst", xmlSetMethod(list, &MenuList::setTargetName));
			xml->getAttribute("focus", xmlSetMethod(list, &MenuList::setFocusName));
			xml->getAttribute("select", xmlSetMethod(list, &MenuList::setSelectCommand));
			xml->getAttribute("format", xmlSetMethod(list, &MenuList::setValueFormat));

			// do children
			parseChildren(xml, &MenuReader::parseList, list);
		}

		else if (xml->value == "key") {
			MenuKeyBind* bind = new MenuKeyBind;
			control           = bind;

			// set parameters
			std::string down, up;
			xml->getAttribute("down", down);
			xml->getAttribute("up", up);
			bind->setBindings(down, up);
		}

		else {
			throw XMLIOException(xml->position, string_util::format(
							"invalid tag `%s'", xml->value.c_str()));
		}
		assert(control != NULL);

		// get type info for control
		std::string type, value;
		if (xml->getAttribute("type", value)) {
			type  = ",";
			type += value;
			type += ",";
		}

		// note if control is hidden
		const bool hidden = (strstr(type.c_str(), ",hidden,") != NULL);

		// set control state
		const State& state = stateStack.back();
		xml->getAttribute("label", xmlSetMethod(control, &MenuControl::setLabel));
		control->setHidden(hidden);
		control->setFont(state.font);
		control->setColor(state.color[0], state.color[1], state.color[2]);
		control->setHeight(state.yIsPixels ? state.y : 0.0,
							state.yIsPixels ? 0.0 : state.y);

		// insert into menu
		assert(state.menu != NULL);
		state.menu->append(control, state.align,
							state.xIsPixels ? state.x : 0.0,
							state.xIsPixels ? 0.0 : state.x,
							yPixel, yFraction);

		// if control is not hidden then advance the position
		if (!hidden) {
			float y = control->getHeightMultiplier() * state.y;
			if (state.yIsPixels)
				yPixel += y;
			else
				yFraction += y;
		}
	}
	catch (...) {
		delete control;
		throw;
	}
}

void					MenuReader::parseCombo(
							XMLTree::iterator xml, MenuCombo* combo)
{
	// no data allowed
	if (xml->type != XMLNode::Tag)
		throw XMLIOException(xml->position, "unexpected data in combo");

	// check standard tags
	if (parseStandardTags(xml))
		return;

	// must be an item
	if (xml->value != "item")
		throw XMLIOException(xml->position, string_util::format(
							"invalid tag `%s'", xml->value.c_str()));

	// get parameters and add option to open combo
	std::string label, value;
	xml->getAttribute("label", label);
	if (xml->getAttribute("value", value))
		combo->append(label, value);
	else
		combo->append(label);
}

void					MenuReader::parseList(
							XMLTree::iterator xml, MenuList* list)
{
	// no data allowed
	if (xml->type != XMLNode::Tag)
		throw XMLIOException(xml->position, "unexpected data in list");

	// check standard tags
	if (parseStandardTags(xml))
		return;

	// must be an item
	if (xml->value != "item")
		throw XMLIOException(xml->position, string_util::format(
							"invalid tag `%s'", xml->value.c_str()));

	// get parameters and add option to open list
	std::string label, value;
	xml->getAttribute("label", label);
	xml->getAttribute("value", value);
	list->append(label, value);
}

void					MenuReader::parseText(
							XMLTree::iterator xml, MenuText* text)
{
	// must be data
	if (xml->type != XMLNode::Data)
		throw XMLIOException(xml->position, string_util::format(
							"invalid tag `%s'", xml->value.c_str()));

	// if string so far doesn't end in a newline and
	// isn't empty then append a space.
	std::string& data = text->getText();
	if (!data.empty() && data.c_str()[data.size() - 1] != '\n')
		data += " ";

	// accumulate text
	data += xml->value;
}

bool					MenuReader::parseStandardTags(
							XMLTree::iterator xml)
{
	if (xml->value == "if" || xml->value == "unless") {
		std::string name, value;
		if (!xml->getAttribute("name", name))
			throw XMLIOException(xml->position, string_util::format(
							"%s must have `name' attribute",
							xml->value.c_str()));
		if (!xml->getAttribute("value", value))
			throw XMLIOException(xml->position, string_util::format(
							"%s must have `value' attribute",
							xml->value.c_str()));

		// ignore descendants if expression is true and tag is
		// "unless" or if expression is false and tag is "if".
		if ((BZDB->get(name) == value) != (xml->value == "unless"))
			parseChildren(xml, &MenuReader::parseMenu, (void*)0);
	}

	else if (xml->value == "mark") {
		yMarkPixel    = yPixel;
		yMarkFraction = yFraction;
	}

	else if (xml->value == "jump") {
		yPixel    = yMarkPixel;
		yFraction = yMarkFraction;
	}

	else if (xml->value == "space") {
		std::string value;
		if (xml->getAttribute("height", value)) {
			float num;
			char type;
			if (sscanf(value.c_str(), "%f%c", &num, &type) == 2 &&
					  (type == 'p' || type == '%' || type == 'x')) {
				if (type == 'p')
					yPixel    += num;
				else if (type == '%')
					yFraction += 0.01 * num;
				else if (stateStack.back().yIsPixels)
					yPixel    += num * stateStack.back().y;
				else
					yFraction += num * stateStack.back().y;
			}
			else {
				throw XMLIOException(
							xml->getAttributePosition("height"),
							string_util::format(
								"invalid height `%s'", value.c_str()));
			}
		}
		else {
			yFraction += stateStack.back().y;
		}
	}

	else {
		return parseGlobalTags(xml);
	}

	return true;
}

bool					MenuReader::parseGlobalTags(
							XMLTree::iterator xml)
{
	static const XMLParseEnumList<Menu::Align> s_enumAlign[] = {
		{ "left",   Menu::Left },
		{ "center", Menu::Center },
		{ "right",  Menu::Right },
		{ "detail", Menu::Detail },
		{ NULL, Menu::Left }
	};

	if (xml->value == "align") {
		xml->getAttribute("ref", xmlParseEnum(s_enumAlign,
							xmlSetVar(stateStack.back().align)));
		xml->getAttribute("pos", menuSetSizeMethod(this, &MenuReader::setPos));
	}

	else if (xml->value == "size") {
		xml->getAttribute("height", menuSetSizeMethod(this,
											&MenuReader::setSize));
	}

	else if (xml->value == "font") {
		std::string name;
		if (xml->getAttribute("name", name)) {
			OpenGLTexFont font(name);
			if (!font.isValid())
				throw XMLIOException(xml->getAttributePosition("name"),
							string_util::format(
								"unknown font name `%s'", name.c_str()));
			stateStack.back().font     = font;
			stateStack.back().fontName = name;
		}
	}

	else {
		return false;
	}

	return true;
}

void					MenuReader::setPos(float pixels, float fraction)
{
	stateStack.back().xIsPixels = (pixels != 0.0f);
	stateStack.back().x         = (pixels != 0.0f) ? pixels : fraction;
}

void					MenuReader::setSize(float pixels, float fraction)
{
	stateStack.back().yIsPixels = (pixels != 0.0f);
	stateStack.back().y         = (pixels != 0.0f) ? pixels : fraction;
}

// crs -- this must come after all uses of it to work around a VC++ 6.0 bug
template <class T>
void					MenuReader::parseChildren(
							XMLTree::iterator xml,
							void (MenuReader::*method)(XMLTree::iterator, T*),
							T* data)
{
	try {
		stateStack.push_back(stateStack.back());
		XMLTree::sibling_iterator scan = xml.begin();
		XMLTree::sibling_iterator end  = xml.end();
		for (; scan != end; ++scan)
			(this->*method)(scan, data);
		stateStack.pop_back();
	}
	catch (...) {
		stateStack.pop_back();
		throw;
	}
}
