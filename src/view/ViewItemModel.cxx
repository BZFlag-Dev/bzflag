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

#include "ViewItemModel.h"
#include "ViewReader.h"
#include "FileManager.h"
#include "ErrorHandler.h"
#include "SceneNodeMatrixTransform.h"
#include "SceneReader.h"
#include <assert.h>
#include <iostream>

//
// ViewItemModel
//

ViewItemModel::ViewItemModel() : pixelProjection(false)
{
	projection = new SceneNodeMatrixTransform;
	projection->type.set(SceneNodeTransform::Projection);
}

ViewItemModel::~ViewItemModel()
{
	projection->unref();
}

void					ViewItemModel::setSceneNode(SceneNode* node)
{
	projection->clearChildren();
	projection->pushChild(node);
}

void					ViewItemModel::setPixelProjection(
							bool _pixelProjection)
{
	pixelProjection = _pixelProjection;
}

bool					ViewItemModel::onPreRender(
								float, float, float w, float h)
{
	// set the projection
	static const float n = -1.0f;
	static const float f =  1.0f;
	float m[16];
	if (pixelProjection) {
		m[0]  =  2.0f / w;
		m[5]  =  2.0f / h;
		m[12] = -1.0f;
		m[13] = -1.0f;
	}
	else {
		m[0]  =  2.0f;
		m[5]  =  2.0f;
		m[12] = -1.0f;
		m[13] = -1.0f;
	}
	m[1]  =  0.0f;
	m[2]  =  0.0f;
	m[3]  =  0.0f;
	m[4]  =  0.0f;
	m[6]  =  0.0f;
	m[7]  =  0.0f;
	m[8]  =  0.0f;
	m[9]  =  0.0f;
	m[10] = -2.0f / (f - n);
	m[11] =  0.0f;
	m[14] = -(f + n) / (f - n);
	m[15] =  1.0f;
	projection->matrix.set(m, 16);

	return true;
}

void					ViewItemModel::onPostRender(
								float, float, float w, float h)
{
	// draw
	render.setArea(w * h);
	render.traverse(projection);
}


//
// ViewItemModelReader
//

ViewItemModelReader::ViewItemModelReader() : item(NULL)
{
	// do nothing
}

ViewItemModelReader::~ViewItemModelReader()
{
	if (item != NULL)
		item->unref();
}

ViewTagReader* 	ViewItemModelReader::clone() const
{
	return new ViewItemModelReader;
}

View*					ViewItemModelReader::open(
								const ConfigReader::Values& values)
{
	assert(item == NULL);

	item = new ViewItemModel;

	ConfigReader::Values::const_iterator index;
	index = values.find("filename");
	if (index != values.end()) {
		istream* stream = FILEMGR->createDataInStream(index->second);
		if (stream == NULL) {
			printError("cannot open view model: %s", index->second.c_str());
		}
		else {
			SceneReader reader;
			SceneNode* scene = reader.read(*stream);
			delete stream;
			item->setSceneNode(scene);
		}
	}
	index = values.find("pixel");
	if (index != values.end()) {
		item->setPixelProjection(index->second == "yes");
	}

	return item;
}

void					ViewItemModelReader::close()
{
	assert(item != NULL);
	item = NULL;
}
