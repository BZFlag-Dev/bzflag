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

#ifndef BZF_VIEWITEMHUD_H
#define BZF_VIEWITEMHUD_H

#include "View.h"

class ViewItemHUD : public View {
public:
	ViewItemHUD(bool primary);

protected:
	virtual ~ViewItemHUD();

	struct MarkerCBInfo {
	public:
		float			zero;
		float			w;
		float			centerHeading;
		float			majorTickDiff;
		float			majorTickSpacing;
	};

	// View overrides
	virtual bool		onPreRender(float x, float y, float w, float h);
	virtual void		onPostRender(float x, float y, float w, float h);

	// draw tapes
	virtual void		drawHeading(int x0, int y0, int x, int y, int w);
	virtual void		drawAltitude(int x0, int y0, int x, int y, int h);
	virtual void		drawMarker(const MarkerCBInfo*,
							float heading, const float* color);

private:
	static void			drawMarkerCB(const BzfString& name,
							float heading, const float* color,
							void* userData);

private:
	struct MarkerCBInfo2 {
	public:
		ViewItemHUD*	self;
		MarkerCBInfo	info;
	};

	bool				primary;
	int					x, y;
	int					noMotionSize;
	int					maxMotionSize;
};

class ViewItemHUDReader : public ViewTagReader {
public:
	ViewItemHUDReader();
	virtual ~ViewItemHUDReader();

	// ViewItemReader overrides
	virtual ViewTagReader* clone() const;
	virtual View*		open(const ConfigReader::Values&);
	virtual void		close();

private:
	ViewItemHUD*		item;
};

#endif
