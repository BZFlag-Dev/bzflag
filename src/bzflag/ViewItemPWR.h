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

#ifndef BZF_VIEWITEMPWR_H
#define BZF_VIEWITEMPWR_H

#include "View.h"

enum PWRType {
	Shots,
	BadFlags
};

class ViewItemPWR : public View {
public:
	ViewItemPWR(PWRType _type);

protected:
	virtual ~ViewItemPWR();

	// View overrides
	virtual bool			onPreRender(float x, float y, float w, float h);
	virtual void			onPostRender(float x, float y, float w, float h);

	virtual void			drawShots(float, float);
	virtual void			drawEvilFlags(float, float);

private:
	void					drawPB(int, int, int, int);

	PWRType					type;
	int						x, y;
};

class ViewItemPWRReader : public ViewTagReader {
public:
	ViewItemPWRReader();
	virtual ~ViewItemPWRReader();

	// ViewItemReader overrides
	virtual ViewTagReader*	clone() const;
	virtual View*			open(XMLTree::iterator);

private:
	ViewItemPWR*			item;
};

#endif
// ex: shiftwidth=4 tabstop=4
