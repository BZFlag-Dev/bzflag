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

#ifndef BZF_VIEWITEMPLAYERSCENE_H
#define BZF_VIEWITEMPLAYERSCENE_H

#include "ViewItemScene.h"
#include "OpenGLGState.h"

class ViewItemPlayerScene : public ViewItemScene {
public:
	ViewItemPlayerScene();

protected:
	virtual ~ViewItemPlayerScene();

	// View overrides
	virtual bool		onPreRender(float x, float y, float w, float h);
	virtual void		onPostRender(float x, float y, float w, float h);

	// ViewItemScene overrides
	virtual SceneNode*	getScene();
	virtual void		getView(ViewFrustum&);

private:
	OpenGLGState		fadeGState;
	OpenGLGState		fadeStippleGState;
};

class ViewItemPlayerSceneReader : public ViewItemSceneReader {
public:
	ViewItemPlayerSceneReader();
	virtual ~ViewItemPlayerSceneReader();

	// ViewItemReader overrides
	virtual ViewTagReader* clone() const;

protected:
	// ViewTagSceneReader overrides
	virtual ViewItemScene* createItem();
};

#endif
// ex: shiftwidth=4 tabstop=4
