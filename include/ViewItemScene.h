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

#ifndef BZF_VIEWITEMSCENE_H
#define BZF_VIEWITEMSCENE_H

#include "View.h"
#include "ViewFrustum.h"
#include "SceneVisitorRender.h"

class SceneNodeMatrixTransform;

class ViewItemScene : public View {
public:
	ViewItemScene();

	void				setOffset(float x, float y, float z);
	void				setTurn(float fixed, float scaled);
	void				setTilt(float fixed, float scaled);
	void				setFOVScale(float fovxScale, float fovyScale);

protected:
	virtual ~ViewItemScene();

	// get the render visitor
	SceneVisitorRender&	getRenderer();

	// get the scene to draw
	virtual SceneNode*	getScene() = 0;

	// get the current view frustum
	virtual void		getView(ViewFrustum&) = 0;

	// View overrides
	virtual bool		onPreRender(float x, float y, float w, float h);
	virtual void		onPostRender(float x, float y, float w, float h);

private:
	float				xOffset, yOffset, zDepth;
	float				zRotateFixed, zRotateScaled;
	float				yRotateFixed, yRotateScaled;
	float				fovxScale, fovyScale;

	SceneNodeMatrixTransform*	sceneProj;
	SceneNodeMatrixTransform*	sceneXForm;

	SceneVisitorRender	render;
};

class ViewItemSceneReader : public ViewTagReader {
public:
	ViewItemSceneReader();
	virtual ~ViewItemSceneReader();

	// ViewItemReader overrides
	virtual ViewTagReader* clone() const = 0;
	virtual View*		open(XMLTree::iterator);

protected:
	// override to return a new ViewItemScene subclass
	virtual ViewItemScene* createItem() = 0;

private:
	ViewItemScene*		item;
};

#endif
