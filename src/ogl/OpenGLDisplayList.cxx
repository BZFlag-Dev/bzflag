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

#include "OpenGLDisplayList.h"
#
//
// OpenGLDisplayList::Rep
//

OpenGLDisplayList::Rep::Rep() : refCount(1), list(0)
{
  // do nothing
}

OpenGLDisplayList::Rep::~Rep()
{
  // free OpenGL display list
  if (list) glDeleteLists(list, 1);
}

//
// OpenGLDisplayList
//

OpenGLDisplayList::OpenGLDisplayList()
{
  rep = new Rep;
}

OpenGLDisplayList::OpenGLDisplayList(const OpenGLDisplayList& list)
{
  rep = list.rep;
  ref();
}

OpenGLDisplayList::~OpenGLDisplayList()
{
  if (unref()) delete rep;
}

OpenGLDisplayList&	OpenGLDisplayList::operator=(
				const OpenGLDisplayList& list)
{
  if (rep != list.rep) {
    if (unref()) delete rep;
    rep = list.rep;
    ref();
  }
  return *this;
}

boolean			OpenGLDisplayList::operator==(
				const OpenGLDisplayList& list) const
{
  // follow test compares undefined display lists equal even if
  // the reps are different.
  return rep->list == list.rep->list;
}

boolean			OpenGLDisplayList::operator!=(
				const OpenGLDisplayList& list) const
{
  return rep->list != list.rep->list;
}

boolean			OpenGLDisplayList::operator<(
				const OpenGLDisplayList& list) const
{
  return rep->list < list.rep->list;
}

boolean			OpenGLDisplayList::isValid() const
{
  return rep->list != 0;
}

GLuint			OpenGLDisplayList::getList() const
{
  return rep->list;
}

void			OpenGLDisplayList::begin()
{
  if (rep->list == 0) {
    rep->list = glGenLists(1);
  }
  glNewList(rep->list, GL_COMPILE);
}

void			OpenGLDisplayList::end()
{
  glEndList();
}

void			OpenGLDisplayList::execute()
{
  glCallList(rep->list);
}

void			OpenGLDisplayList::ref()
{
  ++rep->refCount;
}

boolean			OpenGLDisplayList::unref()
{
  return --rep->refCount == 0;
}
// ex: shiftwidth=2 tabstop=8
