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

#include "View.h"
#include "bzfgl.h"

//
// ViewSize
//

ViewSize::ViewSize() : pixel(0.0f), fraction(0.0f)
{
	// do nothing
}

ViewSize::ViewSize(float pixel_, float fraction_) :
							pixel(pixel_), fraction(fraction_)
{
	// do nothing
}

float					ViewSize::get(float fullSize) const
{
	return pixel + fullSize * fraction;
}


//
// ViewColor
//

float					ViewColor::colors[][3] = {
								{ 0.0f, 0.0f, 0.0f },
								{ 1.0f, 1.0f, 1.0f },
								{ 1.0f, 0.0f, 0.0f },
								{ 0.0f, 1.0f, 0.0f },
								{ 0.0f, 0.0f, 1.0f },
								{ 1.0f, 0.0f, 1.0f },
								{ 1.0f, 1.0f, 0.0f },
								{ 1.0f, 1.0f, 0.0f }
						};
ViewColor::Source		ViewColor::myTeam = Rogue;

ViewColor::ViewColor() : source(Black)
{
	color[3] = 1.0f;
	scale[0] = scale[1] = scale[2] = 1.0f;
	shift[0] = shift[1] = shift[2] = 0.0f;
	update();
}

void					ViewColor::set() const
{
	glColor4fv(color);
}

void					ViewColor::update()
{
	// choose source color
	Source tmpSource = source;
	if (tmpSource == MyTeam)
		tmpSource = myTeam;
	const float* src = colors[tmpSource];

	// compute scaled and shifted color
	color[0] = shift[0] + scale[0] * src[0];
	color[1] = shift[1] + scale[1] * src[1];
	color[2] = shift[2] + scale[2] * src[2];

	// clamp to valid range
#define CLAMP(_x, _a, _b) (((_x) < (_a)) ? (_a) : (((_x) > (_b)) ? (_b) : (_x)))
	color[0] = CLAMP(color[0], 0.0f, 1.0f);
	color[1] = CLAMP(color[1], 0.0f, 1.0f);
	color[2] = CLAMP(color[2], 0.0f, 1.0f);
#undef CLAMP
}

void					ViewColor::setColor(Source src, const float* color)
{
	colors[src][0] = color[0];
	colors[src][1] = color[1];
	colors[src][2] = color[2];
	View::setColorsDirty();
}

void					ViewColor::setMyTeam(Source src)
{
	if (src == MyTeam)
		src = Rogue;
	if (src != myTeam) {
		myTeam = src;
		View::setColorsDirty();
	}
}


//
// ViewState
//

ViewState::ViewState() : hAlign(Left), vAlign(Bottom)
{
	// do nothing
}


//
// View
//

int						View::colorMailbox = 0;

View::View() : refCount(1), style(0), colorMark(colorMailbox - 1)
{
	// do nothing
}

View::~View()
{
	// clean up
	for (Views::iterator index = views.begin(); index != views.end(); ++index)
		(*index)->unref();
}

int						View::ref()
{
	assert(refCount >= 1);
	return ++refCount;
}

int						View::unref()
{
	assert(refCount >= 1);
	int n = --refCount;
	if (n == 0) {
		refCount = 0xdeadbeef;
		delete this;
	}
	return n;
}

void					View::pushChild(View* child)
{
	if (child != NULL) {
		child->ref();
		views.push_back(child);
	}
}

void					View::setState(const ViewState& _state)
{
	state = _state;
}

void					View::render(float x, float y, float w, float h)
{
	// get colors up to date
	if (colorMark != colorMailbox) {
		colorMark = colorMailbox;
		state.color.update();
		onUpdateColors();
	}

	// prep font size
	const float hFont = state.fontSize.get(h);
	state.font.setSize(hFont, hFont);

	// draw
	if (onPreRender(x, y, w, h)) {
		renderChildren(x, y, w, h);
		onPostRender(x, y, w, h);
	}
}

bool					View::onPreRender(float, float, float, float)
{
	return true;
}

void					View::onPostRender(float, float, float, float)
{
	// do nothing
}

void					View::renderChildren(float x, float y, float w, float h)
{
	for (Views::const_iterator index = views.begin();
								index != views.end(); ++index)
		(*index)->render(x, y, w, h);
}

void					View::setColorsDirty()
{
	++colorMailbox;
}
