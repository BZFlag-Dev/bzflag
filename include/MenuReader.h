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

#ifndef BZF_MENU_READER_H
#define BZF_MENU_READER_H

#include "Menu.h"
#include "OpenGLTexFont.h"
#include "XMLTree.h"
#include "ConfigFileReader.h"

class MenuCombo;
class MenuList;
class MenuText;

//
// object to parse a menu configuration file
//

class MenuReader : public ConfigFileReader {
public:
	MenuReader();
	~MenuReader();

	// ConfigFileReader overrides.  menus go to MENUMGR.
	ConfigFileReader*	clone();
	void				parse(XMLTree::iterator);

private:
	// crs -- must have arg names here to work around bug in VC++ 6.0
	template <class T>
	void				parseChildren(XMLTree::iterator xml,
							void (MenuReader::*method)(XMLTree::iterator, T*),
							T* data);

	void				parseMenus(XMLTree::iterator, void*);
	void				parseMenu(XMLTree::iterator, void*);
	void				parseCombo(XMLTree::iterator, MenuCombo*);
	void				parseList(XMLTree::iterator, MenuList*);
	void				parseText(XMLTree::iterator, MenuText*);

	bool				parseConditionalTags(XMLTree::iterator);
	bool				parseStandardTags(XMLTree::iterator);
	bool				parseGlobalTags(XMLTree::iterator);

	void				setPos(float pixels, float fraction);
	void				setSize(float pixels, float fraction);

private:
	struct State {
	public:
		Menu*			menu;
		Menu::Align		align;
		bool			xIsPixels, yIsPixels;
		float			x, y;
		std::string		fontName;
		OpenGLTexFont	font;
		float			color[3];
	};
	typedef std::vector<State> StateStack;

	StateStack			stateStack;
	float				yPixel, yFraction;
	float				yMarkPixel, yMarkFraction;
};

#endif
