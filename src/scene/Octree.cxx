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

#include "common.h"
#include "Octree.h"
#include "Intersect.h"

static const int fullListBreak = 3;

static int maxDepth = 0;
static int minElements = 16;

static int leafNodes = 0;
static int totalNodes = 0;
static int totalElements = 0;

// FIXME ? shared static variables
// won't work for multiple instances of Octree
static int CullListSize = 0;
static int CullListCount = 0;
static SceneNode** CullList = NULL;
static const Frustum* CullFrustum = NULL;

#if (USE_REAL_INLINE)
inline static void addCullListNode (SceneNode* node)
{
  CullList[CullListCount] = node;
  CullListCount++;
  return;
}
#else
#define addCullListNode(node)       \
  CullList[CullListCount] = node;   \
  CullListCount++;
#endif


inline static void squeezeChildren (OctreeNode** children)
{
  for (int dst = 0; dst < 8; dst++) {
    if (children[dst] == NULL) {
      // replace with the next non-NULL
      for (int src = (dst + 1); src < 8; src++) {
        if (children[src] != NULL) {
          children[dst] = children[src];
          children[src] = NULL;
          break;
        }
      }
    }
  }
  return;
}


//////////////////////////////////////////////////////////////////////////////
//
// The Octree
//

Octree::Octree()
{
  root = NULL;
  CullList = NULL;
  CullFrustum = NULL;
  CullListSize = 0;
  CullListCount = 0;
  
  return;
}


Octree::~Octree()
{
  clear();
  return;
}


void Octree::clear ()
{
  delete root;
  root = NULL;
  CullList = NULL;
  CullFrustum = NULL;
  CullListSize = 0;
  CullListCount = 0;
  return;
}


void Octree::addNodes (SceneNode** list, int listSize, int depth, int elements)
{
  if (root) {
    clear();
  }
  
  maxDepth = depth;
  minElements = elements;
  
  CullList = list;
  CullListSize = listSize;
  
  int i;
  float mins[3]; // minimum extents
  float maxs[3]; // maximum extents
  float width;

  getExtents (mins, maxs, width, list, listSize);
  
  float center[3];
  for (i = 0; i < 3; i++) {
    center[i] = mins[i] + (0.5f * (maxs[i] - mins[i]));
  }
  // for bzflag, the top octants probably won't get used much
  center[2] = mins[2] + (0.5f * width) - 1.0f;
  
  DEBUG2 ("Octree width = %f\n", width);
  for (i = 0; i < 3; i++) {
    DEBUG2 ("  extent[%i] = %f, %f\n", i, mins[i], maxs[i]);
  }
  DEBUG2 ("  center = %f, %f, %f\n", center[0], center[1], center[2]);
  DEBUG2 ("Octree scene nodes = %i\n", listSize);

  // making babies
  root = new OctreeNode(center, width, 0, list, listSize);
  
  return;
}


int Octree::getFrustumList (SceneNode** list, int listSize,
                            const Frustum* frustum) const
{
  if (!root) {
    return 0;
  }
  
  // FIXME - testing hack
  if (listSize > CullListSize) {
    printf ("Octree::getFrustumList() Internal error! (%i vs %i)\n",
            listSize, CullListSize);
    exit (1);
  }
  
  CullFrustum = frustum;
  CullList = list;
  CullListCount = 0;
  
  // get the nodes
  DEBUG4 ("Octree::getFrustumList: root count = %i\n", root->getCount());
  root->getFrustumList ();

  return CullListCount;
}


void Octree::getExtents (float* mins, float* maxs, float& width,
                         SceneNode** list, int listSize)
{
  int i;

  for (i = 0; i < 3; i++) {
    mins[i] = +MAXFLOAT;
    maxs[i] = -MAXFLOAT;
  }

  for (i = 0; i < listSize; i++) {
    SceneNode* node = list[i];
    float nodeMin[3];
    float nodeMax[3];
    node->getExtents(nodeMin, nodeMax);
    for (int a = 0; a < 3; a++) {
      if (nodeMin[a] < mins[a]) {
        mins[a] = nodeMin[a];
      }
      if (nodeMax[a] > maxs[a]) {
        maxs[a] = nodeMax[a];
      }
    }
  }

  width = -MAXFLOAT;
  for (i = 0; i < 3; i++) {
    float axisWidth = maxs[i] - mins[i];
    if (axisWidth > width) {
      width = axisWidth;
    }
  }
    
  return;
}


void Octree::draw (DrawLineFunc drawLines)
{
  if (!root) {
    return;
  }

  root->draw (drawLines);
  
  return;
}


//////////////////////////////////////////////////////////////////////////////
//
// The Nodes
//

OctreeNode::OctreeNode(const float* center, float width,
                       unsigned char _depth,
                       SceneNode** _list, int _listSize)
{
  int i;

  if (_depth == 0) {  
    leafNodes = 0;
    totalNodes = 0;
  }
  
  depth = _depth;
  count = 0;
  
  for (i = 0; i < 8; i++) {
    children[i] = NULL;
  }
  childCount = 0;
  
  const float hw = width * 0.5f;
  for (i = 0; i < 3; i++) {
    mins[i] = center[i] - hw;
    maxs[i] = center[i] + hw;
  }

  // copy the incoming list  
  const int listBytes = _listSize * sizeof (SceneNode*);
  list = (SceneNode**) malloc (listBytes);
  memcpy (list, _list, listBytes);

  // find all of the intersecting nodes  
  listSize = 0;
  for (i = 0; i < _listSize; i++) {
    SceneNode* node = _list[i];
    if (node->inAxisBox (mins, maxs)) {
      list[listSize] = node;
      listSize++;
    }
  }
  count = listSize;
  
  // resize the list to save space
  list = (SceneNode**) realloc (list, count * sizeof (SceneNode*));

  // resize the cell
  float absMins[3] = { +MAXFLOAT, +MAXFLOAT, +MAXFLOAT};
  float absMaxs[3] = { -MAXFLOAT, -MAXFLOAT, -MAXFLOAT};
  float tmpMins[3], tmpMaxs[3];
  for (i = 0; i < count; i++) {
    SceneNode* node = list[i];
    node->getExtents (tmpMins, tmpMaxs);
    for (int a = 0; a < 3; a++) {
      if (tmpMins[a] < absMins[a])
        absMins[a] = tmpMins[a];
      if (tmpMaxs[a] > absMaxs[a])
        absMaxs[a] = tmpMaxs[a];
    }
  }
  for (i = 0; i < 3; i++) {
   if (absMins[i] > mins[i])
      mins[i] = absMins[i];
    if (absMaxs[i] < maxs[i])
      maxs[i] = absMaxs[i];
  }

  if (((int)depth >= maxDepth) || (listSize <= minElements)) {
    DEBUG4 ("FINAL NODE: depth = %d, items = %i\n", depth, count);
    leafNodes++;
    totalNodes++;
    totalElements += listSize;
    return;
  }

  // making babies  
  for (int x = 0; x < 2; x++) {
    for (int y = 0; y < 2; y++) {
      for (int z = 0; z < 2; z++) {
        float newCenter[3];
        const float qw = width * 0.25f;
        int kid = x + (2 * y) + (4 * z);
        newCenter[0] = center[0] + ((float)(1 - (x * 2)) * qw) ;
        newCenter[1] = center[1] + ((float)(1 - (y * 2)) * qw) ;
        newCenter[2] = center[2] + ((float)(1 - (z * 2)) * qw) ;

        children[kid] = new OctreeNode (newCenter, hw, depth + 1, list, count);

        if (children[kid]->getCount() == 0) {
          delete children[kid];
          children[kid] = NULL;
        }
        else {
          childCount++;
        }
      }
    }
  }
  
  // clip the top of the box
  // FIXME - clip all axes & dirs?
  for (i = 0; i < 4; i++) {
    if (children[i] != NULL) {
      break;
    }
  }
  if (i == 4) {
    maxs[2] = 0.5f * (mins[2] + maxs[2]);
  }
    
  // non NULLs first
  squeezeChildren (children);

  // leave some lists for FullyVisible grabs
  if (((depth + 1) % fullListBreak) != 0) {
    listSize = 0;
    free (list);
    list = NULL;
  }
  else {
    totalElements += listSize;
  }
  
  DEBUG4 ("BRANCH NODE: depth = %d, children = %i\n", depth, childCount);
  totalNodes++;

  if (_depth == 0) {  
    DEBUG2 ("Octree leaf nodes  = %i\n", leafNodes);
    DEBUG2 ("Octree total nodes = %i\n", totalNodes);
    DEBUG2 ("Octree total elements = %i\n", totalElements);
  }
  
  return;
}


OctreeNode::~OctreeNode()
{
  for (int i = 0; i < 8; i++) {
    delete children[i];
  }
  free (list);
  return;
}


void OctreeNode::getFrustumList () const
{
  IntersectLevel level = testAxisBoxInFrustum (mins, maxs, CullFrustum);

  if (level == Outside) {
    return;
  }
  else if (level == Contained) {
    getFullyVisible ();
    return;
  }
  
  // this cell is only partially contained within the frustum
    
  if (childCount > 0) {
    for (int i = 0; i < childCount; i++) {
      children[i]->getFrustumList ();
    }
  }
  else {
    for (int i = 0; i < listSize; i++) {
      SceneNode* node = list[i];
      if (node->octreeState == SceneNode::OctreeCulled) {
        addCullListNode (node);
        node->octreeState = SceneNode::OctreePartial;
      }
    }
  }

  return;
}

void OctreeNode::getFullyVisible () const
{
/*
  static struct NodeStack {
    OctreeNode* node;
    int child;
  } nodeStack[OctreeNode::maxDepth];
  static int level;
  static int childNum;
  static OctreeNode* working;

  level = 0;
  nodeStack[0].child = 0;
  working = (OctreeNode*) this;

  // FIXME - faster then the recursive version?

  while (level >= 0) {
    childNum = nodeStack[level].child;
    if (working->getChildren() > childNum) {
      nodeStack[level].node = working;
      nodeStack[level].child++;
      working = working->getChild(childNum);
      level++;
      nodeStack[level].child = 0;
    } else {
      static int i;
      for (i = 0; i < working->getListSize(); i++) {
        static SceneNode* node;
        node = working->getList()[i];
        if (node->octreeState == SceneNode::OctreeCulled) {
          addCullListNode (node);
        }
        node->octreeState = SceneNode::OctreeVisible;
      }
      level--;
      working = nodeStack[level].node;
    }
  }
*/  
  if ((childCount > 0) && (((depth + 1) % fullListBreak) != 0)) {
    for (int i = 0; i < childCount; i++) {
      children[i]->getFullyVisible ();
    }
  }
  else {
    for (int i = 0; i < listSize; i++) {
      SceneNode* node = list[i];
      SceneNode::CullState& state = node->octreeState;
      if (state == SceneNode::OctreeCulled) {
        addCullListNode (node);
      }
      state = SceneNode::OctreeVisible;
    }
  }
  return;
}

/*
void OctreeNode::getFullyVisible () const
{
  if (childCount > 0) {
    for (int i = 0; i < childCount; i++) {
      children[i]->getFullyVisible ();
    }
  }
  else {
    for (int i = 0; i < listSize; i++) {
      SceneNode* node = list[i];
      if (node->octreeState == SceneNode::OctreeCulled) {
        addCullListNode (node);
      }
      node->octreeState = SceneNode::OctreeVisible;
    }
  }
  return;
}
*/

void OctreeNode::draw (DrawLineFunc drawLines)
{
  int x, y, z, c;
  float points[5][3];
  bool partial = false;

  if (CullFrustum != NULL) {
    IntersectLevel level = testAxisBoxInFrustum (mins, maxs, CullFrustum);
    if (level == Partial) {
      partial = true;
    }
  }
  
  const float* extents[2] = { mins, maxs };
  
  // draw Z-normal squares
  for (z = 0; z < 2; z++) {
    for (c = 0; c < 4; c++) {
      x = ((c + 0) % 4) / 2;
      y = ((c + 1) % 4) / 2;
      points[c][0] = extents[x][0];
      points[c][1] = extents[y][1];
      points[c][2] = extents[z][2];
    }
    memcpy (points[4], points[0], sizeof (points[4]));
    drawLines (5, points, partial);
  }

  // draw the corner edges
  for (c = 0; c < 4; c++) {
    x = ((c + 0) % 4) / 2;
    y = ((c + 1) % 4) / 2;
    for (z = 0; z < 2; z++) {
      points[z][0] = extents[x][0];
      points[z][1] = extents[y][1];
      points[z][2] = extents[z][2];
    }
    drawLines (2, points, partial);
  }

  // draw the kids
  for (c = 0; c < childCount; c++) {
    children[c]->draw (drawLines);
  }

  return;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
