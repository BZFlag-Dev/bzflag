/* bzflag
* Copyright (c) 1993 - 2007 Tim Riker
* Writen By Jeffrey Myers
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/* interface header */
#include "OpenGLUtils.h"

/* system headers */
#include <math.h>


float getFloatColor ( int )
{
  return 0.0f;
}

void setColor ( float [3], int, int, int )
{
}

void glSetColor ( float [3], float)
{
}

void glTranslatefv ( float [3] )
{
}

void glQuad ( float, float, eAlignment, float )
{
}

void glLineRing ( float, float )
{
}

// DisplayListSystem

DisplayListSystem::~DisplayListSystem()
{
}

void DisplayListSystem::flushLists ( void )
{
}

GLDisplayList DisplayListSystem::newList (GLDisplayListCreator *)
{
  return GLDisplayList();
}

void DisplayListSystem::freeList (GLDisplayList)
{
}

void DisplayListSystem::callList (GLDisplayList)
{
}

void DisplayListSystem::callListsV (std::vector<GLDisplayList> &)
{
}

DisplayListSystem::DisplayListSystem()
{
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
