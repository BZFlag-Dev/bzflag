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

#ifndef BZF_VIEWITEMSCISSOR_H
#define BZF_VIEWITEMSCISSOR_H

#include "ViewItemViewport.h"

// sets/restores the rendering scissor region
class ViewItemScissor : public ViewItemViewport {
public:
	ViewItemScissor();

protected:
	virtual ~ViewItemScissor();

	// ViewViewport overrides
	virtual void		onRender(float x, float y, float w, float h);

	// View overrides
	virtual void		onPostRender(float x, float y, float w, float h);
};

class ViewItemScissorReader : public ViewItemViewportReader {
public:
	ViewItemScissorReader();
	virtual ~ViewItemScissorReader();

protected:
	// ViewItemViewportReader overrides
	virtual ViewItemViewport* create() const;
};


#endif
