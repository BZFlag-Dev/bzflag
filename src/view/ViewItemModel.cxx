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
#include "StateDatabase.h"
#include "SceneNodeMatrixTransform.h"
#include "SceneReader.h"
#include <assert.h>
#include <iostream>

//
// ViewItemModel
//

ViewItemModel::ViewItemModel() : pixelProjection(false)
{
	// prep projection
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

View*					ViewItemModelReader::open(XMLTree::iterator xml)
{
	// create item
	assert(item == NULL);
	item = new ViewItemModel;

	// parse
	std::string filename;
	if (xml->getAttribute("filename", filename)) {
		std::istream* stream = FILEMGR->createDataInStream(filename);
		if (stream == NULL)
			throw XMLIOException(xml->position,
							string_util::format(
								"cannot open model `%s'", filename.c_str()));

		try {
			// read XML
			XMLTree xmlTree;
			xmlTree.read(*stream, XMLStreamPosition(filename));

			// parse scene
			SceneReader reader;
			SceneNode* node = reader.parse(xmlTree.begin());
			if (node != NULL) {
				item->setSceneNode(node);
				node->unref();
			}
		}
		catch (XMLIOException&) {
			delete stream;
			throw;
		}
	}
	xml->getAttribute("pixel", xmlParseEnum(s_xmlEnumBool,
							xmlSetMethod(item,
								&ViewItemModel::setPixelProjection)));

	return item;
}
