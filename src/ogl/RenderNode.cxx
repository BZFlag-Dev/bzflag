/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "RenderNode.h"

//
// RenderNodeList
//

static const int	initialSize = 31;


RenderNodeList::RenderNodeList() : count(0), size(0), list(NULL)
{
  // do nothing
}


RenderNodeList::~RenderNodeList()
{
  delete[] list;
}


void RenderNodeList::clear()
{
  count = 0;
}


void RenderNodeList::append(RenderNode* node)
{
  if (count == size) {
    grow();
  }
  list[count++] = node;
}


void RenderNodeList::render() const
{
  for (int i = 0; i < count; i++) {
    list[i]->renderShadow();
  }
}


void RenderNodeList::grow()
{
  const int newSize = (size == 0) ? initialSize : (size << 1) + 1;
  RenderNode** newList = new RenderNode*[newSize];
  if (list) memcpy(newList, list, count * sizeof(RenderNode*));
  delete[] list;
  list = newList;
  size = newSize;
}


//
// RenderNodeGStateList
//

RenderNodeGStateList::RenderNodeGStateList() :
				count(0), size(0), list(NULL)
{
  // do nothing
}


RenderNodeGStateList::~RenderNodeGStateList()
{
  delete[] list;
}


void RenderNodeGStateList::clear()
{
  count = 0;
}


void RenderNodeGStateList::append(RenderNode* node,
						const OpenGLGState* gstate)
{
  if (count == size) {
    grow();
  }
  list[count].node = node;
  list[count].gstate = gstate;
  list[count].depth = 0.0f;
  count++;
}


void RenderNodeGStateList::render() const
{
  for (int i = 0; i < count; i++) {
    list[i].gstate->setState();
    list[i].node->render();
  }
}


void RenderNodeGStateList::grow()
{
  const int newSize = (size == 0) ? initialSize : (size << 1) + 1;
  Item* newList = new Item[newSize];
  if (list) memcpy(newList, list, count * sizeof(Item));
  delete[] list;
  list = newList;
  size = newSize;
}


static int distCompare(const void *p1, const void* p2)
{
  const RenderNodeGStateList::Item* item1 =
    (const RenderNodeGStateList::Item*) p1;
  const RenderNodeGStateList::Item* item2 =
    (const RenderNodeGStateList::Item*) p2;

  if (item1->depth > item2->depth) {
    return -1;
  } else {
    return +1;
  }
}

void RenderNodeGStateList::sort(const GLfloat* e)
{
  int i;

  // get depths
  for (i = 0; i < count; i++) {
    const GLfloat* p = list[i].node->getPosition();
    list[i].depth = ((p[0] - e[0]) * (p[0] - e[0])) +
		    ((p[1] - e[1]) * (p[1] - e[1])) +
		    ((p[2] - e[2]) * (p[2] - e[2]));
  }

  qsort (list, count, sizeof(Item), distCompare);

  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

