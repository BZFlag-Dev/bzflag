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

#ifndef BZF_VIEWITEMACCUM_H
#define BZF_VIEWITEMACCUM_H

#include "View.h"

class ViewItemAccum : public View {
public:
	ViewItemAccum();
protected:
	virtual ~ViewItemAccum();

	// View overrides
	virtual bool		onPreRender(float x, float y, float w, float h);
	virtual void		onPostRender(float x, float y, float w, float h);
	virtual void		renderChildren(float x, float y, float w, float h);
};

class ViewItemAccumReader : public ViewTagReader {
public:
	ViewItemAccumReader();
	virtual ~ViewItemAccumReader();

	// ViewItemReader overrides
	virtual ViewTagReader* clone() const;
	virtual View*		open(XMLTree::iterator);

private:
	ViewItemAccum*		item;
};

#endif
// ex: shiftwidth=4 tabstop=4
