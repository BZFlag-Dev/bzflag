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

#ifndef BZF_VIEWITEMVIEWPORT_H
#define BZF_VIEWITEMVIEWPORT_H

#include "View.h"

// sets/restores the rendering viewport
class ViewItemViewport : public View {
public:
	enum Aspect { Free, WidthFromHeight, HeightFromWidth };

	ViewItemViewport();

	void				setRegion(
							const ViewSize& x, const ViewSize& y,
							const ViewSize& w, const ViewSize& h,
							Aspect aspect);

protected:
	virtual ~ViewItemViewport();

	const ViewSize&		getX() const { return x; }
	const ViewSize&		getY() const { return y; }
	const ViewSize&		getW() const { return w; }
	const ViewSize&		getH() const { return h; }
	Aspect				getAspect() const { return aspect; }

	// default sets the viewport to the given pixel coordinates
	virtual void		onRender(float x, float y, float w, float h);

	// View overrides
	virtual bool		onPreRender(float x, float y, float w, float h);
	virtual void		onPostRender(float x, float y, float w, float h);

private:
	ViewSize			x, y, w, h;
	Aspect				aspect;
};

class ViewItemViewportReader : public ViewTagReader {
public:
	ViewItemViewportReader();
	virtual ~ViewItemViewportReader();

	// ViewTagReader overrides
	virtual ViewTagReader* clone() const;
	virtual View*		open(const ConfigReader::Values& values);
	virtual void		close();

protected:
	virtual ViewItemViewport* create() const;

private:
	ViewItemViewport*	item;
};

#endif
