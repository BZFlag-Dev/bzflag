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

#include "ViewItemViewport.h"
#include "ViewReader.h"
#include "bzfgl.h"

//
// ViewItemViewport
//

ViewItemViewport::ViewItemViewport()
{
	w.fraction = 1.0f;
	h.fraction = 1.0f;
	aspect     = Free;
}

ViewItemViewport::~ViewItemViewport()
{
	// do nothing
}

void					ViewItemViewport::setRegion(
								const ViewSize& _x, const ViewSize& _y,
								const ViewSize& _w, const ViewSize& _h,
								Aspect _aspect)
{
	x      = _x;
	y      = _y;
	w      = _w;
	h      = _h;
	aspect = _aspect;
}

bool					ViewItemViewport::onPreRender(
								float xParent, float yParent,
								float wParent, float hParent)
{
	float vx = x.get(wParent);
	float vy = y.get(hParent);
	float vw = w.get(wParent);
	float vh = h.get(hParent);
	if (aspect == WidthFromHeight)
		vw = w.fraction * vh;
	else if (aspect == HeightFromWidth)
		vh = h.fraction * vw;

	// flip negative coordinates
	if (vx < 0.0f)
		vx = wParent + vx - vw;
	if (vy < 0.0f)
		vy = hParent + vy - vh; 

	// prep
	onRender(xParent + vx, yParent + vy, vw, vh);

	// we want to override the region passed to child views so we
	// render them here and return false.
	renderChildren(xParent + vx, yParent + vy, vw, vh);

	// clean up
	onPostRender(xParent, yParent, wParent, hParent);

	return false;
}

void					ViewItemViewport::onPostRender(
								float, float, float wParent, float hParent)
{
	glPopAttrib();

	// restore *probable* parent transform
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, wParent, 0.0, hParent, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void					ViewItemViewport::onRender(
								float x, float y, float w, float h)
{
	// compute area carefully to avoid gaps and overlap
	GLint   ix = static_cast<GLint>(x + 0.5f);
	GLint   iy = static_cast<GLint>(y + 0.5f);
	GLsizei iw = static_cast<GLsizei>(x + w + 0.5f) - ix;
	GLsizei ih = static_cast<GLsizei>(y + h + 0.5f) - iy;

	// set viewport and scissor to area
	glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT);
	glViewport(ix, iy, iw, ih);
	glScissor(ix, iy, iw, ih);

	// 1-to-1 pixel mapping
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, w, 0.0, h, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


//
// ViewItemViewportReader
//

ViewItemViewportReader::ViewItemViewportReader() : item(NULL)
{
	// do nothing
}

ViewItemViewportReader::~ViewItemViewportReader()
{
	if (item != NULL)
		item->unref();
}

ViewTagReader*			ViewItemViewportReader::clone() const
{
	return new ViewItemViewportReader;
}

View*					ViewItemViewportReader::open(
								const ConfigReader::Values& values)
{
	assert(item == NULL);

	// get region
	ViewSize x, y, w, h;
	ViewItemViewport::Aspect aspect = ViewItemViewport::Free;
	ViewReader::readSize(values, "x", x, 0.0f, 0.0f);
	ViewReader::readSize(values, "y", y, 0.0f, 0.0f);
	const bool wRelative = ViewReader::readSize(values, "w", w, 0.0f, 1.0f);
	const bool hRelative = ViewReader::readSize(values, "h", h, 0.0f, 1.0f);
	if (wRelative && hRelative) {
		w.pixel    = 0.0;
		w.fraction = 1.0;
		h.pixel    = 0.0;
		h.fraction = 1.0;
	}
	else if (wRelative) {
		aspect = ViewItemViewport::WidthFromHeight;
	}
	else if (hRelative) {
		aspect = ViewItemViewport::HeightFromWidth;
	}

	// create item
	item = create();
	item->setRegion(x, y, w, h, aspect);

	return item;
}

void					ViewItemViewportReader::close()
{
	assert(item != NULL);
	item = NULL;
}

ViewItemViewport*				ViewItemViewportReader::create() const
{
	return new ViewItemViewport;
}
