;/* bzflag
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

#include "SceneVisitorSimpleRender.h"
#include "StateDatabase.h"
#include "TimeKeeper.h"
#include "bzfgl.h"
#include <math.h>

SceneVisitorSimpleRender::SceneVisitorSimpleRender() : numLights(0)
{
	// some built-in paramters
	getParams().pushFloat("area", 1.0f);
	getParams().pushFloat("frame", 0.0f);
	getParams().pushFloat("time", 0.0f);

	// default gstate
	gstateStack.push_back(OpenGLGState());

	// default matrices
	Matrix m;
	modelXFormStack.push_back(m);
	projectionXFormStack.push_back(m);
	textureXFormStack.push_back(m);

	// allow up to 8 lights
	maxLights = 8;
}

SceneVisitorSimpleRender::~SceneVisitorSimpleRender()
{
	// do nothing
}

void					SceneVisitorSimpleRender::setArea(float size)
{
	getParams().setFloat("area", size);
}

void					SceneVisitorSimpleRender::setTime(float t)
{
	getParams().setFloat("time", t);
}

void					SceneVisitorSimpleRender::setFrame(float n)
{
	getParams().setFloat("frame", n);
}

const SceneVisitorSimpleRender::Instruments*
						SceneVisitorSimpleRender::instrGet() const
{
	return &instruments;
}

bool					SceneVisitorSimpleRender::descend(SceneNodeGroup* n)
{
	++instruments.nNodes;
	return SceneVisitor::descend(n);
}

bool					SceneVisitorSimpleRender::traverse(SceneNode* node)
{
	// if debugging then flush rendering pipeline for accurate timing
	const bool debug = BZDB->isTrue("displayDebug");
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

	// prep rendering state
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	// FIXME -- init() should become unnecessary with consistent use of gstates
	OpenGLGState::init();

	// draw
	const bool result = node->visit(this);

	// restore state
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	// flush pipeline again so we get the time required to actually
	// draw everything, not just to shove it down the pipeline.
	if (debug)
		glFinish();

	// record the elapsed time
	instruments.time = TimeKeeper::getCurrent() - t;

	return result;
}

bool					SceneVisitorSimpleRender::visit(SceneNodeAnimate* n)
{
	const BzfString& dst = n->dst.get();
	getParams().pushFloat(dst, n->get(getParams()));
	const bool result = descend(n);
	getParams().popFloat(dst);
	return result;
}

bool					SceneVisitorSimpleRender::visit(SceneNodeParameters* n)
{
	n->push(getParams());
	const bool result = descend(n);
	n->pop(getParams());
	return result;
}

bool					SceneVisitorSimpleRender::visit(SceneNodeSelector* n)
{
	// FIXME -- what do we pass to a selector?  need enough to do
	// LOD based on projected area and z-depth.
	getParams().pushInt("mask", n->get(modelXFormStack.back(),
								projectionXFormStack.back(),
								getParams()));
	const bool result = descend(n);
	getParams().popInt("mask");
	return result;
}

bool					SceneVisitorSimpleRender::visit(SceneNodeGState* n)
{
	gstateStack.push_back(n->get());
	gstateStack.back().setState();
	const bool result = descend(n);
	gstateStack.pop_back();
	gstateStack.back().setState();
	return result;
}

bool					SceneVisitorSimpleRender::visit(SceneNodeBaseTransform* n)
{
	XFormStack* stack;
	GLenum mode;

	// what kind of matrix?
	switch (n->type.get()) {
		case SceneNodeBaseTransform::ModelView:
			stack = &modelXFormStack;
			mode  = GL_MODELVIEW;
			break;

		case SceneNodeBaseTransform::Projection:
			stack = &projectionXFormStack;
			mode  = GL_PROJECTION;
			break;

		case SceneNodeBaseTransform::Texture:
			stack = &textureXFormStack;
			mode  = GL_TEXTURE;
			break;

     default:
       return descend(n);
	}

	// verify that we're not trying to access a non-existent matrix
	unsigned int level = n->up.get();
	if (level >= stack->size()) {
		// FIXME -- should note the error
		return descend(n);
	}

	// push a copy of the old matrix
	stack->push_back((*stack)[stack->size() - 1 - level]);

	// let node modify new matrix
	n->get(stack->back(), getParams());

	// load the new matrix
	glMatrixMode(mode);
	glLoadMatrixf(stack->back().get());
	glMatrixMode(GL_MODELVIEW);

	// descend
	++instruments.nTransforms;
	const bool result = descend(n);

	// pop stack
	stack->pop_back();
	glMatrixMode(mode);
	glLoadMatrixf(stack->back().get());
	glMatrixMode(GL_MODELVIEW);

	return result;
}
bool					SceneVisitorSimpleRender::visit(SceneNodeGeometry* n)
{
	// calculate geometry
	n->compute(getParams());
	n->setBundle(SceneNodeGeometry::kComputedIndex);

	// get arrays
	const SceneNodeVFFloat* color   = n->color;
	const SceneNodeVFFloat* texture = n->texcoord;
	const SceneNodeVFFloat* normal  = n->normal;
	const SceneNodeVFFloat* vertex  = n->vertex;

	// check which arrays are used
	const bool hasColor   = (color->getNum()   > 0);
	const bool hasTexture = (texture->getNum() > 0);
	const bool hasNormal  = (normal->getNum()  > 0);
	const bool hasVertex  = (vertex->getNum()  > 0);

	// set rendering state
	if (hasColor) {
		colorStack.push_back(color);
		if (color->getNum() == 4) {
			glColor4fv(color->get());
			glDisableClientState(GL_COLOR_ARRAY);
		}
		else {
			glColorPointer(4, GL_FLOAT, 0, color->get());
			glEnableClientState(GL_COLOR_ARRAY);
		}
	}
	if (hasTexture) {
		texcoordStack.push_back(texture);
		glTexCoordPointer(2, GL_FLOAT, 0, texture->get());
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	if (hasNormal) {
		normalStack.push_back(normal);
/*
    specularStack.push_back(n->getSpecularColor());
    emissiveStack.push_back(n->getEmissiveColor());
    shininessStack.push_back(n->getShininess());
*/
    if (normal->getNum() == 3) {
      glNormal3fv(normal->get());
      glDisableClientState(GL_NORMAL_ARRAY);
    }
    else {
      glNormalPointer(GL_FLOAT, 0, normal->get());
      glEnableClientState(GL_NORMAL_ARRAY);
    }
/*
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, n->getSpecularColor());
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, n->getEmissiveColor());
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, n->getShininess());
*/
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
	}
	if (hasVertex) {
		vertexStack.push_back(vertex);
		glVertexPointer(3, GL_FLOAT, 0, vertex->get());
		glEnableClientState(GL_VERTEX_ARRAY);
	}

	// descend
	const bool result = descend(n);

	// restore rendering state
	if (hasColor) {
		colorStack.pop_back();
		if (colorStack.empty()) {
			glDisableClientState(GL_COLOR_ARRAY);
		}
		else {
			color = colorStack.back();
			if (color->getNum() == 4) {
				glColor4fv(color->get());
				glDisableClientState(GL_COLOR_ARRAY);
			}
			else {
				glColorPointer(4, GL_FLOAT, 0, color->get());
			}
		}
	}
	if (hasTexture) {
		texcoordStack.pop_back();
		if (texcoordStack.empty())
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		else
			glTexCoordPointer(2, GL_FLOAT, 0, texcoordStack.back()->get());
	}
	if (hasNormal) {
		normalStack.pop_back();
/*
    specularStack.pop_back();
    emissiveStack.pop_back();
    shininessStack.pop_back();
*/
    if (normalStack.empty()) {
      glDisableClientState(GL_NORMAL_ARRAY);
      glDisable(GL_COLOR_MATERIAL);
      glDisable(GL_LIGHTING);
    }
    else {
      normal = normalStack.back();
      if (normal->getNum() == 3) {
		glNormal3fv(normal->get());
		glDisableClientState(GL_NORMAL_ARRAY);
      }
      else {
		glNormalPointer(GL_FLOAT, 0, normal->get());
      }
/*
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularStack.back());
      glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissiveStack.back());
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininessStack.back());
*/
    }
	}
	if (hasVertex) {
		vertexStack.pop_back();
		if (vertexStack.empty())
			glDisableClientState(GL_VERTEX_ARRAY);
		else
			glVertexPointer(3, GL_FLOAT, 0, vertexStack.back()->get());
	}

	return result;
}

bool					SceneVisitorSimpleRender::visit(SceneNodePrimitive* n)
{
	static const GLenum typeMap[] = {
		GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP,
		GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN };

	// count primitives
	const unsigned int num  = n->index.getNum();
	const unsigned int type = n->type.get();
	switch (type) {
		case SceneNodePrimitive::Points:
			instruments.nPoints += num;
			break;

		case SceneNodePrimitive::Lines:
			instruments.nLines += (num >> 1);
			break;

		case SceneNodePrimitive::LineStrip:
			instruments.nLines += num - 1;
			break;

		case SceneNodePrimitive::LineLoop:
			instruments.nLines += num;
			break;

		case SceneNodePrimitive::Triangles:
			instruments.nTriangles += num / 3;
			break;

		case SceneNodePrimitive::TriangleStrip:
			instruments.nTriangles += num - 2;
			break;

		case SceneNodePrimitive::TriangleFan:
			instruments.nTriangles += num - 2;
			break;
	}
	++instruments.nNodes;

	// render
	const unsigned int* index = n->index.get();
	glDrawElements(typeMap[type], num, GL_UNSIGNED_INT, index);

	return true;
}

bool					SceneVisitorSimpleRender::visit(SceneNodeLight* n)
{
	const bool bind = (numLights < maxLights);
	if (bind) {
		n->compute(getParams());
		glLightfv(GL_LIGHT0 + numLights, GL_AMBIENT, n->getAmbientColor());
		glLightfv(GL_LIGHT0 + numLights, GL_DIFFUSE, n->getDiffuseColor());
		glLightfv(GL_LIGHT0 + numLights, GL_SPECULAR, n->getSpecularColor());
		glLightfv(GL_LIGHT0 + numLights, GL_POSITION, n->getPosition());
		glLightfv(GL_LIGHT0 + numLights, GL_SPOT_DIRECTION, n->getSpotDirection());
		glLightf(GL_LIGHT0 + numLights, GL_SPOT_EXPONENT, n->getSpotExponent());
		glLightf(GL_LIGHT0 + numLights, GL_SPOT_CUTOFF, n->getSpotCutoff());
		const float* atten = n->getAttenuation();
		glLightf(GL_LIGHT0 + numLights, GL_CONSTANT_ATTENUATION, atten[0]);
		glLightf(GL_LIGHT0 + numLights, GL_LINEAR_ATTENUATION, atten[1]);
		glLightf(GL_LIGHT0 + numLights, GL_QUADRATIC_ATTENUATION, atten[2]);
		glEnable(GL_LIGHT0 + numLights);
	}
	++numLights;

	const bool result = descend(n);

	--numLights;
	if (bind) {
		glDisable(GL_LIGHT0 + numLights);
	}

	return result;
}
