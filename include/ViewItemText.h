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

#ifndef BZF_VIEWITEMTEXT_H
#define BZF_VIEWITEMTEXT_H

#include "View.h"

class ViewItemText : public View {
public:
	ViewItemText();

	void				setText(const std::string&);
	void				setPosition(const ViewSize& x, const ViewSize& y);
	void				setShadow(bool);

protected:
	virtual ~ViewItemText();

	// View overrides
	virtual bool		onPreRender(float x, float y, float w, float h);
	virtual void		onPostRender(float x, float y, float w, float h);

private:
	typedef std::vector<std::string> Items;

	void				makeLines(Items& lines);
	std::string			makeLine(Items::const_iterator&) const;

private:
	Items				items;
	ViewSize			x, y;
	bool				shadow;
};

class ViewItemTextReader : public ViewTagReader {
public:
	ViewItemTextReader();
	virtual ~ViewItemTextReader();

	// ViewItemReader overrides
	virtual ViewTagReader* clone() const;
	virtual View*		open(XMLTree::iterator);
	virtual bool		parse(XMLTree::iterator);
	virtual void		close();

private:
	std::string			msg;
	ViewItemText*		item;
};

#endif
// ex: shiftwidth=4 tabstop=4
