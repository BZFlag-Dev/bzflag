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

#include "View.h"
#include "XMLTree.h"
#include "ConfigFileReader.h"
#include <stdio.h>
#include <map>
#include <vector>

//
// object to parse a view configuration
//

class ViewReader : public ConfigFileReader {
public:
	ViewReader();
	~ViewReader();

	// add/remove ViewItem readers, used by read()
	void				insert(const std::string& tag,
							ViewTagReader* adoptedReader);
	void				remove(const std::string& tag);

	// ConfigFileReader overrides.  views go to VIEWMGR.
	ConfigFileReader*	clone();
	void				parse(XMLTree::iterator);

private:
	// crs -- must have arg names here to work around bug in VC++ 6.0
	template <class T>
	void				parseChildren(XMLTree::iterator xml,
							void (ViewReader::*method)(XMLTree::iterator, T*),
							T* data);

	void				parseViews(XMLTree::iterator xml, void*);
	void				parseView(XMLTree::iterator xml, View* view);

	bool				parseStandardTags(XMLTree::iterator xml);
	void				saveNamedItem(const std::string& id, View* item);

	static void			onTagReaderCB(const ViewTagReader*,
							const std::string&, void*);

private:
	struct State {
	public:
		ViewState		state;
		int				levels;
	};
	typedef std::vector<State> Stack;
	typedef std::map<std::string, ViewTagReader*> ItemReaders;
	typedef std::map<std::string, View*> NamedItems;

	ItemReaders			itemReaders;
	Stack				stack;
	NamedItems			namedItems;
};

//
// handy function objects
//

class ViewSetSize_t : public unary_function<std::string, ViewSize&> {
public:
	ViewSetSize_t(ViewSize& size_, bool* scaled_) :
							size(size_), scaled(scaled_) { }
	ViewSize&			operator()(const std::string& arg) const
	{
		float num;
		char type;
		if (sscanf(arg.c_str(), "%f%c", &num, &type) == 2) {
			if (type == 'p') {
				size.pixel    = num;
				size.fraction = 0.0f;
				if (scaled)
					*scaled = false;
			}
			else if (type == '%') {
				size.pixel    = 0.0f;
				size.fraction = 0.01f * num;
				if (scaled)
					*scaled = false;
			}
			else if (type == 'x') {
				size.pixel    = 0.0f;
				size.fraction = num;
				if (scaled)
					*scaled = true;
			}
			return size;
		}
		else {
			throw XMLNode::AttributeException("invalid size");
		}
	}

private:
	ViewSize&			size;
	bool*				scaled;
};

inline ViewSetSize_t
viewSetSize(ViewSize& size, bool* scaled = 0)
{
	return ViewSetSize_t(size, scaled);
}

#endif
// ex: shiftwidth=4 tabstop=4
