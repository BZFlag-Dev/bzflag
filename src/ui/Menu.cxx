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

#include "Menu.h"
#include "MenuControls.h"
#include "MenuManager.h"
#include "ErrorHandler.h"
#include "FileManager.h"
#include "SceneReader.h"
#include "SceneNodeMatrixTransform.h"
#include "SceneVisitorSimpleRender.h"
#include "TimeKeeper.h"
#include "bzfgl.h"
#include <sstream>
//
// Menu
//

// default menu cursor
static const char* menuptr =
"<gstate>"
	"<texture filename=\"ybolt\" />"
	"<shading model=\"flat\" />"
	"<blending src=\"src\" dst=\"1-sa\" />"
	"<depth func=\"1\" write=\"off\" />"
	"<geometry>"
		"<color>1 1 1 1</color>"
		"<texcoord>0 0  1 0  0 1  1 1</texcoord>"
		"<vertex>-1 -1 0   1 -1  0  -1  1  0   1  1  0</vertex>"
		"<primitive type=\"tstrip\">"
			"<index>0 1 2 3</index>"
		"</primitive>"
	"</geometry>"
"</gstate>";

unsigned int				Menu::count      = 0;
SceneNodeMatrixTransform*	Menu::projection = NULL;
SceneVisitorSimpleRender*	Menu::renderer   = NULL;
static TimeKeeper			wallClock;

Menu::Menu() : active(0), x(0), y(0)
{
	if (++count == 1) {
		renderer = new SceneVisitorSimpleRender;
		projection = new SceneNodeMatrixTransform;
		projection->type.set(SceneNodeTransform::Projection);

		// FIXME -- fall back to built-in if error reading external model
		std::istream* stream = FILEMGR->createDataInStream("menuptr.bzg");
		if (stream == NULL)
			stream = new std::istringstream(menuptr);
		if (stream != NULL) {
			try {
				// read XML
				XMLTree xmlTree;
				xmlTree.read(*stream, XMLStreamPosition("menuptr.bzg"));

				// parse scene
				SceneReader reader;
				SceneNode* node = reader.parse(xmlTree.begin());
				if (node != NULL) {
					projection->pushChild(node);
					node->unref();
				}
			}
			catch (XMLIOException& e) {
				printError("%s (%d,%d): %s",
								e.position.filename.c_str(),
								e.position.line,
								e.position.column,
								e.what());
			}
			delete stream;
		}

		wallClock = TimeKeeper::getCurrent();
	}
}

Menu::~Menu()
{
	// clean up
	for (Controls::iterator index = controls.begin();
								index != controls.end(); ++index)
		delete index->control;

	if (--count == 0) {
		if (projection != NULL) {
			projection->unref();
			projection = NULL;
		}
		delete renderer;
		renderer = NULL;
	}
}

void					Menu::append(MenuControl* control, Align alignment,
								float xPixel, float xFraction,
								float yPixel, float yFraction)
{
	assert(control != NULL);

	Control item;
	item.control   = control;
	item.align     = alignment;
	item.xPixel    = xPixel;
	item.yPixel    = yPixel;
	item.xFraction = xFraction;
	item.yFraction = yFraction;
	item.x         = 0.0;
	item.y         = 0.0;
	controls.push_back(item);
}

void					Menu::open()
{
	active = -1;
	for (Controls::iterator index = controls.begin();
								index != controls.end(); ++index) {
		index->control->init();
		if (active == -1 && index->control->isActive())
			active = index - controls.begin();
	}
}

void					Menu::close()
{
	// do nothing
}

void					Menu::setOpenCommand(const std::string& cmd)
{
	openCommand = cmd;
}

void					Menu::setCloseCommand(const std::string& cmd)
{
	closeCommand = cmd;
}

std::string				Menu::getOpenCommand() const
{
	return openCommand;
}

std::string				Menu::getCloseCommand() const
{
	return closeCommand;
}

int						Menu::getX() const
{
	return x;
}

int						Menu::getY() const
{
	return y;
}

void					Menu::reshape(
								int _x, int _y,
								int iwidth, int iheight)
{
	x = _x;
	y = _y;
	w = iwidth;
	h = iheight;

	float wWindow = iwidth;
	float hWindow = iheight;

	for (Controls::iterator index = controls.begin();
								index != controls.end(); ++index) {
		Control& item = *index;
		item.control->calcSize(wWindow, hWindow);
		item.x = item.xPixel + item.xFraction * wWindow;
		item.y = hWindow - 1 - (item.yPixel + item.yFraction * hWindow);
		switch (item.align) {
			case Left:
				break;

			case Center:
				item.x -= 0.5 * item.control->getWidth();
				break;

			case Right:
				item.x -= item.control->getWidth();
				break;

			case Detail:
				item.x -= item.control->getDetailX();
				break;
		}
	}
}

void					Menu::render()
{
	for (Controls::iterator index = controls.begin();
								index != controls.end(); ++index) {
		Control& item = *index;
		if (!item.control->isHidden()) {
			const bool isFocus = (index - controls.begin() == active);
			item.control->render(item.x, item.y, isFocus);
			if (isFocus && !item.control->isFocusHidden())
				renderFocus(item);
		}
	}
}

void					Menu::renderFocus(const Control& control)
{
	float h  = control.control->getFont().getAscent();
	float fx = control.x + control.control->getDetailX();
	float fy = control.y + control.control->getDetailY() - h;

	// set the projection
	static const float n = -1.0f;
	static const float f =  1.0f;
	float m[16];
	m[0]  =  1.0f;
	m[1]  =  0.0f;
	m[2]  =  0.0f;
	m[3]  =  0.0f;
	m[4]  =  0.0f;
	m[5]  =  1.0f;
	m[6]  =  0.0f;
	m[7]  =  0.0f;
	m[8]  =  0.0f;
	m[9]  =  0.0f;
	m[10] = -2.0f / (f - n);
	m[11] =  0.0f;
	m[12] =  0.0f;
	m[13] =  0.0f;
	m[14] = -(f + n) / (f - n);
	m[15] =  1.0f;
	projection->matrix.set(m, 16);

	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(static_cast<int>(x + fx), static_cast<int>(y + fy),
								static_cast<int>(h), static_cast<int>(h));
	renderer->setArea(h * h);
	renderer->getParams().pushFloat("time", wallClock - TimeKeeper::getCurrent());
	renderer->traverse(projection);
	renderer->getParams().popFloat("time");
	glPopAttrib();
}

void					Menu::prev()
{
	const int n = controls.size();
	int index;
	for (index = active - 1; index >= 0; --index)
		if (controls[index].control->isActive()) {
			active = index;
			return;
		}
	for (index = n - 1; index > active; --index)
		if (controls[index].control->isActive()) {
			active = index;
			return;
		}
}

void					Menu::next()
{
	const int n = controls.size();
	int index;
	for (index = active + 1; index < n; ++index)
		if (controls[index].control->isActive()) {
			active = index;
			return;
		}
	for (index = 0; index < active; ++index)
		if (controls[index].control->isActive()) {
			active = index;
			return;
		}
}

MenuControl*			Menu::getFocus() const
{
	if (active == -1)
		return NULL;
	else
		return controls[active].control;
}

void					Menu::keyPress(const BzfKeyEvent& key)
{
	if (key.ascii == 27) {
		MENUMGR->pop();
		return;
	}

	if (active != -1)
		if (controls[active].control->onKeyPress(key))
			return;

	if (key.ascii == 0) {
		switch (key.button) {
			case BzfKeyEvent::Up:
				prev();
				return;

			case BzfKeyEvent::Down:
				next();
				return;
		}
	}
	else if (key.ascii == 8) {
		if (key.shift & BzfKeyEvent::ShiftKey)
			prev();
		else
			next();
		return;
	}
}

void					Menu::keyRelease(const BzfKeyEvent& key)
{
	if (key.ascii == 27)
		return;

	if (active != -1)
		if (controls[active].control->onKeyRelease(key))
			return;

/*
	if (key.ascii == 0) {
		switch (key.button) {
			case BzfKeyEvent::Up:
			case BzfKeyEvent::Down:
				return;
		}
	}
	if (key.ascii == 8)
		return;
*/
}
