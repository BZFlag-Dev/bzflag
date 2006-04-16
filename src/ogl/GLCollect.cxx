/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "GLCollect.h"

int GLCollect::count = 0;

GLCollect::GLCollect( GLenum en )
{
	if( count++ == 0 )
		glBegin( en );
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

GLCollect::~GLCollect()
{
	if( --count == 0 )
		glEnd();
}
