/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

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

  const float diff = (itemA->depth - itemB->depth);

  // sort from back-to-from
  if (diff > 0.0f) {
    return -1; // a is further away, render it first
  }
  else if (diff < 0.0f) {
    return +1; // b is further away, render it first
  }
  else {
    // sort by ascending orders
    const OpenGLGState* aState = itemA->gstate;
    const OpenGLGState* bState = itemB->gstate;
    if (aState->getOrder() < bState->getOrder()) {
      return -1; // a order is smaller, render it first
    } else {
      return +1; // b order is smaller, render it first
    }
  }
}

void RenderNodeGStateList::sort(const fvec3& eye)
{
  // calculate distances from the eye (squared)
  for (int i = 0; i < count; i++) {
    const fvec3& pos = list[i].node->getPosition();
    list[i].depth = (pos - eye).lengthSq();
  }

  // sort from farthest to closest
  qsort(list, count, sizeof(Item), nodeCompare);

  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
