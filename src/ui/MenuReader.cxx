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

#ifdef macintosh
#include "mac_funcs.h"
#endif

#include "MenuReader.h"
#include "StateDatabase.h"
#include "MenuControls.h"
#include "MenuManager.h"
#include "ErrorHandler.h"
#include <stdio.h>
#include <ctype.h>

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

bool					MenuReader::read(istream& s)
{
	// set initial state
	State state;
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
	openMenu        = NULL;
	openCombo       = NULL;
	openList        = NULL;
	openText        = NULL;
	stateStack.push_back(state);

	// prep config reader
	ConfigReader reader;
	reader.push(&openCB, &closeCB, NULL);

	// read stream
	return reader.read(s, this);
}

bool					MenuReader::open(ConfigReader* reader,
								const BzfString& tag,
								const ConfigReader::Values& values)
{
	if (tag == "menu") {
		// check validity
		if (openMenu != NULL || openCombo != NULL || openList != NULL) {
			printError("%s: nesting not allowed",
								reader->getPosition().c_str());
			return false;
		}

		// get id parameter
		ConfigReader::Values::const_iterator index;
		index = values.find("id");
		if (index == values.end()) {
			printError("%s: menu must have an id", reader->getPosition().c_str());
			return false;
		}
		BzfString id = index->second;

		// create menu
		openMenu = new Menu;

		// get other parameters
		index = values.find("open");
		if (index != values.end())
			openMenu->setOpenCommand(index->second);
		index = values.find("close");
		if (index != values.end())
			openMenu->setCloseCommand(index->second);

		// start on new menu
		MENUMGR->insert(id, openMenu);
		yPixel    = 0.0f;
		yFraction = 0.0f;
		stateStack.push_back(stateStack.back());
	}

	else if (tag == "mark") {
		yMarkPixel    = yPixel;
		yMarkFraction = yFraction;
	}

	else if (tag == "jump") {
		yPixel    = yMarkPixel;
		yFraction = yMarkFraction;
	}

	else if (tag == "align") {
		ConfigReader::Values::const_iterator index;
		index = values.find("ref");
		if (index != values.end()) {
			const BzfString value = index->second;
			if (value == "left")
				stateStack.back().align = Menu::Left;
			else if (value == "center")
				stateStack.back().align = Menu::Center;
			else if (value == "right")
				stateStack.back().align = Menu::Right;
			else if (value == "detail")
				stateStack.back().align = Menu::Detail;
			else
				printError("%s: ignoring invalid alignment ref: %s",
								reader->getPosition().c_str(),
								value.c_str());
		}

		index = values.find("pos");
		if (index != values.end()) {
			float value;
			bool inPixels;
			if (parseSize(index->second, value, inPixels) && value >= 0.0f) {
				stateStack.back().xIsPixels = inPixels;
				stateStack.back().x         = value;
			}
			else {
				printError("%s: ignoring invalid alignment pos: %s",
								reader->getPosition().c_str(),
								index->second.c_str());
			}
		}
	}

	else if (tag == "size") {
		ConfigReader::Values::const_iterator index;
		index = values.find("height");
		if (index != values.end()) {
			float value;
			bool inPixels;
			if (parseSize(index->second, value, inPixels) && value >= 0.0) {
				stateStack.back().yIsPixels = inPixels;
				stateStack.back().y         = value;
			}
			else {
				printError("%s: ignoring invalid size: %s",
								reader->getPosition().c_str(),
								index->second.c_str());
			}
		}
	}

	else if (tag == "font") {
		OpenGLTexFont font(stateStack.back().font);
		BzfString name(stateStack.back().fontName);

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

		// save new state
		stateStack.back().font      = font;
		stateStack.back().fontName  = name;
	}

	else if (tag == "space") {
		ConfigReader::Values::const_iterator index;
		index = values.find("height");
		if (index != values.end()) {
			float value;
			char type;
			if (sscanf(index->second.c_str(), "%f%c", &value, &type) == 2 &&
		  (type == 'p' || type == '%' || type == 'x')) {
				if (type == 'p') {
					yPixel += value;
				}
				else if (type == '%') {
					yFraction += 0.01 * value;
				}
				else if (stateStack.back().yIsPixels) {
					yPixel += value * stateStack.back().y;
				}
				else {
					yFraction += value * stateStack.back().y;
				}
			}
			else {
				printError("%s: ignoring invalid height: %s",
								reader->getPosition().c_str(),
								index->second.c_str());
			}
		}
		else {
			yFraction += stateStack.back().y;
		}
	}

	else if (tag == "if" || tag == "unless") {
		ConfigReader::Values::const_iterator index1 = values.find("name");
		ConfigReader::Values::const_iterator index2 = values.find("value");
		if (index1 == values.end() || index2 == values.end()) {
			printError("%s: %s must have \"%s\" parameter",
								reader->getPosition().c_str(),
								tag.c_str(),
								(index1 == values.end()) ? "name" : "value");
			return false;
		}

		// ignore all tags if expression is true and tag is "unless" or if
		// expression is false and tag is "if" until we reach the closing
		// tag.
		const BzfString name  = index1->second;
		const BzfString value = index2->second;
		if ((BZDB->get(name) == value) == (tag == "unless"))
			reader->push(NULL, NULL, NULL);
	}

	else {
		if (openMenu == NULL) {
			printError("%s: menu control must be in a menu",
								reader->getPosition().c_str());
			return false;
		}

		if (tag == "item") {
			if (openCombo == NULL && openList == NULL) {
				printError("%s: option must be in a combo or list",
								reader->getPosition().c_str());
				return false;
			}

			// get parameters and add option to open list
			ConfigReader::Values::const_iterator index1 = values.find("label");
			ConfigReader::Values::const_iterator index2 = values.find("value");
			if (openCombo != NULL) {
				if (index2 != values.end())
					openCombo->append(index1->second, index2->second);
				else
					openCombo->append(index1->second);
			}
			else {
				if (index2 != values.end())
					openList->append(index1->second, index2->second);
			}
		}

		else {
			if (openCombo != NULL || openList != NULL || openText != NULL) {
				printError("%s: control nesting not allowed",
								reader->getPosition().c_str());
				return false;
			}

			ConfigReader::Values::const_iterator index;
			MenuControl* control;
			if (tag == "combo") {
				MenuCombo* combo = new MenuCombo;
				control          = combo;

				// set parameters
				index = values.find("name");
				if (index != values.end())
					combo->setName(index->second);
				index = values.find("default");
				if (index != values.end())
					combo->setDefault(index->second);

				// start on new combo
				openCombo = combo;
				stateStack.push_back(stateStack.back());
			}

			else if (tag == "edit") {
				MenuEdit* edit = new MenuEdit;
				control        = edit;

				// set parameters
				index = values.find("name");
				if (index != values.end())
					edit->setName(index->second);
				index = values.find("maxlen");
				if (index != values.end())
					edit->setMaxLength(atoi(index->second.c_str()));
				index = values.find("type");
				if (index != values.end())
					if (index->second == "int")
						edit->setNumeric(true);
			}

			else if (tag == "button") {
				MenuButton* button = new MenuButton;
				control            = button;

				// set parameters
				index = values.find("action");
				if (index != values.end())
					button->setAction(index->second);
			}

			else if (tag == "static") {
				MenuLabel* label = new MenuLabel;
				control          = label;

				// set parameters
				index = values.find("name");
				if (index != values.end())
					label->setName(index->second);
				index = values.find("texture");
				if (index != values.end())
					label->setTexture(index->second);
			}

			else if (tag == "text") {
				MenuText* text = new MenuText;
				control        = text;

				// set parameters
				index = values.find("lines");
				if (index != values.end()) {
					int lines = atoi(index->second.c_str());
					if (lines < 1)
						lines = 1;
					text->setLines(lines);
				}
				index = values.find("width");
				if (index != values.end()) {
					float value;
					bool inPixels;
					if (parseSize(index->second, value, inPixels) && value >= 0.0f) {
						text->setWidth(inPixels ? value : 0.0f, inPixels ? 0.0f : value);
					}
					else {
						printError("%s: ignoring invalid text width: %s",
								reader->getPosition().c_str(),
								index->second.c_str());
					}
				}
				index = values.find("speed");
				if (index != values.end()) {
					text->setScrollSpeed(atof(index->second.c_str()));
				}

				textString = "";
				openText = text;
				reader->push(&openCB, &closeCB, &dataCB);
				stateStack.push_back(stateStack.back());
			}

			else if (tag == "list") {
				MenuList* list = new MenuList;
				control        = list;

				// set parameters
				index = values.find("lines");
				if (index != values.end()) {
					int lines = atoi(index->second.c_str());
					if (lines < 1)
						lines = 1;
					list->setLines(lines);
				}
				index = values.find("columns");
				if (index != values.end()) {
					int columns = atoi(index->second.c_str());
					if (columns < 1)
						columns = 1;
					list->setColumns(columns);
				}
				index = values.find("width");
				if (index != values.end()) {
					float value;
					bool inPixels;
					if (parseSize(index->second, value, inPixels) && value >= 0.0f) {
						list->setWidth(inPixels ? value : 0.0f, inPixels ? 0.0f : value);
					}
					else {
						printError("%s: ignoring invalid text width: %s",
								reader->getPosition().c_str(),
								index->second.c_str());
					}
				}
				index = values.find("src");
				if (index != values.end())
					list->setSourceName(index->second);
				index = values.find("dst");
				if (index != values.end())
					list->setTargetName(index->second);
				index = values.find("focus");
				if (index != values.end())
					list->setFocusName(index->second);
				index = values.find("select");
				if (index != values.end())
					list->setSelectCommand(index->second);
				index = values.find("format");
				if (index != values.end())
					list->setValueFormat(index->second);

				// start on new list
				openList = list;
				stateStack.push_back(stateStack.back());
			}

			else if (tag == "key") {
				MenuKeyBind* bind = new MenuKeyBind;
				control           = bind;

				// set parameters
				BzfString down, up;
				index = values.find("down");
				if (index != values.end())
					down = index->second;
				index = values.find("up");
				if (index != values.end())
					up = index->second;
				bind->setBindings(down, up);
			}

			else {
				control = NULL;
				printError("%s: ignoring unknown control: %s",
								reader->getPosition().c_str(),
								tag.c_str());
			}

			// do stuff common to all controls
			if (control != NULL) {
				const State& state = stateStack.back();

				// get type info for control
				BzfString type;
				index = values.find("type");
				if (index != values.end()) {
					type = ",";
					type += index->second;
					type += ",";
				}

				// note if control is hidden
				const bool hidden = (strstr(type.c_str(), ",hidden,") != NULL);

				// set control state
				index = values.find("label");
				if (index != values.end())
					control->setLabel(index->second);
				control->setHidden(hidden);
				control->setFont(state.font);
				control->setColor(state.color[0], state.color[1], state.color[2]);
				control->setHeight(state.yIsPixels ? state.y : 0.0,
								state.yIsPixels ? 0.0 : state.y);

				// insert into openMenu
				openMenu->append(control, state.align,
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
		}
	}

	return true;
}

bool					MenuReader::close(const BzfString& tag)
{
	if (tag == "menu") {
		openMenu = NULL;
		stateStack.pop_back();
	}

	else if (tag == "combo") {
		openCombo = NULL;
		stateStack.pop_back();
	}

	else if (tag == "list") {
		openList = NULL;
		stateStack.pop_back();
	}

	else if (tag == "text") {
		openText->setText(textString);
		openText = NULL;
		stateStack.pop_back();
	}

	return true;
}

bool					MenuReader::data(const BzfString& data)
{
	// skip leading whitespace
	const char* scan = data.c_str();
	while (*scan != '\0' && isspace(*scan) && *scan != '\n')
		++scan;
	if (*scan == '\0')
		return true;

	// if string so far doesn't end in a newline and isn't empty then
	// append a space.
	if (!textString.empty() && textString.c_str()[textString.size() - 1] != '\n')
		textString += " ";

	// append new data
	textString += scan;

	return true;
}

bool					MenuReader::openCB(ConfigReader* reader,
								const BzfString& tag,
								const ConfigReader::Values& values,
								void* self)
{
	return reinterpret_cast<MenuReader*>(self)->open(reader, tag, values);
}

bool					MenuReader::closeCB(ConfigReader*,
								const BzfString& tag,
								void* self)
{
	return reinterpret_cast<MenuReader*>(self)->close(tag);
}

bool					MenuReader::dataCB(ConfigReader*,
								const BzfString& data,
								void* self)
{
	return reinterpret_cast<MenuReader*>(self)->data(data);
}

bool					MenuReader::parseSize(const BzfString& s,
								float& value, bool& inPixels) const
{
	char type;
	if (sscanf(s.c_str(), "%f%c", &value, &type) != 2)
		return false;
	if (type != 'p' && type != '%')
		return false;

	inPixels = (type == 'p');
	if (!inPixels)
		value = 0.01f * value;

	return true;
}
