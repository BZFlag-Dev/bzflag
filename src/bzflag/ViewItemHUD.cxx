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

#include "ViewItemHUD.h"
#include "HUDManager.h"
#include "LocalPlayer.h"
#include "World.h"
#include "bzfgl.h"
#include <assert.h>

//
// ViewItemHUD
//

ViewItemHUD::ViewItemHUD(bool _primary) : primary(_primary)
{
	// do nothing
}

ViewItemHUD::~ViewItemHUD()
{
	// do nothing
}

bool					ViewItemHUD::onPreRender(
								float x0, float y0, float w, float h)
{
	// compute size of no-motion and max-motion boxes
	{
		const float xScale = w / 1280.0f;
		const float yScale = h / 1024.0f;
		const float  scale = (xScale < yScale) ? xScale : yScale;
		noMotionSize       = 10;
		maxMotionSize      = static_cast<int>(200.0f * scale);
	}

	// compute center of view
	x = static_cast<int>(0.5f * w);
	y = static_cast<int>(0.5f * h);

	// save box shapes
	if (primary) {
		HUDMGR->setCenter(static_cast<int>(x0) + x, static_cast<int>(y0) + y);
		HUDMGR->setNoMotionSize(noMotionSize, noMotionSize);
		HUDMGR->setMaxMotionSize(maxMotionSize, maxMotionSize);
	}

	// now decide if we should draw the HUD.  we set the motion boxes
	// even if we don't draw the HUD because it's possible that other
	// things need that information.
	LocalPlayer* myTank = LocalPlayer::getMyTank();
	return (myTank != NULL && !myTank->isPaused() && myTank->isAlive());
}

void					ViewItemHUD::onPostRender(
								float x0, float y0, float, float)
{
	OpenGLGState::resetState();

	// draw targeting box
	glColor3fv(HUDMGR->getColor());
	glBegin(GL_LINE_LOOP);
		glVertex2i(x - noMotionSize, y - noMotionSize);
		glVertex2i(x + noMotionSize, y - noMotionSize);
		glVertex2i(x + noMotionSize, y + noMotionSize);
		glVertex2i(x - noMotionSize, y + noMotionSize);
	glEnd();
	glBegin(GL_LINE_LOOP);
		glVertex2i(x - maxMotionSize, y - maxMotionSize);
		glVertex2i(x + maxMotionSize, y - maxMotionSize);
		glVertex2i(x + maxMotionSize, y + maxMotionSize);
		glVertex2i(x - maxMotionSize, y + maxMotionSize);
	glEnd();

	glPushAttrib(GL_SCISSOR_BIT);

	// draw heading strip
	if (true /* always draw heading strip */)
		drawHeading(static_cast<int>(x0), static_cast<int>(y0),
								x, y + maxMotionSize, maxMotionSize);

	// draw altitude strip
	if (LocalPlayer::getMyTank()->getFlag() == JumpingFlag ||
		World::getWorld()->allowJumping())
		drawAltitude(static_cast<int>(x0), static_cast<int>(y0),
								x + maxMotionSize, y, maxMotionSize);

	glPopAttrib();
}

void					ViewItemHUD::drawHeading(int x0, int y0,
								int x, int y, int w)
{
	int i;

	// tick mark parameters
	static const float range = 25.0f;
	static const int minorTickDiff = 5;
	static const int minorTicksPerMajor = 2;

	// derived constant tick mark parameters
	static const int majorTickDiff = minorTicksPerMajor * minorTickDiff;
	static const int max           = 360 / majorTickDiff;

	LocalPlayer* myTank = LocalPlayer::getMyTank();
	float heading = fmodf(810.0f - 180.0f * myTank->getAngle() / M_PI, 360.0f);

	// clip
	glScissor(x0 + x - w, y0 + y - 5,
				2 * w, 15 + (int)getState().font.getSpacing());

	// draw heading mark
	glBegin(GL_LINES);
		glVertex2i(x, y);
		glVertex2i(x, y - 5);
	glEnd();

	// derived tick mark parameters
	float minorTickSpacing = floorf(w / (range / minorTickDiff));
	float majorTickSpacing = minorTicksPerMajor * minorTickSpacing;
	float centerTick = (heading / majorTickDiff);
	float closestTick = -floorf(majorTickSpacing *
								(centerTick - floorf(centerTick)));
	int numMajorTicksPerSide = static_cast<int>(0.5f * w / majorTickSpacing) + 2;
	int base = max + static_cast<int>(centerTick);

	// draw tick marks
	float tx = x + closestTick - (numMajorTicksPerSide - 1) * majorTickSpacing;
	glBegin(GL_LINES);
	for (i = -numMajorTicksPerSide + 1; i <= numMajorTicksPerSide + 1; i++) {
		glVertex2f(tx, y + 0.0f);
		glVertex2f(tx, y + 8.0f);
		tx += minorTickSpacing;
		for (int j = 1; j < minorTicksPerMajor; ++j) {
			glVertex2f(tx, y + 0.0f);
			glVertex2f(tx, y + 4.0f);
			tx += minorTickSpacing;
		}
	}
	glEnd();

	// draw labels
	tx = x + closestTick - (numMajorTicksPerSide - 1) * majorTickSpacing;
	float ty = y + 8.0f + floorf(getState().font.getDescent());
	for (i = -numMajorTicksPerSide + 1; i <= numMajorTicksPerSide + 1; i++) {
		BzfString label = BzfString::format("%d",
								majorTickDiff * ((i + base) % max));
		float tw = getState().font.getWidth(label);
		getState().font.draw(label, tx - 0.5f * tw, ty);
		tx += majorTickSpacing;
	}
	OpenGLGState::resetState();

	// draw markers (give 'em a little more space on the sides)
	MarkerCBInfo2 cbInfo;
	cbInfo.self                  = this;
	cbInfo.info.zero             = -majorTickSpacing * centerTick;
	cbInfo.info.w                = w;
	cbInfo.info.centerHeading    = heading;
	cbInfo.info.majorTickDiff    = majorTickDiff;
	cbInfo.info.majorTickSpacing = majorTickSpacing;
	glScissor(x0 + x - w - 8, y0 + y, 2 * w + 16, 10);
	glPushMatrix();
	glTranslatef((float)x, (float)y, 0.0f);
	HUDMGR->iterate(&drawMarkerCB, &cbInfo);
	glPopMatrix();

	// restore color
	glColor3fv(HUDMGR->getColor());
}

void					ViewItemHUD::drawAltitude(int x0, int y0,
								int x, int y, int h)
{
	int i;

	// tick mark parameters
	static const float range = 20.0f;
	static const float minorTickDiff = 5.0f;
	static const int minorTicksPerMajor = 2;

	LocalPlayer* myTank = LocalPlayer::getMyTank();
	float altitude = myTank->getPosition()[2];

	// clip
	int maxLabelWidth = static_cast<int>(getState().font.getWidth("XXX"));
	glScissor(x0 + x - 5, y0 + y - h, maxLabelWidth + 15, 2 * h);

	// draw altitude mark
	glBegin(GL_LINES);
		glVertex2i(x, y);
		glVertex2i(x - 5, y);
	glEnd();

	// derived tick mark parameters
	float majorTickDiff = minorTicksPerMajor * minorTickDiff;
	float minorTickSpacing = floorf(h / (range / minorTickDiff));
	float majorTickSpacing = minorTicksPerMajor * minorTickSpacing;
	float centerTick = (altitude / majorTickDiff);
	float closestTick = -floorf(majorTickSpacing *
								(centerTick - floorf(centerTick)));
	int numMajorTicksPerSide = static_cast<int>(0.5f * h / majorTickSpacing) + 2;
	int mult = static_cast<int>(majorTickDiff);
	int base = static_cast<int>(centerTick);

	// draw tick marks
	float ty = y + closestTick - (numMajorTicksPerSide - 1) * majorTickSpacing;
	glBegin(GL_LINES);
	for (i = -numMajorTicksPerSide + 1; i <= numMajorTicksPerSide + 1; i++) {
		if (i + base >= 0) {
			glVertex2f(x + 0.0f, ty);
			glVertex2f(x + 8.0f, ty);
			ty += minorTickSpacing;
			for (int j = 1; j < minorTicksPerMajor; ++j) {
				glVertex2f(x + 0.0f, ty);
				glVertex2f(x + 4.0f, ty);
				ty += minorTickSpacing;
			}
		}
		else {
			ty += majorTickSpacing;
		}
	}
	glEnd();

	// draw labels
	float tx = x + 10.0f;
	ty = y + closestTick - (numMajorTicksPerSide - 1) * majorTickSpacing +
  								floorf(getState().font.getBaselineFromCenter());
	for (i = -numMajorTicksPerSide + 1; i <= numMajorTicksPerSide + 1; i++) {
		if (i + base >= 0) {
			BzfString label = BzfString::format("%d", mult * (i + base));
			getState().font.draw(label, tx, ty);
		}
		ty += majorTickSpacing;
	}
	OpenGLGState::resetState();
}

void					ViewItemHUD::drawMarker(const MarkerCBInfo* info,
								float markerHeading, const float* color)
{
	// marker heading should be within +/-180 of center heading
	if (markerHeading - info->centerHeading > 180.0f)
		markerHeading -= 360.0f;
	else if (markerHeading - info->centerHeading < -180.0f)
		markerHeading += 360.0f;

	// compute position on marker
	float ticks = markerHeading / info->majorTickDiff;
	float x = info->majorTickSpacing * ticks + info->zero;

	// draw marker
	glColor3fv(color);
	if (x > -info->w && x < info->w) {
		// inside tape
		glBegin(GL_QUADS);
			glVertex2f(x, 0.0f);
			glVertex2f(x + 4.0f, 4.0f);
			glVertex2f(x, 8.0f);
			glVertex2f(x - 4.0f, 4.0f);
		glEnd();
	}
	else if (x <= -info->w) {
		// off to the left
		glBegin(GL_TRIANGLES);
			glVertex2f(-info->w, 8.0f);
			glVertex2f(-info->w - 4.0f, 4.0f);
			glVertex2f(-info->w, 0.0f);
		glEnd();
	}
	else {
		// off to the right
		glBegin(GL_TRIANGLES);
			glVertex2f(info->w, 0.0f);
			glVertex2f(info->w + 4.0f, 4.0f);
			glVertex2f(info->w, 8.0f);
		glEnd();
	}
}

void					ViewItemHUD::drawMarkerCB(const BzfString&,
								float heading, const float* color,
								void* userData)
{
	MarkerCBInfo2* info = reinterpret_cast<MarkerCBInfo2*>(userData);
	info->self->drawMarker(&info->info, heading, color);
}


//
// ViewItemHUDReader
//

ViewItemHUDReader::ViewItemHUDReader() : item(NULL)
{
	// do nothing
}

ViewItemHUDReader::~ViewItemHUDReader()
{
	if (item != NULL)
		item->unref();
}

ViewTagReader* 			ViewItemHUDReader::clone() const
{
	return new ViewItemHUDReader;
}

View*					ViewItemHUDReader::open(XMLTree::iterator xml)
{
	// parse
	bool primary = false;
	xml->getAttribute("primary", xmlParseEnum(s_xmlEnumBool, xmlSetVar(primary)));

	// create item
	assert(item == NULL);
	item = new ViewItemHUD(primary);
	return item;
}
