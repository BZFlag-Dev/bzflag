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

#include "ConfigIO.h"
#include "Menu.h"
#include "OpenGLTexFont.h"

class MenuCombo;
class MenuList;
class MenuText;

//
// object to parse a menu configuration file
//

class MenuReader {
public:
	MenuReader();
	~MenuReader();

	bool				read(istream&);

private:
	bool				open(ConfigReader*, const BzfString&,
							const ConfigReader::Values&);
	bool				close(const BzfString&);
	bool				data(const BzfString&);

	static bool			openCB(ConfigReader*, const BzfString&,
							const ConfigReader::Values&, void*);
	static bool			closeCB(ConfigReader*,
							const BzfString&, void*);
	static bool			dataCB(ConfigReader*,
							const BzfString&, void*);

	bool				parseSize(const BzfString&,
							float& value, bool& inPixels) const;

private:
	struct State {
	public:
		Menu::Align		align;
		bool			xIsPixels, yIsPixels;
		float			x, y;
		BzfString		fontName;
		OpenGLTexFont	font;
		float			color[3];
	};
	typedef std::vector<State> StateStack;

	StateStack			stateStack;
	Menu*				openMenu;
	MenuCombo*			openCombo;
	MenuList*			openList;
	MenuText*			openText;
	BzfString			textString;
	float				yPixel, yFraction;
	float				yMarkPixel, yMarkFraction;
};

#endif
