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

#include "ViewItemPlayerScene.h"
#include "SceneManager.h"
#include "bzfgl.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

//
// ViewItemPlayerScene
//

ViewItemPlayerScene::ViewItemPlayerScene()
{
	// prep gstate
	OpenGLGStateBuilder builder;
	builder.setBlending(GState::kSrcAlpha, GState::kOneMinusSrcAlpha);
	builder.setDepthFunc(GState::kAlways);
	builder.setDepthMask(false);
	fadeGState = builder.getState();
}

ViewItemPlayerScene::~ViewItemPlayerScene()
{
	// do nothing
}

bool					ViewItemPlayerScene::onPreRender(
								float x, float y, float w, float h)
{
	// see if there's a scene
	if (getScene() == NULL)
		return false;

	// clear depth buffer
	glClear(GL_DEPTH_BUFFER_BIT);

	// prep
	return ViewItemScene::onPreRender(x, y, w, h);
}

void					ViewItemPlayerScene::onPostRender(
								float x, float y, float w, float h)
{
	// draw
	SCENEMGR->setParams(getRenderer().getParams());
	ViewItemScene::onPostRender(x, y, w, h);

	// draw fade rectangle
	const float* fade = SCENEMGR->getFade();
	if (fade[3] != 0.0f) {
		fadeGState.setState();
		glColor4fv(fade);
		glRectf(0.0f, 0.0f, w, h);
	}
}

SceneNode*				ViewItemPlayerScene::getScene()
{
	return SCENEMGR->getScene();
}

void					ViewItemPlayerScene::getView(ViewFrustum& view)
{
	view = SCENEMGR->getView();
}


//
// ViewItemPlayerSceneReader
//

ViewItemPlayerSceneReader::ViewItemPlayerSceneReader()
{
	// do nothing
}

ViewItemPlayerSceneReader::~ViewItemPlayerSceneReader()
{
	// do nothing
}

ViewTagReader* 			ViewItemPlayerSceneReader::clone() const
{
	return new ViewItemPlayerSceneReader;
}

ViewItemScene*			ViewItemPlayerSceneReader::createItem()
{
	return new ViewItemPlayerScene;
}
