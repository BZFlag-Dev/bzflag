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

#ifndef BZF_HUD_MANAGER_H
#define BZF_HUD_MANAGER_H

#include "common.h"
#include "BzfString.h"
#include <map>

#define HUDMGR (HUDManager::getInstance())

class HUDManager {
public:
	typedef void (*Callback)(const BzfString& name,
							float heading, const float* color,
							void* userData);

	~HUDManager();

	// set the motion box shapes.  the sizes are measured each way
	// from the center.
	void				setCenter(int x, int y);
	void				setNoMotionSize(int w, int h);
	void				setMaxMotionSize(int w, int h);

	// set the hud color
	void				setColor(const float*);

	// add/remove markers.  adding an existing marker overwrites the
	// current values.
	void				addHeadingMarker(const BzfString&,
							float heading, const float* color);
	void				removeHeadingMarker(const BzfString&);

	// get the motion box shapes
	void				getCenter(int& x, int& y) const;
	void				getNoMotionSize(int& w, int& h) const;
	void				getMaxMotionSize(int& w, int& h) const;

	// get the hud color
	const float*		getColor() const;

	// invoke a callback for each heading marker
	void				iterate(Callback, void* userData);

	static HUDManager*	getInstance();

private:
	HUDManager();

private:
	struct Marker {
	public:
		float			heading;
		float			color[3];
	};
	typedef std::map<BzfString, Marker> Markers;

	int					x, y;
	int					wMin, hMin;
	int					wMax, hMax;
	float				color[3];
	Markers				markers;
	static HUDManager*	mgr;
};

#endif
