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

#ifndef BZF_VIEWITEMMODEL_H
#define BZF_VIEWITEMMODEL_H

#include "View.h"
#include "SceneVisitorSimpleRender.h"

class SceneNodeMatrixTransform;

class ViewItemModel : public View {
public:
	ViewItemModel();

	void				setSceneNode(SceneNode*);

	// if oneToOnePixel is true then the projection matrix maps
	// coordinates from 0,0 to width,height, otherwise it maps
	// coordinates from 0,0 to 1,1.  default is the latter.
	void				setPixelProjection(bool oneToOnePixel);


protected:
	virtual ~ViewItemModel();

	// View overrides
	virtual bool		onPreRender(float x, float y, float w, float h);
	virtual void		onPostRender(float x, float y, float w, float h);

private:
	SceneNodeMatrixTransform*	projection;
	SceneVisitorSimpleRender	render;
	bool				pixelProjection;
};

class ViewItemModelReader : public ViewTagReader {
public:
	ViewItemModelReader();
	virtual ~ViewItemModelReader();

	// ViewItemReader overrides
	virtual ViewTagReader* clone() const;
	virtual View*		open(const ConfigReader::Values& values);
	virtual void		close();

private:
	ViewItemModel*		item;
};

#endif
