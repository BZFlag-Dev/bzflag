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

#include "ViewItemScene.h"
#include "SceneVisitorRender.h"
#include "SceneNodeMatrixTransform.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

//
// ViewItemScene
//

ViewItemScene::ViewItemScene() : xOffset(0.0f), yOffset(0.0f), zDepth(0.0f),
								zRotateFixed(0.0f), zRotateScaled(0.0f),
								yRotateFixed(0.0f), yRotateScaled(0.0f),
								fovScale(1.0f)
{
	// create nodes
	sceneProj  = new SceneNodeMatrixTransform;	// player projection
	sceneXForm = new SceneNodeMatrixTransform;	// player view

	// set transform types
	sceneProj->type.set(SceneNodeTransform::Projection);

	// assemble
	sceneProj->pushChild(sceneXForm);
	sceneXForm->unref();
}

ViewItemScene::~ViewItemScene()
{
	sceneProj->unref();
}

void					ViewItemScene::setOffset(float x, float y, float z)
{
	xOffset = x;
	yOffset = y;
	zDepth  = z;
}

void					ViewItemScene::setTurn(float fixed, float scaled)
{
	zRotateFixed  = fixed;
	zRotateScaled = scaled;
}

void					ViewItemScene::setTilt(float fixed, float scaled)
{
	yRotateFixed  = fixed;
	yRotateScaled = scaled;
}

void					ViewItemScene::setFOVScale(float _fovScale)
{
	fovScale = _fovScale;
}

SceneVisitorRender&		ViewItemScene::getRenderer()
{
	return render;
}

bool					ViewItemScene::onPreRender(
								float, float, float w, float h)
{
	// get scene
	SceneNode* scene = getScene();
	if (scene == NULL)
		return false;

	// get the current view
	ViewFrustum view;
	getView(view);

	// get fov
	float fov = view.getFOVx();

	// compute angle sines and cosines
	const float zAngle = -(zRotateFixed + fov * zRotateScaled) * M_PI / 180.0f;
	const float yAngle = -(yRotateFixed + fov * yRotateScaled) * M_PI / 180.0f;
	const float zc     = cosf(zAngle);
	const float zs     = sinf(zAngle);
	const float yc     = cosf(yAngle);
	const float ys     = sinf(yAngle);

	// adjust field of view
	fov *= fovScale;

	// adjust focus point based on our parameters
	const float* eye = view.getEye();
	const float* dir = view.getDirection();
	float focus[3];
/*
	focus[0] = eye[0] + yc * zc * dir[0] - yc * zs * dir[1] - ys * dir[2];
	focus[1] = eye[1] +      zs * dir[0] +      zc * dir[1]              ;
	focus[2] = eye[2] - ys * zc * dir[0] + ys * zs * dir[1] + yc * dir[2];
*/
	focus[0] = eye[0] + yc * zc * dir[0] - zs * dir[1] + ys * zc * dir[2];
	focus[1] = eye[1] + yc * zs * dir[0] + zc * dir[1] + ys * zs * dir[2];
	focus[2] = eye[2] - ys *      dir[0]               + yc *      dir[2];

	// adjust frustum
	view.setProjection(fov, view.getNear(), view.getFar(),
								static_cast<int>(w), static_cast<int>(h));
	if (zDepth > 0.0f)
		view.setOffset(xOffset, zDepth);
	view.setView(eye, focus);

	// set the projection and viewing transform
	sceneProj->matrix.set(view.getProjectionMatrix(), 16);
	sceneXForm->matrix.set(view.getViewMatrix(), 16);

	// push scene under our transformations
	sceneXForm->pushChild(scene);
	scene->unref();

	return true;
}

void					ViewItemScene::onPostRender(
								float, float, float w, float h)
{
	// draw
	render.setArea(w * h);
	render.traverse(sceneProj);
	sceneXForm->popChild();
}


//
// ViewItemSceneReader
//

ViewItemSceneReader::ViewItemSceneReader() : item(NULL)
{
	// do nothing
}

ViewItemSceneReader::~ViewItemSceneReader()
{
	if (item != NULL)
		item->unref();
}

View*					ViewItemSceneReader::open(
								const ConfigReader::Values& values)
{
	assert(item == NULL);

	float x = 0.0f, y = 0.0f, z = 0.0f;
	float yFixed = 0.0f, yScaled = 0.0f;
	float zFixed = 0.0f, zScaled = 0.0f;
	float fov = 1.0f;

	ConfigReader::Values::const_iterator index;
	index = values.find("x");
	if (index != values.end())
		x = atof(index->second.c_str());
	index = values.find("y");
	if (index != values.end())
		y = atof(index->second.c_str());
	index = values.find("z");
	if (index != values.end())
		z = atof(index->second.c_str());
	index = values.find("fov");
	if (index != values.end())
		fov = atof(index->second.c_str());
	index = values.find("theta");
	if (index != values.end()) {
		float value;
		char type;
		switch (sscanf(index->second.c_str(), "%f%c", &value, &type)) {
			case 1:
				zFixed = value;
				break;

			case 2:
				if (type == '%')
					zScaled = 0.01f * value;
				break;
		}
	}
	index = values.find("phi");
	if (index != values.end()) {
		float value;
		char type;
		switch (sscanf(index->second.c_str(), "%f%c", &value, &type)) {
			case 1:
				yFixed = value;
				break;

			case 2:
				if (type == '%')
					yScaled = 0.01f * value;
				break;
		}
	}

	item = createItem();
	if (z > 0.0f)
		item->setOffset(x, y, z);
	item->setTurn(zFixed, zScaled);
	item->setTilt(yFixed, yScaled);
	item->setFOVScale(fov);

	return item;
}

void					ViewItemSceneReader::close()
{
	assert(item != NULL);
	item = NULL;
}
