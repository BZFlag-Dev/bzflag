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

#ifndef BZF_VIEWITEMBUFFER_H
#define BZF_VIEWITEMBUFFER_H

#include "View.h"

// sets/restores the draw buffer
class ViewItemBuffer : public View {
public:
	enum Buffer { Both, Left, Right };

	ViewItemBuffer();

	void				setBuffer(Buffer);

protected:
	virtual ~ViewItemBuffer();

	Buffer				getBuffer() const { return buffer; }

	// View overrides
	virtual bool		onPreRender(float x, float y, float w, float h);
	virtual void		onPostRender(float x, float y, float w, float h);

private:
	Buffer				buffer;
	Buffer				saved;
	static Buffer		current;
};

class ViewItemBufferReader : public ViewTagReader {
public:
	ViewItemBufferReader();
	virtual ~ViewItemBufferReader();

	// ViewTagReader overrides
	virtual ViewTagReader* clone() const;
	virtual View*		open(const ConfigReader::Values& values);
	virtual void		close();

private:
	ViewItemBuffer*		item;
};

#endif
