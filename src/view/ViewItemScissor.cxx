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

#include "ViewItemScissor.h"
#include "bzfgl.h"

//
// ViewItemScissor
//

ViewItemScissor::ViewItemScissor()
{
	// do nothing
}

ViewItemScissor::~ViewItemScissor()
{
	// do nothing
}
void					ViewItemScissor::onPostRender(
								float, float, float, float)
{
	glPopAttrib();
}

void					ViewItemScissor::onRender(
								float x, float y, float w, float h)
{
	// compute area carefully to avoid gaps and overlap
	GLint   ix = static_cast<GLint>(x + 0.5f);
	GLint   iy = static_cast<GLint>(y + 0.5f);
	GLsizei iw = static_cast<GLsizei>(x + w + 0.5f) - ix;
	GLsizei ih = static_cast<GLsizei>(y + h + 0.5f) - iy;

	// set scissor only
	glPushAttrib(GL_SCISSOR_BIT);
	glScissor(ix, iy, iw, ih);
}


//
// ViewItemScissorReader
//

ViewItemScissorReader::ViewItemScissorReader()
{
	// do nothing
}

ViewItemScissorReader::~ViewItemScissorReader()
{
	// do nothing
}

ViewItemViewport*		ViewItemScissorReader::create() const
{
	return new ViewItemScissor;
}
// ex: shiftwidth=4 tabstop=4
