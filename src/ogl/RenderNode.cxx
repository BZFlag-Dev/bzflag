/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "RenderNode.h"


//
// RenderNode
//

int RenderNode::triangleCount = 0;


int RenderNode::getTriangleCount()
{
  return triangleCount;
}


void RenderNode::resetTriangleCount()
{
  triangleCount = 0;
  return;
}


//
// RenderNodeList
//

static const int initialSize = 31;

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


static int nodeCompare(const void *a, const void* b)
{
  const RenderNodeGStateList::Item* itemA =
    (const RenderNodeGStateList::Item*) a;
  const RenderNodeGStateList::Item* itemB =
    (const RenderNodeGStateList::Item*) b;

  // draw from back to front
  if (itemA->depth > itemB->depth) {
    return -1;
  } else {
    return +1;
  }
}

void RenderNodeGStateList::sort(const GLfloat* e)
{
  // calculate distances from the eye (squared)
  for (int i = 0; i < count; i++) {
    const GLfloat* p = list[i].node->getPosition();
    const float dx = (p[0] - e[0]);
    const float dy = (p[1] - e[1]);
    const float dz = (p[2] - e[2]);
    list[i].depth = ((dx * dx) + (dy * dy) + (dz * dz));
    // FIXME - dirty hack (they are all really getSphere())
    //if (list[i].depth < p[3]) {
    //  list[i].depth = -1.0f;
    //}
  }

  // sort from farthest to closest
  qsort (list, count, sizeof(Item), nodeCompare);

  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

