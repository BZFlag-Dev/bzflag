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

#include "ViewItemPWR.h"
#include "HUDManager.h"
#include "LocalPlayer.h"
#include "StateDatabase.h"
#include "World.h"
#include "bzfgl.h"
#include <assert.h>

ViewItemPWR::ViewItemPWR(PWRType _type) :
							type(_type)
{
}

ViewItemPWR::~ViewItemPWR()
{
}

bool			ViewItemPWR::onPreRender(float, float, float, float)
{
	LocalPlayer* myTank = LocalPlayer::getMyTank();
	return (myTank != NULL && !myTank->isPaused() && myTank->isAlive());
}

#define EPSILON 0.0001

void			ViewItemPWR::onPostRender(float, float, float w, float h)
{
	OpenGLGState::resetState();

	if (type == Shots)
		drawShots(w, h);
	else if (type == BadFlags)
		drawEvilFlags(w, h);
}

void			ViewItemPWR::drawShots(float w, float h)
{
	if (BZDB->isTrue("renderBlending"))
		glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	LocalPlayer* tank = LocalPlayer::getMyTank();

	glBegin(GL_QUADS);

	// draw remaining shots and fill status of recharging shots.
	// if no shots remain, give big bar with the time remaining until
	// at least one shot is ready.  one shot could mean the game

	if (tank->getReloadTime() > EPSILON) {
		glColor4f(1.0, 0.0, 0.0, 0.5);
		drawPB(0, 0, int(w), int(h * tank->getReloadTime() / 5.0)); // fix this constant
	}
	else {
		int maxshots = World::getWorld()->getMaxShots();
		for (int i = 0; i < maxshots; ++i) {
			ShotPath* shot = tank->getShot(i);
			if (shot) {
				glColor4f(1.0, 0.0, 0.0, 0.5);
				float c = shot->getCurrentTime() - shot->getStartTime();
				drawPB(0, i * 10, (int)(w * c/shot->getReloadTime()), 7);
			}
			else {
				glColor3fv(Team::getRadarColor(tank->getTeam()));
				drawPB(0, i * 10, int(w), 7);
			}
		}
	}

	glEnd();
	if (BZDB->isTrue("renderBlending"))
		glDisable(GL_BLEND);
}

void			ViewItemPWR::drawEvilFlags(float w, float h)
{
	if (BZDB->isTrue("renderBlending"))
	glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// draw time
	LocalPlayer* tank = LocalPlayer::getMyTank();
	World* world = World::getWorld();

	glBegin(GL_QUADS);

	if(tank->getFlagShakingTime() > EPSILON) {
		glColor4f(0.0, 0.0, 0.0, 0.5);
		glVertex2i(0,       0);
		glVertex2i((int) w, 0);

		glColor4f(1.0, 1.0, 0.0, 0.5);

		double fr = tank->getFlagShakingTime() / world->getFlagShakeTimeout();
		glVertex2i((int) w, (int) (h * fr));
		glVertex2i(0,       (int) (h * fr));
	}

	glEnd();
	if (BZDB->isTrue("renderBlending"))
		glDisable(GL_BLEND);
}

void ViewItemPWR::drawPB(int x, int y, int w, int h)
{
	glVertex2i(x,     y);
	glVertex2i(x + w, y);
	glVertex2i(x + w, y + h);
	glVertex2i(x,     y + h);
}

ViewItemPWRReader::ViewItemPWRReader() : item(NULL)
{
}

ViewItemPWRReader::~ViewItemPWRReader()
{
	if (item != NULL)
		item->unref();
}

ViewTagReader*	ViewItemPWRReader::clone() const
{
	return new ViewItemPWRReader;
}

View*			ViewItemPWRReader::open(XMLTree::iterator xml)
{
	static const XMLParseEnumList<PWRType> s_enumPWRType[] = {
		{"shots", Shots},
		{"badflags", BadFlags}
	};

	PWRType type;

	// parse
	xml->getAttribute("type", xmlParseEnum(s_enumPWRType,xmlSetVar(type)));

	// create item
	assert(item == NULL);
	item = new ViewItemPWR(type);
	return item;
}
// ex: shiftwidth=4 tabstop=4
