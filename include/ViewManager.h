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

#ifndef BZF_VIEW_MANAGER_H
#define BZF_VIEW_MANAGER_H

#include <string>
#include <map>

#define VIEWMGR (ViewManager::getInstance())

class View;
class ViewTagReader;

class ViewManager {
public:
	typedef void (*Callback)(View*, const std::string&, void* userData);
	typedef void (*ReaderCallback)(const ViewTagReader*,
							const std::string&, void* userData);

	~ViewManager();

	// add/remove/get a view.  get() does *not* ref the view.  add
	// ref's the view so the caller must unref it to release it.
	void				add(const std::string& name, View*);
	void				remove(const std::string& name);
	View*				get(const std::string& name) const;

	// iterate over all views.  the views passed to the callback are
	// not ref'd.  views must not be added or removed during iteration.
	void				iterate(Callback, void* userData);

	// add/remove a view reader.  readers are adopted.
	void				addReader(const std::string& name,
							ViewTagReader* adopted);
	void				removeReader(const std::string& name);
	const ViewTagReader* getReader(const std::string& name) const;

	// iterate over all view readers.  readers must not be added or
	// removed during iteration.
	void				iterateReaders(ReaderCallback, void* userData);

	static ViewManager*	getInstance();

private:
	ViewManager();

private:
	typedef std::map<std::string, View*> Views;
	typedef std::map<std::string, ViewTagReader*> Readers;

	Views				views;
	Readers				readers;

	static ViewManager*	mgr;
};

#endif
