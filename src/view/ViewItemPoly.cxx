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

#include "ViewItemPoly.h"
#include "ViewReader.h"
#include "bzfgl.h"
#include <assert.h>

//
// ViewItemPoly
//

ViewItemPoly::ViewItemPoly() : alpha(1.0f)
{
	// do nothing
}

ViewItemPoly::~ViewItemPoly()
{
	// do nothing
}

void					ViewItemPoly::addVertex(
								const ViewSize& _x,
								const ViewSize& _y,
								const ViewSize& _z,
								const ViewColor& color,
								const float* uv)
{
	vertices.push_back(Vertex());
	Vertex& vertex = vertices.back();
	vertex.x     = _x;
	vertex.y     = _y;
	vertex.z     = _z;
	vertex.color = color;
	vertex.uv[0] = uv[0];
	vertex.uv[1] = uv[1];

	if (vertex.color.color[3] < alpha)
		alpha = vertex.color.color[3];
}

void					ViewItemPoly::close()
{
	OpenGLGStateBuilder builder;
	builder.setTexture(getState().texture);
	if (alpha < 1.0f)
		builder.setBlending(GState::kSrcAlpha, GState::kOneMinusSrcAlpha);
	gstate = builder.getState();
}

bool					ViewItemPoly::onPreRender(
								float, float, float, float)
{
	return true;
}

void					ViewItemPoly::onPostRender(
								float, float, float w, float h)
{
	// draw
	gstate.setState();
	glBegin(GL_POLYGON);
		for (Vertices::const_iterator index = vertices.begin();
								index != vertices.end(); ++index) {
			const Vertex& vertex = *index;
			glTexCoord2fv(vertex.uv);
			vertex.color.set();
			glVertex3f(vertex.x.get(w), vertex.y.get(h), vertex.z.get(1.0));
		}
	glEnd();
}

void					ViewItemPoly::onUpdateColors()
{
	for (Vertices::iterator index = vertices.begin();
								index != vertices.end(); ++index)
		index->color.update();
}


//
// ViewItemPolyReader
//

ViewItemPolyReader::ViewItemPolyReader() : item(NULL)
{
	uv[0] = uv[1] = 0.0f;
}

ViewItemPolyReader::~ViewItemPolyReader()
{
	if (item != NULL)
		item->unref();
}

ViewTagReader* 	ViewItemPolyReader::clone() const
{
	return new ViewItemPolyReader;
}

View*					ViewItemPolyReader::open(
								const ConfigReader::Values&)
{
	assert(item == NULL);
	item = new ViewItemPoly;
	return item;
}

void					ViewItemPolyReader::close()
{
	assert(item != NULL);
	item->close();
	item = NULL;
}

bool					ViewItemPolyReader::push(const BzfString& tag,
								const ConfigReader::Values& values)
{
	assert(item != NULL);

	ConfigReader::Values::const_iterator index;

	if (tag == "texcoord") {
		index = values.find("x");
		if (index != values.end())
			uv[0] = atof(index->second.c_str());
		index = values.find("y");
		if (index != values.end())
			uv[1] = atof(index->second.c_str());
		return true;
	}

	else if (tag == "vertex") {
		ConfigReader::Values::const_iterator xIndex = values.find("x");
		ConfigReader::Values::const_iterator yIndex = values.find("y");
		if (xIndex != values.end() || yIndex != values.end()) {
			ViewSize x, y, z;
			ViewReader::readSize(values, "x", x, 0.0f, 0.0f);
			ViewReader::readSize(values, "y", y, 0.0f, 0.0f);
			ViewReader::readSize(values, "z", z, 0.0f, 0.0f);
			item->addVertex(x, y, z, getState().color, uv);
		}
		return true;
	}

	return false;
}

void					ViewItemPolyReader::pop(const BzfString&)
{
	// ignore
}
