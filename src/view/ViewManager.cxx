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

#include "ViewManager.h"
#include "View.h"
#include "ViewItemBuffer.h"
#include "ViewItemIf.h"
#include "ViewItemMenu.h"
#include "ViewItemMessages.h"
#include "ViewItemModel.h"
#include "ViewItemPoly.h"
#include "ViewItemScissor.h"
#include "ViewItemText.h"
#include "ViewItemViewport.h"

//
// ViewManager
//

ViewManager*			ViewManager::mgr = NULL;

ViewManager::ViewManager()
{
	// register built-in tag readers
	addReader("buffer", new ViewItemBufferReader);
	addReader("if", new ViewItemIfReader(false));
	addReader("menu", new ViewItemMenuReader);
	addReader("message", new ViewItemMessagesReader);
	addReader("model", new ViewItemModelReader);
	addReader("poly", new ViewItemPolyReader);
	addReader("scissor", new ViewItemScissorReader);
	addReader("text", new ViewItemTextReader);
	addReader("unless", new ViewItemIfReader(true));
	addReader("viewport", new ViewItemViewportReader);
}

ViewManager::~ViewManager()
{
	// done with views
	for (Views::iterator vIndex = views.begin();
								vIndex != views.end(); ++vIndex)
		vIndex->second->unref();

	// done with readers
	for (Readers::iterator rIndex = readers.begin();
								rIndex != readers.end(); ++rIndex)
		delete rIndex->second;

	mgr = NULL;
}

void					ViewManager::add(const BzfString& name, View* view)
{
	remove(name);
	if (view != NULL) {
		view->ref();
		views.insert(std::make_pair(name, view));
	}
}

void					ViewManager::remove(const BzfString& name)
{
	Views::iterator index = views.find(name);
	if (index != views.end()) {
		index->second->unref();
		views.erase(index);
	}
}

View*					ViewManager::get(const BzfString& name) const
{
	Views::const_iterator index = views.find(name);
	if (index == views.end())
		return NULL;
	else
		return index->second;
}

void					ViewManager::iterate(
								Callback callback, void* userData)
{
	for (Views::iterator vIndex = views.begin();
								vIndex != views.end(); ++vIndex)
		callback(vIndex->second, vIndex->first, userData);
}

void					ViewManager::addReader(
								const BzfString& name, ViewTagReader* reader)
{
	removeReader(name);
	if (reader != NULL)
		readers.insert(std::make_pair(name, reader));
}

void					ViewManager::removeReader(const BzfString& name)
{
	Readers::iterator index = readers.find(name);
	if (index != readers.end()) {
		delete index->second;
		readers.erase(index);
	}
}

const ViewTagReader*		ViewManager::getReader(const BzfString& name) const
{
	Readers::const_iterator index = readers.find(name);
	if (index == readers.end())
		return NULL;
	else
		return index->second;
}

void					ViewManager::iterateReaders(
								ReaderCallback callback, void* userData)
{
	for (Readers::iterator rIndex = readers.begin();
								rIndex != readers.end(); ++rIndex)
		callback(rIndex->second, rIndex->first, userData);
}

ViewManager*			ViewManager::getInstance()
{
	if (mgr == NULL)
		mgr = new ViewManager;
	return mgr;
}
