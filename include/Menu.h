/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_MENU_H
#define BZF_MENU_H

#include "common.h"
#include "BzfString.h"
#include "BzfEvent.h"
#include <vector>

class MenuControl;
class SceneNodeMatrixTransform;
class SceneVisitorSimpleRender;

class Menu {
public:
	enum Align { Left, Center, Right, Detail };

	Menu();
	~Menu();

	void				append(MenuControl*, Align alignment,
							float xPixel, float xFraction,
							float yPixel, float yFraction);

	// open/close the menu.  normally called by the menu manager.
	void				open();
	void				close();

	// get/set the actions to perform when the menu is opened/closed
	void				setOpenCommand(const BzfString&);
	void				setCloseCommand(const BzfString&);
	BzfString			getOpenCommand() const;
	BzfString			getCloseCommand() const;

	// change/get active control
	void				prev();
	void				next();
	MenuControl*		getFocus() const;

	// set the size and position of the menu.  these should be in
	// window coordinates.
	void				reshape(int x, int y, int width, int height);

	// draw the menu
	void				render();

	// handle events
	void				keyPress(const BzfKeyEvent&);
	void				keyRelease(const BzfKeyEvent&);

	// get the most recently set position
	int					getX() const;
	int					getY() const;

private:
	struct Control {
	public:
		MenuControl*	control;
		Align			align;
		float			xPixel, yPixel;
		float			xFraction, yFraction;
		float			x, y;
	};
	typedef std::vector<Control> Controls;

	void				renderFocus(const Control&);

private:
	Controls			controls;
	int					active;
	int					x, y;
	int					w, h;
	BzfString			openCommand;
	BzfString			closeCommand;

	static unsigned int	count;
	static SceneNodeMatrixTransform*	projection;
	static SceneVisitorSimpleRender*	renderer;
};

#endif
