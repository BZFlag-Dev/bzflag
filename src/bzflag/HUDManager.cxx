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

#include "HUDManager.h"
#include <assert.h>
#include <math.h>

//
// HUDManager
//

HUDManager*				HUDManager::mgr = NULL;

HUDManager::HUDManager()
{
	color[0] = color[1] = color[2] = 0.0f;
}

HUDManager::~HUDManager()
{
	mgr = NULL;
}

HUDManager*				HUDManager::getInstance()
{
	if (mgr == NULL)
		mgr = new HUDManager;
	return mgr;
}

void					HUDManager::setCenter(int _x, int _y)
{
	x = _x;
	y = _y;
}

void					HUDManager::setNoMotionSize(int w, int h)
{
	wMin = w;
	hMin = h;
}

void					HUDManager::setMaxMotionSize(int w, int h)
{
	wMax = w;
	hMax = h;
}

void					HUDManager::setColor(const float* _color)
{
	color[0] = _color[0];
	color[1] = _color[1];
	color[2] = _color[2];
}

void					HUDManager::addHeadingMarker(const BzfString& id,
								float heading, const float* color)
{
	Marker marker;
	marker.heading  = fmodf(810.0f - 180.0f * heading / M_PI, 360.0f);
	marker.color[0] = color[0];
	marker.color[1] = color[1];
	marker.color[2] = color[2];
	markers.erase(id);
	markers.insert(std::make_pair(id, marker));
}

void					HUDManager::removeHeadingMarker(const BzfString& id)
{
	markers.erase(id);
}

void					HUDManager::getCenter(int& _x, int& _y) const
{
	_x = x;
	_y = y;
}

void					HUDManager::getNoMotionSize(int& w, int& h) const
{
	w = wMin;
	h = hMin;
}

void					HUDManager::getMaxMotionSize(int& w, int& h) const
{
	w = wMax;
	h = hMax;
}

const float*				HUDManager::getColor() const
{
	return color;
}

void					HUDManager::iterate(Callback callback, void* userData)
{
	assert(callback != NULL);

	for (Markers::const_iterator index = markers.begin();
								index != markers.end(); ++index)
		(*callback)(index->first, index->second.heading,
								index->second.color, userData);
}
