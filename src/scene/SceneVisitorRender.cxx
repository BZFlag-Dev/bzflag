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

#include "SceneVisitorRender.h"
#include "StateDatabase.h"
#include "TimeKeeper.h"
#include "ViewFrustum.h"
#include "bzfgl.h"
#include <math.h>
#include <algorithm>

//
// SceneVisitorRenderReset -- calls resetScratchBundles() on geometry nodes
//

class SceneVisitorRenderReset : public SceneVisitor {
public:
	SceneVisitorRenderReset() { }
	~SceneVisitorRenderReset() { }

	virtual bool		visit(SceneNodeGroup* n);
	virtual bool		visit(SceneNodeGeometry* n);
};

bool					SceneVisitorRenderReset::visit(SceneNodeGroup* n)
{
	// descend into all children (regardless of group type)
	return n->SceneNodeGroup::descend(this, getParams());
}

bool					SceneVisitorRenderReset::visit(SceneNodeGeometry* n)
{
	n->resetScratchBundles();
	return SceneVisitor::visit(n);
}


//
// SceneVisitorRender::Job
//

SceneVisitorRender::Job::Job(const OpenGLGState& _gstate) :
								gstate(_gstate)
{
	// do nothing.  note that other members are not set.
}


//
// SceneVisitorRender
//

SceneVisitorRender::SceneVisitorRender() :
								nameMask("mask"),
								nameLighting("renderLighting"),
								dummyParams("dummy", 0, 0, 1)
{
	// create resetter visitor
	resetter = new SceneVisitorRenderReset;

	// default gstate
	gstateStack.push_back(OpenGLGState());

	// default matrices
	Matrix m;
	modelXFormStack.push_back(m);
	projectionXFormStack.push_back(m);
	textureXFormStack.push_back(m);
	modelXFormIndexStack.push_back(0);
	projectionXFormIndexStack.push_back(0);
	textureXFormIndexStack.push_back(0);

	// default primitive parameters
	stippleStack.push_back(&dummyParams);
	colorStack.push_back(&dummyParams);
	texcoordStack.push_back(&dummyParams);
	normalStack.push_back(&dummyParams);

	// default lights
	lightSetIndexStack.push_back(0);

	// allow up to this many simultaneous lights
	maxLights = 8;
}

SceneVisitorRender::~SceneVisitorRender()
{
	delete resetter;
}

bool					SceneVisitorRender::traverse(SceneNode* node)
{
	TimeKeeper t(TimeKeeper::getCurrent());

	// first matrix is the identity
	matrixList.push_back(modelXFormStack.back());

	// first light set is empty
	LightSet noLights;
	noLights.size = 0;
	lightSetList.push_back(noLights);

	// bounding box isn't set
	boundingBoxCull = kCullOld;

	// view frustum isn't computed
	frustumDirty = true;

	// reuse scratch bundles
	resetter->traverse(node);

	// collect primitives
	const bool result = SceneVisitorBaseRender::traverse(node);

	// sort 'em
	sort();

	// and render
	draw();

	// done with collected primitives
	jobs.clear();
	matrixList.clear();
	lightList.clear();
	lightSetList.clear();

	// flush pipeline again so we get the time required to actually
	// draw everything, not just to shove it down the pipeline.
	if (isDebugging())
		glFinish();

	// record the elapsed time
	getInstr()->time = TimeKeeper::getCurrent() - t;

	return result;
}

bool					SceneVisitorRender::jobLess(const Job& a, const Job& b)
{
	// always separate passes
	if (a.compare->pass != b.compare->pass)
		return (a.compare->pass < b.compare->pass);

	// non-blended stuff goes first
	const bool aOpaque = (a.compare->blendingSrc == GState::kOne &&
						   a.compare->blendingDst == GState::kZero);
	const bool bOpaque = (b.compare->blendingSrc == GState::kOne &&
						   b.compare->blendingDst == GState::kZero);
	if (aOpaque != bOpaque)
		return aOpaque;

	// blended and non-blended primitives sort differently
	if (aOpaque) {
		// avoid changing lights
		if (a.lightSet != b.lightSet)
			return (a.lightSet < b.lightSet);

		// avoid changing textures
		if (a.compare->texture != b.compare->texture)
			return (a.compare->texture < b.compare->texture);

		// draw roughly front-to-back
		if (a.depth != b.depth)
			return (a.depth < b.depth);
	}

	// blended
	else {
		// draw roughly back-to-front
		if (a.depth != b.depth)
			return (a.depth > b.depth);

		// avoid changing lights
		if (a.lightSet != b.lightSet)
			return (a.lightSet < b.lightSet);

		// avoid changing textures
		if (a.compare->texture != b.compare->texture)
			return (a.compare->texture < b.compare->texture);
	}

	// equal
	return false;
}

void					SceneVisitorRender::sort()
{
	std::sort(jobs.begin(), jobs.end(), &jobLess);
}

#include <stdio.h>
void					SceneVisitorRender::draw()
{
	static const GLenum typeMap[] = {
		GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP,
		GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN,
		GL_QUADS };

	// prep rendering state
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glEnableClientState(GL_VERTEX_ARRAY);
	// FIXME -- init() should become unnecessary with consistent use of gstates
	OpenGLGState::init();
	setStipple(1.0f);

	// initialize indices
	unsigned int xformView       = 0xffffffff;
	unsigned int xformProjection = 0xffffffff;
	unsigned int xformTexture    = 0xffffffff;
	unsigned int lightSetIndex   = 0xffffffff;
	unsigned int enabledLights   = 0;

	// initialize pointers
	const SceneNodeVFFloat* pStipple  = NULL;
	const SceneNodeVFFloat* pColor    = NULL;
	const SceneNodeVFFloat* pTexcoord = NULL;
	const SceneNodeVFFloat* pNormal   = NULL;
	const SceneNodeVFFloat* pVertex   = NULL;

	// should we use lighting?
	const bool lighting = BZDB->isTrue(nameLighting);

	// what's front facing?
	bool frontCCW = true;

	// draw
	const unsigned int n = jobs.size();
	for (unsigned int i = 0; i < n; ++i) {
		// get job
		const Job& job = jobs[i];

		// set lights
		if (lighting && lightSetIndex != job.lightSet) {
			// lights are pre-transformed
			glLoadIdentity();

			// set light parameters
			// FIXME -- should only set changed lights
			const LightSet& lightSet = lightSetList[job.lightSet];
			const unsigned int numLights = lightSet.size;
			unsigned int j;
			for (j = 0; j < numLights; ++j) {
				const LightInfo& light = lightList[lightSet.index[j]];
				glLightfv(GL_LIGHT0 + j, GL_AMBIENT,  light.ambient);
				glLightfv(GL_LIGHT0 + j, GL_DIFFUSE,  light.diffuse);
				glLightfv(GL_LIGHT0 + j, GL_SPECULAR, light.specular);
				glLightfv(GL_LIGHT0 + j, GL_POSITION, light.position);
				glLightfv(GL_LIGHT0 + j, GL_SPOT_DIRECTION, light.spotDirection);
				glLightf(GL_LIGHT0 + j, GL_SPOT_EXPONENT, light.spotExponent);
				glLightf(GL_LIGHT0 + j, GL_SPOT_CUTOFF, light.spotCutoff);
				glLightf(GL_LIGHT0 + j, GL_CONSTANT_ATTENUATION, light.attenuation[0]);
				glLightf(GL_LIGHT0 + j, GL_LINEAR_ATTENUATION, light.attenuation[1]);
				glLightf(GL_LIGHT0 + j, GL_QUADRATIC_ATTENUATION, light.attenuation[2]);
			}

			if (numLights < enabledLights)
				for (j = numLights; j < enabledLights; ++j)
					glDisable(GL_LIGHT0 + j);
			else if (numLights > enabledLights)
				for (j = enabledLights; j < numLights; ++j)
					glEnable(GL_LIGHT0 + j);
			enabledLights = numLights;
			lightSetIndex = job.lightSet;
		}

		// set transformations
		bool matrixModeChanged = false;
		if (job.xformTexture != xformTexture) {
			xformTexture = job.xformTexture;
			glMatrixMode(GL_TEXTURE);
			glLoadMatrixf(matrixList[xformTexture].get());
			matrixModeChanged = true;
			++getInstr()->nTransforms;
		}
		if (job.xformProjection != xformProjection) {
			xformProjection = job.xformProjection;
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(matrixList[xformProjection].get());
			matrixModeChanged = true;
			++getInstr()->nTransforms;

			// should compute the sign of the adjoint but we're
			// expecting a very specific kind of matrix so we
			// can take a shortcut.
			const float* m = matrixList[xformProjection].get();
			frontCCW = ((m[0] < 0.0f) == (m[5] < 0.0f));
			glFrontFace(frontCCW ? GL_CCW : GL_CW);
		}
		if (matrixModeChanged)
			glMatrixMode(GL_MODELVIEW);
		if (job.xformView != xformView) {
			xformView = job.xformView;
			glLoadMatrixf(matrixList[xformView].get());
			++getInstr()->nTransforms;
		}

		// prepare for primitive
		unsigned int numItems;
		job.gstate.setState();
		if (pStipple != job.stipple) {
			pStipple = job.stipple;
			if (pStipple->getNum() == 0) {
				setStipple(1.0f);
			}
			else {
				setStipple(pStipple->get()[0]);
			}
		}
		if (pColor != job.color) {
			pColor = job.color;
			numItems = pColor->getNum();
			if (numItems < 4) {
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				glDisableClientState(GL_COLOR_ARRAY);
			}
			else if (numItems == 4) {
				glColor4fv(pColor->get());
				glDisableClientState(GL_COLOR_ARRAY);
			}
			else {
				glColorPointer(4, GL_FLOAT, 0, pColor->get());
				glEnableClientState(GL_COLOR_ARRAY);
			}
		}
		if (pTexcoord != job.texcoord) {
			pTexcoord = job.texcoord;
			numItems = pTexcoord->getNum();
			if (numItems < 2) {
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}
			else {
				glTexCoordPointer(2, GL_FLOAT, 0, pTexcoord->get());
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			}
		}
		if (lighting && pNormal != job.normal) {
			pNormal = job.normal;
			numItems = pNormal->getNum();
			if (numItems < 3) {
				glDisableClientState(GL_NORMAL_ARRAY);
				glDisable(GL_COLOR_MATERIAL);
				glDisable(GL_LIGHTING);
			}
			else if (numItems == 3) {
				glNormal3fv(pNormal->get());
				glDisableClientState(GL_NORMAL_ARRAY);
				glEnable(GL_COLOR_MATERIAL);
				glEnable(GL_LIGHTING);
			}
			else {
				glNormalPointer(GL_FLOAT, 0, pNormal->get());
				glEnableClientState(GL_NORMAL_ARRAY);
				glEnable(GL_COLOR_MATERIAL);
				glEnable(GL_LIGHTING);
			}
		}
		if (pVertex != job.vertex) {
			pVertex = job.vertex;
			glVertexPointer(3, GL_FLOAT, 0, pVertex->get());
		}

		// draw whatever
		unsigned int num;
		unsigned int type;
		unsigned int *index;
		if(job.primitive != NULL) {
			num   = job.primitive->index.getNum();
			type  = job.primitive->type.get();
			index = (unsigned int *) job.primitive->index.get();
		}
		else if(job.particle != NULL) {
			num   = job.particle->index.getNum();
			type  = job.particle->type;
			index = (unsigned int *) job.particle->index.get();
		}
		else {
			num = 0;
			type = 0;
			index = NULL;
		}

		glDrawElements(typeMap[type], num, GL_UNSIGNED_INT, index);
	}

	// restore state
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	setStipple(1.0f);
	if (lighting) {
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_LIGHTING);
		for (unsigned int j = 0; j < enabledLights; ++j)
			glDisable(GL_LIGHT0 + j);
	}
	if (!frontCCW)
		glFrontFace(GL_CCW);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

bool					SceneVisitorRender::visit(SceneNodeAnimate* n)
{
	const std::string& dst = n->dst.get();
	getParams().pushFloat(dst, n->get(getParams()));
	const bool result = descend(n);
	getParams().popFloat(dst);
	return result;
}

bool					SceneVisitorRender::visit(SceneNodeParameters* n)
{
	n->push(getParams());
	const bool result = descend(n);
	n->pop(getParams());
	return result;
}

bool					SceneVisitorRender::visit(SceneNodeSelector* n)
{
	// FIXME -- what do we pass to a selector?  need enough to do
	// LOD based on projected area and z-depth.
	getParams().pushInt(nameMask, n->get(modelXFormStack.back(),
								projectionXFormStack.back(),
								getParams()));
	const bool result = descend(n);
	getParams().popInt(nameMask);
	return result;
}

bool					SceneVisitorRender::visit(SceneNodeGState* n)
{
	gstateStack.push_back(n->get());
	const bool result = descend(n);
	gstateStack.pop_back();
	return result;
}

bool					SceneVisitorRender::visit(SceneNodeBaseTransform* n)
{
	XFormStack* stack;
	IndexStack* indexStack;

	// what kind of matrix?
	switch (n->type.get()) {
		case SceneNodeBaseTransform::ModelView:
			stack      = &modelXFormStack;
			indexStack = &modelXFormIndexStack;

			// bounding box must be retransformed
			boundingBoxCull = kCullDirty;
			break;

		case SceneNodeBaseTransform::Projection:
			stack      = &projectionXFormStack;
			indexStack = &projectionXFormIndexStack;

			// view frustum must be recomputed
			frustumDirty = true;
			break;

		case SceneNodeBaseTransform::Texture:
			stack      = &textureXFormStack;
			indexStack = &textureXFormIndexStack;
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
	indexStack->push_back(0xffffffff);

	// let node modify new matrix
	n->get(stack->back(), getParams());

	// descend
	const bool result = descend(n);

	// pop stack
	stack->pop_back();
	indexStack->pop_back();

	return result;
}

bool					SceneVisitorRender::visit(SceneNodeGeometry* n)
{
	// use next scratch bundle
	n->nextScratchBundle();

	// calculate geometry
	n->compute(getParams());
	n->setBundle(SceneNodeGeometry::kComputedIndex);

	// check which arrays are used
	const bool hasStipple = (n->stipple->getNum()    > 0);
	const bool hasColor   = (n->color->getNum()    > 0);
	const bool hasTexture = (n->texcoord->getNum() > 0);
	const bool hasNormal  = (n->normal->getNum()   > 0);
	const bool hasVertex  = (n->vertex->getNum()   > 0);

	// push state
	if (hasStipple)
		stippleStack.push_back(n->stipple);
	if (hasColor)
		colorStack.push_back(n->color);
	if (hasTexture)
		texcoordStack.push_back(n->texcoord);
	if (hasNormal)
		normalStack.push_back(n->normal);
	if (hasVertex) {
		vertexStack.push_back(n->vertex);
		geometryStack.push_back(n);
		boundingBoxCull = kCullOld;
	}

	// descend
	const bool result = descend(n);

	// pop state
	if (hasStipple)
		stippleStack.pop_back();
	if (hasColor)
		colorStack.pop_back();
	if (hasTexture)
		texcoordStack.pop_back();
	if (hasNormal)
		normalStack.pop_back();
	if (hasVertex) {
		vertexStack.pop_back();
		geometryStack.pop_back();
		boundingBoxCull = kCullOld;
	}

	return result;
}

bool					SceneVisitorRender::visit(SceneNodePrimitive* n)
{
	// skip primitive if there are no vertices
	if (vertexStack.size() == 0 || vertexStack.back()->getNum() < 3)
		return true;
	assert(geometryStack.size() > 0);

	// handle bounding box
	switch (boundingBoxCull) {
		case kCullOld:
			geometryStack.back()->getBoundingBox(&boundingBox);
			// fall through

		case kCullDirty: {
			if (frustumDirty) {
				computeFrustum();
			}
			BoundingBox xformBoundingBox(boundingBox);
			xformBoundingBox.transform(modelXFormStack.back());
			xformBoundingBox.get(aaBoundingBox[0], aaBoundingBox[1]);
			if (isCulled(aaBoundingBox)) {
				boundingBoxCull = kCullYes;
				return true;
			}
			boundingBoxCull = kCullNo;
			break;
		}

		case kCullYes:
			return true;

		case kCullNo:
			break;
	}

	// save transforms that haven't been added to a list yet
	if (modelXFormIndexStack.back() == 0xffffffff) {
		modelXFormIndexStack.back() = matrixList.size();
		matrixList.push_back(modelXFormStack.back());
	}
	if (projectionXFormIndexStack.back() == 0xffffffff) {
		projectionXFormIndexStack.back() = matrixList.size();
		matrixList.push_back(projectionXFormStack.back());
	}
	if (textureXFormIndexStack.back() == 0xffffffff) {
		textureXFormIndexStack.back() = matrixList.size();
		matrixList.push_back(textureXFormStack.back());
	}

	// save lights that haven't been added to the list yet
	unsigned int numLights = lightIndexStack.size();
	if (numLights > maxLights)
		numLights = maxLights;
	unsigned int i = numLights;
	while (i-- > 0 && lightIndexStack[i] == 0xffffffff) {
		lightIndexStack[i] = lightList.size();
		lightList.push_back(lightStack[i]);
	}

	// save light set if it hasn't been added to the list yet
	if (lightSetIndexStack.back() == 0xffffffff) {
		// create set
		LightSet lights;
		lights.size = 0;
		for (i = 0; i < numLights; ++i)
			lights.index[lights.size++] = lightIndexStack[i];

		// add to list
		lightSetIndexStack.back() = lightSetList.size();
		lightSetList.push_back(lights);
	}

	// prepare job
	Job job(gstateStack.back());
	job.primitive       = n;
	job.particle        = NULL;
	job.depth           = -aaBoundingBox[0][2];
	job.gstate          = gstateStack.back();
	job.compare         = job.gstate.getState();
	job.stipple         = stippleStack.back();
	job.color           = colorStack.back();
	job.texcoord        = texcoordStack.back();
	job.normal          = normalStack.back();
	job.vertex          = vertexStack.back();
	job.xformView       = modelXFormIndexStack.back();
	job.xformProjection = projectionXFormIndexStack.back();
	job.xformTexture    = textureXFormIndexStack.back();
	job.lightSet        = lightSetIndexStack.back();

	// add job
	jobs.push_back(job);

	// count
	const unsigned int num  = n->index.getNum();
	switch (n->type.get()) {
		case SceneNodePrimitive::Points:
			getInstr()->nPoints += num;
			break;

		case SceneNodePrimitive::Lines:
			getInstr()->nLines += (num >> 1);
			break;

		case SceneNodePrimitive::LineStrip:
			getInstr()->nLines += num - 1;
			break;

		case SceneNodePrimitive::LineLoop:
			getInstr()->nLines += num;
			break;

		case SceneNodePrimitive::Triangles:
			getInstr()->nTriangles += num / 3;
			break;

		case SceneNodePrimitive::TriangleStrip:
			getInstr()->nTriangles += num - 2;
			break;

		case SceneNodePrimitive::TriangleFan:
			getInstr()->nTriangles += num - 2;
			break;
	}
	++getInstr()->nNodes;

	return true;
}

bool					SceneVisitorRender::visit(SceneNodeParticleSystem* n)
{
	// skip if the emitter isn't going
	if(n->isStopped())
		return true;

	n->type = 7; // GL_QUADS

	// handle bounding box
	switch(boundingBoxCull) {
		case kCullOld:
			n->getBoundingBox(&boundingBox);
			// fall through

		case kCullDirty: {
			if(frustumDirty) {
				computeFrustum();
			}
			BoundingBox xformBoundingBox(boundingBox);
			xformBoundingBox.transform(modelXFormStack.back());
			xformBoundingBox.get(aaBoundingBox[0], aaBoundingBox[1]);
			if(isCulled(aaBoundingBox)) {
				boundingBoxCull = kCullYes;
				return true;
			}
			boundingBoxCull = kCullNo;
			break;
		}

		case kCullYes:
			return true;

		case kCullNo:
			break;
	}

	// save transforms that haven't been added to a list yet
	if (modelXFormIndexStack.back() == 0xffffffff) {
		modelXFormIndexStack.back() = matrixList.size();
		matrixList.push_back(modelXFormStack.back());
	}
	if (projectionXFormIndexStack.back() == 0xffffffff) {
		projectionXFormIndexStack.back() = matrixList.size();
		matrixList.push_back(projectionXFormStack.back());
	}
	if (textureXFormIndexStack.back() == 0xffffffff) {
		textureXFormIndexStack.back() = matrixList.size();
		matrixList.push_back(textureXFormStack.back());
	}

	// save lights that haven't been added to the list yet
	unsigned int numLights = lightIndexStack.size();
	if (numLights > maxLights)
		numLights = maxLights;
	unsigned int i = numLights;
	while (i-- > 0 && lightIndexStack[i] == 0xffffffff) {
		lightIndexStack[i] = lightList.size();
		lightList.push_back(lightStack[i]);
	}

	// save light set if it hasn't been added to the list yet
	if (lightSetIndexStack.back() == 0xffffffff) {
		// create set
		LightSet lights;
		lights.size = 0;
		for (i = 0; i < numLights; ++i)
			lights.index[lights.size++] = lightIndexStack[i];

		// add to list
		lightSetIndexStack.back() = lightSetList.size();
		lightSetList.push_back(lights);
	}

	// update our particle system
	Matrix matrix;
	matrix.set(ViewFrustum::getTransform());
	matrix.mult(modelXFormStack.back());
	matrix.inverse();
	n->update(getParams().getFloat("time"), matrix);

	Job job(gstateStack.back());
	job.primitive		= NULL;
	job.particle		= n;
	job.depth		= -aaBoundingBox[0][2];
	job.gstate		= gstateStack.back();
	job.compare		= job.gstate.getState();
	job.stipple		= stippleStack.back();
	job.color		= &(n->colors);
	job.texcoord		= &(n->texcoords);
	job.normal		= normalStack.back();
	job.vertex		= &(n->verteces);
	job.xformView		= modelXFormIndexStack.back();
	job.xformProjection	= projectionXFormIndexStack.back();
	job.xformTexture	= textureXFormIndexStack.back();
	job.lightSet		= lightSetIndexStack.back();

	jobs.push_back(job);

	// count
	getInstr()->nQuads += n->particles.size();

	return true;
}

bool					SceneVisitorRender::visit(SceneNodeLight* n)
{
	n->compute(getParams());

	// transform light position by current modelview matrix
	float position[4];
	modelXFormStack.back().transform4(position, n->getPosition());

	// save light state
	LightInfo light;
	memcpy(light.ambient,       n->getAmbientColor(),  sizeof(light.ambient));
	memcpy(light.diffuse,       n->getDiffuseColor(),  sizeof(light.diffuse));
	memcpy(light.specular,      n->getSpecularColor(), sizeof(light.specular));
	memcpy(light.position,      position,              sizeof(light.position));
	memcpy(light.spotDirection, n->getSpotDirection(), sizeof(light.spotDirection));
	light.spotExponent        = n->getSpotExponent();
	light.spotCutoff          = n->getSpotCutoff();
	memcpy(light.attenuation,   n->getAttenuation(),   sizeof(light.attenuation));
	lightStack.push_back(light);
	lightIndexStack.push_back(0xffffffff);
	lightSetIndexStack.push_back(0xffffffff);

	// continue
	const bool result = descend(n);

	// remove light
	lightStack.pop_back();
	lightIndexStack.pop_back();
	lightSetIndexStack.pop_back();

	return result;
}

void					SceneVisitorRender::computeFrustum()
{
	// clip coordinates to transform
	static const float s_lbn[4] = { -1.0f, -1.0f, -1.0f, 1.0f };
	static const float s_lbf[4] = { -1.0f, -1.0f,  1.0f, 1.0f };
	static const float s_rtn[4] = {  1.0f,  1.0f, -1.0f, 1.0f };
	static const float s_rtf[4] = {  1.0f,  1.0f,  1.0f, 1.0f };
	static const float s_xv[3]  = {  1.0f,  0.0f,  0.0f };
	static const float s_yv[3]  = {  0.0f,  1.0f,  0.0f };

	// compute inverse of projection matrix.  note if the matrix is
	// inside-out from the expected and invert it.
	// FIXME -- this isn't a general purpose solution
	Matrix matrix(projectionXFormStack.back());
	float* p = matrix.get();
	const bool xFlip = (p[0] < 0.0f);
	const bool yFlip = (p[5] < 0.0f);
	if (xFlip)
		p[0] = -p[0];
	if (yFlip)
		p[5] = -p[5];
	matrix.inverse();

	// transform points from clip coordinates to eye coordinates
	float lbn[4], lbf[4], rtn[4], rtf[4];
	matrix.transform4(lbn, s_lbn);
	matrix.transform4(lbf, s_lbf);
	matrix.transform4(rtn, s_rtn);
	matrix.transform4(rtf, s_rtf);
	lbn[3] = 1.0f / lbn[3];
	lbf[3] = 1.0f / lbf[3];
	rtn[3] = 1.0f / rtn[3];
	rtf[3] = 1.0f / rtf[3];
	lbn[0] *= lbn[3];
	lbn[1] *= lbn[3];
	lbn[2] *= lbn[3];
	lbf[0] *= lbf[3];
	lbf[1] *= lbf[3];
	lbf[2] *= lbf[3];
	rtn[0] *= rtn[3];
	rtn[1] *= rtn[3];
	rtn[2] *= rtn[3];
	rtf[0] *= rtf[3];
	rtf[1] *= rtf[3];
	rtf[2] *= rtf[3];

	float lbv[3], rtv[3];
	lbv[0] = lbf[0] - lbn[0];
	lbv[1] = lbf[1] - lbn[1];
	lbv[2] = lbf[2] - lbn[2];
	rtv[0] = rtf[0] - rtn[0];
	rtv[1] = rtf[1] - rtn[1];
	rtv[2] = rtf[2] - rtn[2];

	// compute frustum planes
	setPlane(0, lbn, s_yv, s_xv);
	setPlane(1, rtf, s_xv, s_yv);
	setPlane(2, lbn,  lbv, s_yv);
	setPlane(3, rtf, s_yv,  rtv);
	setPlane(4, lbn, s_xv,  lbv);
	setPlane(5, rtf,  rtv, s_xv);

	// frustum is clean
	frustumDirty = false;
}

void					SceneVisitorRender::setPlane(
								unsigned int index,
								const float* p,
								const float* v1, const float* v2)
{
	float* plane = frustum[index];

	// compute normal
	plane[0] = v1[1] * v2[2] - v1[2] * v2[1];
	plane[1] = v1[2] * v2[0] - v1[0] * v2[2];
	plane[2] = v1[0] * v2[1] - v1[1] * v2[0];

	// finish plane equation
	plane[3] = -(p[0] * plane[0] + p[1] * plane[1] + p[2] * plane[2]);

	// compute plane's normal direction.  for each component, the
	// direction is 1 if the component is >= 0.0 and 0 otherwise.
	// 0 corresponds to the more negative side of the axis-aligned
	// bounding box's component and 1 to the more positive side.
	// these are the sides we want to compare against the plane
	// in isCulled().
	//
	// for example, if the plane's normal points in the direction
	// +1,+1,+1 then we can see if an axis aligned bounding box is
	// entirely to the other side of the plane by testing the point
	// in the box with the most positive x, y, and z components
	// against the plane.  if the point is in the negative half-
	// space then the entire box must be too.
	unsigned int* dir = frustumDir[index];
	dir[0] = (plane[0] >= 0.0f) ? 1 : 0;
	dir[1] = (plane[1] >= 0.0f) ? 1 : 0;
	dir[2] = (plane[2] >= 0.0f) ? 1 : 0;
}

inline
bool					SceneVisitorRender::insidePlane(const float* plane,
								float x, float y, float z) const
{
	return (plane[0] * x + plane[1] * y + plane[2] * z + plane[3] >= 0.0f);
}

bool					SceneVisitorRender::isCulled(
								const float aabb[2][3]) const
{
	// compare with frustum
	if (!insidePlane(frustum[0], aabb[frustumDir[0][0]][0],
								aabb[frustumDir[0][1]][1],
								aabb[frustumDir[0][2]][2]))
		return true;
	if (!insidePlane(frustum[2], aabb[frustumDir[2][0]][0],
								aabb[frustumDir[2][1]][1],
								aabb[frustumDir[2][2]][2]))
		return true;
	if (!insidePlane(frustum[3], aabb[frustumDir[3][0]][0],
								aabb[frustumDir[3][1]][1],
								aabb[frustumDir[3][2]][2]))
		return true;
	if (!insidePlane(frustum[4], aabb[frustumDir[4][0]][0],
								aabb[frustumDir[4][1]][1],
								aabb[frustumDir[4][2]][2]))
		return true;
	if (!insidePlane(frustum[5], aabb[frustumDir[5][0]][0],
								aabb[frustumDir[5][1]][1],
								aabb[frustumDir[5][2]][2]))
		return true;
/* skip far plane
	if (!insidePlane(frustum[1], aabb[frustumDir[1][0]][0],
								aabb[frustumDir[1][1]][1],
								aabb[frustumDir[1][2]][2]))
		return true;
*/

	// inside frustum
	return false;
}
