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

#ifndef BZF_VIEW_READER_H
#define BZF_VIEW_READER_H

#include "ConfigIO.h"
#include "View.h"
#include <map>
#include <vector>

//
// object to parse a view configuration
//

class ViewReader {
public:
	ViewReader();
	~ViewReader();

	// add/remove ViewItem readers, used by read()
	void				insert(const BzfString& tag,
							ViewTagReader* adoptedReader);
	void				remove(const BzfString& tag);

	// read a view configuration, saving results to the VIEWMGR
	bool				read(istream&);

	static bool			parseColor(ViewColor&,
							const ConfigReader::Values&);
	static bool			readSize(const ConfigReader::Values& values,
							const BzfString& name,
							ViewSize& value,
							float pixel,
							float fraction);

private:
	bool				openTop(ConfigReader*, const BzfString&,
							const ConfigReader::Values&);
	bool				openView(ConfigReader*, const BzfString&,
							const ConfigReader::Values&);
	bool				closeTop(const BzfString&);
	bool				closeView(const BzfString&);
	bool				dataView(ConfigReader*, const BzfString&);

	static void			onTagReaderCB(const ViewTagReader*,
							const BzfString&, void*);

	static bool			parseColor(const BzfString& value, float* color);

	static bool			openTopCB(ConfigReader*, const BzfString&, const ConfigReader::Values&, void*);
	static bool			openViewCB(ConfigReader*, const BzfString&, const ConfigReader::Values&, void*);
	static bool			closeTopCB(ConfigReader*, const BzfString&, void*);
	static bool			closeViewCB(ConfigReader*, const BzfString&, void*);
	static bool			dataViewCB(ConfigReader*, const BzfString&, void*);

private:
	struct State {
	public:
		ViewState		state;
		View*			view;
		ViewTagReader*	reader;
		int				levels;
	};
	typedef std::vector<State> Stack;
	typedef std::map<BzfString, ViewTagReader*> ItemReaders;
	typedef std::map<BzfString, View*> NamedItems;

	ItemReaders			itemReaders;
	Stack				stack;
	BzfString			viewName;
	NamedItems			namedItems;
};

#endif
