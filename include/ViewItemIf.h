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

#ifndef BZF_VIEWITEMIF_H
#define BZF_VIEWITEMIF_H

#include "View.h"

class MessageBuffer;

class ViewItemIf : public View {
public:
	ViewItemIf(bool negate);

	void				setName(const std::string& name);
	void				setValue(const std::string& value);

protected:
	virtual ~ViewItemIf();

	// View overrides
	virtual bool		onPreRender(float x, float y, float w, float h);
	virtual void		onPostRender(float x, float y, float w, float h);

private:
	bool				negate;
	bool				truth;
	std::string			name;
	std::string			value;
};

class ViewItemIfReader : public ViewTagReader {
public:
	ViewItemIfReader(bool negate);
	virtual ~ViewItemIfReader();

	// ViewItemReader overrides
	virtual ViewTagReader* clone() const;
	virtual View*		open(XMLTree::iterator);

private:
	bool				negate;
	ViewItemIf*			item;
};

#endif
// ex: shiftwidth=4 tabstop=4
