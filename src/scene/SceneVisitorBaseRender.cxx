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

#include "SceneVisitorBaseRender.h"
#include "StateDatabase.h"
#include "TimeKeeper.h"
#include "bzfgl.h"
#include <math.h>
#include <algorithm>

//
// SceneVisitorBaseRender
//

const std::string SceneVisitorBaseRender::s_nameArea("area");
const std::string SceneVisitorBaseRender::s_nameTime("time");
const std::string SceneVisitorBaseRender::s_nameFrame("frame");
const std::string SceneVisitorBaseRender::s_nameDebug("displayDebug");
const std::string SceneVisitorBaseRender::s_renderSmoothing("renderSmoothing");
const std::string SceneVisitorBaseRender::s_renderBlending("renderBlending");
const std::string SceneVisitorBaseRender::s_renderLighting("renderLighting");
const std::string SceneVisitorBaseRender::s_renderTexturing("renderTexturing");

SceneVisitorBaseRender::SceneVisitorBaseRender()
{
	// set some parameters
	getParams().pushFloat(s_nameArea,  1.0f);
	getParams().pushFloat(s_nameFrame, 0.0f);
	getParams().pushFloat(s_nameTime,  0.0f);
	getParams().pushFloat(s_renderSmoothing, 1.0f);
	getParams().pushFloat(s_renderBlending,  1.0f);
	getParams().pushFloat(s_renderLighting,  1.0f);
	getParams().pushFloat(s_renderTexturing, 1.0f);
}

SceneVisitorBaseRender::~SceneVisitorBaseRender()
{
	// do nothing
}

void					SceneVisitorBaseRender::setArea(float size)
{
	getParams().setFloat(s_nameArea, size);
}

void					SceneVisitorBaseRender::setTime(float t)
{
	getParams().setFloat(s_nameTime, t);
}

void					SceneVisitorBaseRender::setFrame(float n)
{
	getParams().setFloat(s_nameFrame, n);
}

const bool				SceneVisitorBaseRender::isDebugging() const
{
	return BZDB->isTrue(s_nameDebug);
}

const SceneVisitorBaseRender::Instruments*
						SceneVisitorBaseRender::instrGet() const
{
	return &instruments;
}

SceneVisitorBaseRender::Instruments*
						SceneVisitorBaseRender::getInstr()
{
	return &instruments;
}

bool					SceneVisitorBaseRender::descend(SceneNodeGroup* n)
{
	++instruments.nNodes;
	return SceneVisitor::descend(n);
}

bool					SceneVisitorBaseRender::traverse(SceneNode* node)
{
	// if debugging then flush rendering pipeline for accurate timing
	const bool debug = isDebugging();
	if (debug)
		glFinish();

	// initialize instruments
	TimeKeeper t(TimeKeeper::getCurrent());
	instruments.time        = 0.0f;
	instruments.nNodes      = 0;
	instruments.nTransforms = 0;
	instruments.nPoints     = 0;
	instruments.nLines      = 0;
	instruments.nTriangles  = 0;

	// done if nothing to traverse
	if (node == NULL)
		return false;

	// set parameters
	getParams().setFloat(s_renderSmoothing,
								BZDB->isTrue(s_renderSmoothing) ? 1.0f : 0.0f);
	getParams().setFloat(s_renderBlending,
								BZDB->isTrue(s_renderBlending) ? 1.0f : 0.0f);
	getParams().setFloat(s_renderLighting,
								BZDB->isTrue(s_renderLighting) ? 1.0f : 0.0f);
	getParams().setFloat(s_renderTexturing,
								BZDB->isTrue(s_renderTexturing) ? 1.0f : 0.0f);

	// collect primitives
	const bool result = node->visit(this);

	// flush pipeline again so we get the time required to actually
	// draw everything, not just to shove it down the pipeline.
	if (debug)
		glFinish();

	// record the elapsed time
	instruments.time = TimeKeeper::getCurrent() - t;

	return result;
}

void					SceneVisitorBaseRender::setStipple(float alpha)
{
#define REPMASK(__s1,__s2,__s3,__s4) { __s1, __s2, __s3, __s4, __s1, __s2, __s3, __s4, __s1, __s2, __s3, __s4, __s1, __s2, __s3, __s4, __s1, __s2, __s3, __s4, __s1, __s2, __s3, __s4, __s1, __s2, __s3, __s4, __s1, __s2, __s3, __s4 }
	static const GLuint polygonStipples[][32] =
	{
		REPMASK(0x00000000, 0x00000000, 0x00000000, 0x00000000),

		REPMASK(0x88888888, 0x00000000, 0x00000000, 0x00000000),
		REPMASK(0x88888888, 0x00000000, 0x22222222, 0x00000000),
		REPMASK(0xaaaaaaaa, 0x00000000, 0x22222222, 0x00000000),
		REPMASK(0xaaaaaaaa, 0x00000000, 0xaaaaaaaa, 0x00000000),

		REPMASK(0xaaaaaaaa, 0x44444444, 0xaaaaaaaa, 0x00000000),
		REPMASK(0xaaaaaaaa, 0x44444444, 0xaaaaaaaa, 0x11111111),
		REPMASK(0xaaaaaaaa, 0x55555555, 0xaaaaaaaa, 0x11111111),
		REPMASK(0xaaaaaaaa, 0x55555555, 0xaaaaaaaa, 0x55555555),

		REPMASK(0xeeeeeeee, 0x55555555, 0xaaaaaaaa, 0x55555555),
		REPMASK(0xeeeeeeee, 0x55555555, 0xbbbbbbbb, 0x55555555),
		REPMASK(0xffffffff, 0x55555555, 0xbbbbbbbb, 0x55555555),
		REPMASK(0xffffffff, 0x55555555, 0xffffffff, 0x55555555),

		REPMASK(0xffffffff, 0xdddddddd, 0xffffffff, 0x55555555),
		REPMASK(0xffffffff, 0xdddddddd, 0xffffffff, 0x77777777),
		REPMASK(0xffffffff, 0xffffffff, 0xffffffff, 0x77777777),
		REPMASK(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff),
	};
#undef REPMASK
	static const GLushort lineStipples[] =
	{
		0x0000,
		0x0001,
		0x0101,
		0x0111,
		0x1111,

		0x1115,
		0x1515,
		0x1555,
		0x5555,

		0x5557,
		0x5757,
		0x5777,
		0x7777,

		0x777f,
		0x7f7f,
		0x7fff,
		0xffff
	};

	unsigned int pIndex = static_cast<unsigned int>(
							countof(polygonStipples) * alpha);
	unsigned int lIndex = static_cast<unsigned int>(
							countof(lineStipples) * alpha);
	if (pIndex == countof(polygonStipples))
		pIndex = countof(polygonStipples) - 1;
	if (lIndex == countof(lineStipples))
		lIndex = countof(lineStipples) - 1;

	glLineStipple(1, lineStipples[lIndex]);
	glPolygonStipple(reinterpret_cast<const GLubyte*>(polygonStipples[pIndex]));
}
// ex: shiftwidth=4 tabstop=4
