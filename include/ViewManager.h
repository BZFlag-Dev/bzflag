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

#ifndef BZF_VIEW_MANAGER_H
#define BZF_VIEW_MANAGER_H

#include "BzfString.h"
#include <map>

#define VIEWMGR (ViewManager::getInstance())

class View;
class ViewTagReader;

class ViewManager {
public:
	typedef void (*Callback)(View*, const BzfString&, void* userData);
	typedef void (*ReaderCallback)(const ViewTagReader*,
							const BzfString&, void* userData);

	~ViewManager();

	// add/remove/get a view.  get() does *not* ref the view.  add
	// ref's the view so the caller must unref it to release it.
	void				add(const BzfString& name, View*);
	void				remove(const BzfString& name);
	View*				get(const BzfString& name) const;

	// iterate over all views.  the views passed to the callback are
	// not ref'd.  views must not be added or removed during iteration.
	void				iterate(Callback, void* userData);

	// add/remove a view reader.  readers are adopted.
	void				addReader(const BzfString& name,
							ViewTagReader* adopted);
	void				removeReader(const BzfString& name);
	const ViewTagReader* getReader(const BzfString& name) const;

	// iterate over all view readers.  readers must not be added or
	// removed during iteration.
	void				iterateReaders(ReaderCallback, void* userData);

	static ViewManager*	getInstance();

private:
	ViewManager();

private:
	typedef std::map<BzfString, View*> Views;
	typedef std::map<BzfString, ViewTagReader*> Readers;

	Views				views;
	Readers				readers;

	static ViewManager*	mgr;
};

#endif
