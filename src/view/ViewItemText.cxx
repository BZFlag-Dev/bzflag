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

#include "ViewItemText.h"
#include "ViewReader.h"
#include "StateDatabase.h"
#include "bzfgl.h" // FIXME -- for shadow.  make fonts with built-in shadow
#include <assert.h>

//
// ViewItemText
//

ViewItemText::ViewItemText() : shadow(false)
{
	// do nothing
}

ViewItemText::~ViewItemText()
{
	// do nothing
}

void					ViewItemText::setText(const BzfString& text)
{
	// parse line looking for '\n' and "${...}".  the former is a newline
	// and gets its own plain string entry.  the latter is a database
	// entry to be interpolated.  the items member contains pairs of
	// strings;  the first of each pair is a database name and the
	// second is a plain string.  $$ becomes a plain $.
	const char* scan = text.c_str();
	while (*scan != '\0') {
		BzfString name, plain;

		if (*scan == '$') {
			++scan;
			if (*scan == '\0') {
				// plain $ (a trailing $ should really be an error)
				plain = "$";
			}
			else if (*scan == '$') {
				// plain $
				plain = "$";
				++scan;
			}
			else if (*scan != '{') {
				// plain text
				plain.append(scan - 1, 2);
				++scan;
			}
			else {
				// it's a name.  find the closing brace.
				const char* base = ++scan;
				while (*scan != '0' && *scan != '}')
					++scan;

				// if no closing brace then it's an error but we'll just
				// use the plain text.
				if (*scan == '\0') {
					plain.append(base - 2, scan - base);
				}
				else {
					// save name
					name.append(base, scan - base);

					// eat }
					++scan;
				}
			}
		}

		// read plain string
		while (*scan != '\0') {
			// check for newline
			if (*scan == '\\' && scan[1] == 'n') {
				// newline.  if plain isn't empty then we can't use it yet,
				// but either way we're done with plain string.
				if (plain.empty()) {
					plain = "\n";
					scan += 2;
				}
				break;
			}

			// check for name
			if (*scan == '$' && scan[1] == '{')
				break;

			// check for $$
			if (*scan == '$' && scan[1] == '$') {
				plain += "$";
				scan += 2;
			}

			else {
				// append literal character
				plain.append(scan, 1);
				++scan;
			}
		}

		// append item
		items.push_back(name);
		items.push_back(plain);
		name  = "";
		plain = "";
	}
}

void					ViewItemText::setPosition(
								const ViewSize& _x, const ViewSize& _y)
{
	x = _x;
	y = _y;
}

void					ViewItemText::setShadow(bool _shadow)
{
	shadow = _shadow;
}

bool					ViewItemText::onPreRender(
								float, float, float, float)
{
	return true;
}

void					ViewItemText::onPostRender(
								float, float, float w, float h)
{
	// get the lines to draw
	Items lines;
	makeLines(lines);

	// get reference to font
	const OpenGLTexFont& font = getState().font;

	// compute width and height
	float wText = 0.0f, hText = 0.0f;
	float spacing = font.getSpacing();
	Items::const_iterator index;
	for (index = lines.begin(); index != lines.end(); ++index) {
		hText += spacing;
		float wLine = font.getWidth(*index);
		if (wLine > wText)
			wText = wLine;
	}
	if (shadow) {
		wText += 1.0f;
		hText += 1.0f;
	}

	// compute position
	float xText = x.get(w);
	float yText = y.get(h);
	if (xText < 0.0)
		xText = w + xText;
	if (yText < 0.0)
		yText = h + yText;
	switch (getState().vAlign) {
		case ViewState::Bottom:
			yText += hText;
			break;

		case ViewState::Center:
			yText += 0.5f * hText;
			break;

		case ViewState::Top:
			// no adjustment
			break;
	}
	yText -= font.getAscent();
	const ViewState::Align xAlign = getState().hAlign;

	// draw
	OpenGLGState::resetState();
	getState().color.set();
	for (index = lines.begin(); index != lines.end(); ++index) {
		float wLine = font.getWidth(*index);
		if (shadow)
			wLine += 1.0f;
		float xLine = xText;
		switch (xAlign) {
			case ViewState::Left:
				// no adjustment
				break;

			case ViewState::Center:
				xLine -= 0.5f * wLine;
				break;

			case ViewState::Right:
				xLine -= wLine;
				break;
		}
		if (shadow) {
			glColor3f(0.0f, 0.0f, 0.0f);
			font.draw(*index, xLine, yText - 1.0f);
			font.draw(*index, xLine + 1.0f, yText);
			font.draw(*index, xLine + 1.0f, yText - 1.0f);
			getState().color.set();
		}
		font.draw(*index, xLine, yText);
		yText -= spacing;
	}
}

void					ViewItemText::makeLines(Items& lines)
{
	Items::const_iterator index = items.begin();
	while (index != items.end())
		lines.push_back(makeLine(index));
}

BzfString				ViewItemText::makeLine(
								Items::const_iterator& index) const
{
	BzfString line;

	// items is a list of pairs.  the first of each pair is a database
	// name and the second is a plain string (possibly \n).
	while (index != items.end()) {
		// get database name
		BzfString item = *index;
		++index;
		assert(index != items.end());

		// interpolate name
		if (!item.empty())
			line += BZDB->get(item);

		// get plain string
		item = *index;
		++index;

		// if end of line then we're done
		if (item == "\n")
			break;

		// append plain string
		line += item;
	}

	return line;
}


//
// ViewItemTextReader
//

ViewItemTextReader::ViewItemTextReader() : item(NULL)
{
	// do nothing
}

ViewItemTextReader::~ViewItemTextReader()
{
	if (item != NULL)
		item->unref();
}

ViewTagReader* 			ViewItemTextReader::clone() const
{
	return new ViewItemTextReader;
}

View*					ViewItemTextReader::open(
								const ConfigReader::Values& values)
{
	assert(item == NULL);

	// get parameters
	ViewSize x, y;
	bool shadow = false;
	ViewReader::readSize(values, "x", x, 0.0f, 0.0f);
	ViewReader::readSize(values, "y", y, 0.0f, 0.0f);
	ConfigReader::Values::const_iterator index = values.find("shadow");
	if (index != values.end())
		shadow = (index->second == "yes");

	// create item
	item = new ViewItemText;
	item->setPosition(x, y);
	item->setShadow(shadow);
	msg = "";

	return item;
}

void					ViewItemTextReader::close()
{
	assert(item != NULL);
	item->setText(msg);
	item = NULL;
}

bool					ViewItemTextReader::data(const BzfString& line)
{
	if (!msg.empty())
		msg += " ";
	msg += line;
	return true;
}
