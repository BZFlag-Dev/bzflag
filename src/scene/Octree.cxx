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

#include <math.h>

#include "Octree.h"
#include "Intersect.h"
#include "Occluder.h"

static const int fullListBreak = 3; // FIXME

static int maxDepth = 0;
static int minElements = 16;

static int leafNodes = 0;
static int totalNodes = 0;
static int totalElements = 0;

// FIXME - make them class static members once the memory is flattened
static int CullListSize = 0;
static int CullListCount = 0;
static SceneNode** CullList = NULL;
static const Frustum* CullFrustum = NULL;
static int ShadowCount = 0;
static const float (*ShadowPlanes)[4] = NULL;

static OccluderManager OcclMgr;


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
  OcclMgr.clear();
  return;
}


void Octree::addNodes (SceneNode** list, int listSize, int depth, int elements)
{
  int i;
  
  if (root) {
    clear();
  }
  
  // just in case
  for (i = 0; i < listSize; i++) {
    list[i]->octreeState = SceneNode::OctreeCulled;
  }
  
  maxDepth = depth;
  minElements = elements;

  CullList = list;
  CullListSize = listSize;

  float mins[3]; // minimum extents
  float maxs[3]; // maximum extents

  getExtents (mins, maxs, list, listSize);

  leafNodes = 0;
  totalNodes = 0;
  totalElements = 0;

  // making babies
  root = new OctreeNode(0, mins, maxs, list, listSize);

  DEBUG2 ("Octree scene nodes = %i\n", listSize);
  for (i = 0; i < 3; i++) {
    DEBUG2 ("  extent[%i] = %f, %f\n", i, mins[i], maxs[i]);
  }
  DEBUG2 ("Octree leaf nodes  = %i\n", leafNodes);
  DEBUG2 ("Octree total nodes = %i\n", totalNodes);
  DEBUG2 ("Octree total elements = %i\n", totalElements);

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

  // update the occluders before using them
  OcclMgr.update(CullFrustum);

  // get the nodes
  DEBUG4 ("Octree::getFrustumList: root count = %i\n", root->getCount());
  root->getFrustumList ();

  OcclMgr.select(CullList, CullListCount);

  return CullListCount;
}


int Octree::getShadowList (SceneNode** list, int listSize,
                           const Frustum* frustum, const float* sun) const
{
  if (!root) {
    return 0;
  }

  // we project the frustum onto the ground plane, and then
  // use those lines to generate planes in the direction of
  // the sun's light. that is the potential shadow volume.
  
  // The frustum planes are as follows:
  // 0: front
  // 1: left
  // 2: right
  // 3: bottom
  // 4: top

  int planeCount = 0;
  float planes[4][4];
  const float* eye = frustum->getEye();

  // FIXME: As a first cut, i'll be assuming that the frustum
  //        top points towards Z. Also, this should be split
  //        into a setupShadowVolume() function
  
  if (frustum->getUp()[2] > 0.999999) {
    planeCount = 2;
    float edge[2];
    // left edge
    edge[0] = -frustum->getSide(1)[1];
    edge[1] = +frustum->getSide(1)[0];
    planes[0][0] =  (edge[1] * sun[2]);
    planes[0][1] = -(edge[0] * sun[2]);
    planes[0][2] =  (edge[0] * sun[1]) - (edge[1] * sun[0]);
    planes[0][3] = -((planes[0][0] * eye[0]) + (planes[0][1] * eye[1]));
    // right edge
    edge[0] = -frustum->getSide(2)[1];
    edge[1] = +frustum->getSide(2)[0];
    planes[1][0] =  (edge[1] * sun[2]);
    planes[1][1] = -(edge[0] * sun[2]);
    planes[1][2] =  (edge[0] * sun[1]) - (edge[1] * sun[0]);
    planes[1][3] = -((planes[1][0] * eye[0]) + (planes[1][1] * eye[1]));
    // only use the bottom edge if we have some height (about one jump's worth)
    if (eye[2] > 20.0f) {
      // bottom edge
      edge[0] = -frustum->getSide(3)[1];
      edge[1] = +frustum->getSide(3)[0];
      planes[2][0] =  (edge[1] * sun[2]);
      planes[2][1] = -(edge[0] * sun[2]);
      planes[2][2] =  (edge[0] * sun[1]) - (edge[1] * sun[0]);
      const float hlen = sqrtf ((frustum->getSide(3)[0] * frustum->getSide(3)[0]) +
                                (frustum->getSide(3)[1] * frustum->getSide(3)[1]));
      const float slope = frustum->getSide(3)[2] / hlen;
      float point[2];
      point[0] = eye[0] + (eye[2] * frustum->getSide(3)[0] * slope);
      point[1] = eye[1] + (eye[2] * frustum->getSide(3)[1] * slope);
      planes[2][3] = -((planes[2][0] * point[0]) + (planes[2][1] * point[1]));
      planeCount++;
    }
  } else {
    planeCount = 0;
  }
  
  // FIXME - testing hack
  if (listSize > CullListSize) {
    printf ("Octree::getShadowList() Internal error! (%i vs %i)\n",
            listSize, CullListSize);
    exit (1);
  }

  CullList = list;
  CullListCount = 0;
  ShadowCount = planeCount;
  ShadowPlanes = planes;

  // update the occluders before using them
  // FIXME: use occluders later?  OcclMgr.update(CullFrustum);

  // get the nodes
  DEBUG4 ("Octree::getFrustumList: root count = %i\n", root->getCount());
  root->getShadowList ();

  return CullListCount;
}


void Octree::getExtents (float* mins, float* maxs,
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

  // find the longest axis
  float width = -MAXFLOAT;
  for (i = 0; i < 3; i++) {
    float axisWidth = maxs[i] - mins[i];
    if (axisWidth > width) {
      width = axisWidth;
    }
  }

  // make it a cube, with Z on its minimum
  for (i = 0; i < 2; i++) {
    const float axisWidth = maxs[i] - mins[i];
    if (axisWidth < width) {
      const float adjust = 0.5f * (width - axisWidth);
      mins[i] = mins[i] - adjust;
      maxs[i] = maxs[i] + adjust;
    }
  }
  maxs[2] = mins[2] + width;

  return;
}


void Octree::draw () const
{
  if (!root) {
    return;
  }

  GLboolean usingTextures;
  glGetBooleanv(GL_TEXTURE_2D, &usingTextures);
  glDisable (GL_TEXTURE_2D);

  // CullFrustum needs to still be valid here
  // It should still exist in SceneRender.cxx
  // when this function is called.
  root->draw ();
  OcclMgr.update(CullFrustum);
  OcclMgr.draw();

  if (usingTextures) {
    glEnable (GL_TEXTURE_2D);
  }

  return;
}


//////////////////////////////////////////////////////////////////////////////
//
// The Nodes
//

OctreeNode::OctreeNode(unsigned char _depth,
                       const float* _mins, const float *_maxs,
                       SceneNode** _list, int _listSize)
{
  int i;

  depth = _depth;

  for (i = 0; i < 8; i++) {
    children[i] = NULL;
  }
  childCount = 0;

  // copy the incoming list
  const int listBytes = _listSize * sizeof (SceneNode*);
  list = (SceneNode**) malloc (listBytes);
  memcpy (list, _list, listBytes);

  // copy the extents, and make a slighty puffed up version
  float testMins[3];
  float testMaxs[3];
  for (i = 0; i < 3; i++) {
    mins[i] = _mins[i];
    maxs[i] = _maxs[i];
    testMins[i] = mins[i] - 0.1f;
    testMaxs[i] = maxs[i] + 0.1f;
  }

  // find all of the intersecting nodes
  listSize = 0;
  for (i = 0; i < _listSize; i++) {
    SceneNode* node = _list[i];
    if (node->inAxisBox (testMins, testMaxs)) {
      list[listSize] = node;
      listSize++;
    }
  }

  // count will remain as the total numbers of
  // scene nodes that intersect with this cell
  count = listSize;

  // resize the list to save space
  list = (SceneNode**) realloc (list, count * sizeof (SceneNode*));

  // return if this is a leaf node
  if (((int)depth >= maxDepth) || (listSize <= minElements)) {
    DEBUG4 ("LEAF NODE: depth = %d, items = %i\n", depth, count);
    leafNodes++;
    totalNodes++;
    totalElements += listSize;
    resizeCell();
    return;
  }

  // sow the seeds
  depth++;
  makeChildren();

  // non NULLs first
  squeezeChildren (children);

  // resize this branch cell
  resizeCell();

  // leave some lists for FullyVisible grabs
  if (((depth + 1) % fullListBreak) != 0) {
    listSize = 0;
    free (list);
    list = NULL;
  }
  else {
    totalElements += listSize;
  }

  // tally ho
  totalNodes++;

  DEBUG4 ("BRANCH NODE: depth = %d, children = %i\n", depth, childCount);

  return;
}


void OctreeNode::makeChildren ()
{
  int side[3];    // the axis sides  (0 or 1)
  float cmins[3];
  float cmaxs[3];
  float center[3];

  // setup the center point
  for (int i = 0; i < 3; i++) {
    center[i] = 0.5f * (maxs[i] + mins[i]);
  }

  const float* extentSet[3] = { mins, center, maxs };

  for (side[0] = 0; side[0] < 2; side[0]++) {
    for (side[1] = 0; side[1] < 2; side[1]++) {
      for (side[2] = 0; side[2] < 2; side[2]++) {

        // calculate the child's extents
        for (int a = 0; a < 3; a++) {
          cmins[a] = extentSet[side[a]+0][a];
          cmaxs[a] = extentSet[side[a]+1][a];
        }

        int kid = side[0] + (2 * side[1]) + (4 * side[2]);

        children[kid] = new OctreeNode (depth, cmins, cmaxs, list, count);

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

  return;
}


void OctreeNode::resizeCell()
{
  int i;

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

  if (OcclMgr.occlude (mins, maxs, count) == Contained) {
    return;
  }

  if (level == Contained) {
    getFullyVisible ();
    return;
  }

  // this cell is only partially contained within
  // the frustum and is not being fully occluded

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
  // FIXME - faster then the recursive version?

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


void OctreeNode::getShadowList () const
{
  IntersectLevel level = testAxisBoxOcclusion (mins, maxs, 
                                               ShadowPlanes, ShadowCount);
  if (level == Outside) {
    return;
  }

  if (level == Contained) {
    getFullyShadow ();
    return;
  }

  // this cell is only partially contained within the shadow
  // volume we'll have to test all of its sub-cells as well.

  if (childCount > 0) {
    for (int i = 0; i < childCount; i++) {
      children[i]->getShadowList ();
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


void OctreeNode::getFullyShadow () const
{
  if ((childCount > 0) && (((depth + 1) % fullListBreak) != 0)) {
    for (int i = 0; i < childCount; i++) {
      children[i]->getFullyShadow ();
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


void OctreeNode::getExtents(float* _mins, float* _maxs) const
{
  memcpy (_mins, mins, 3 * sizeof(float));
  memcpy (_maxs, maxs, 3 * sizeof(float));
  return;
}


void OctreeNode::draw ()
{
  GLfloat red[4] = {1.0f, 0.0f, 0.0f, 0.75f};    // red
  GLfloat blue[4] = {0.0f, 0.0f, 1.0f, 0.75f};   // blue
  GLfloat green[4] = {0.0f, 1.0f, 0.0f, 0.75f};  // green
  GLfloat yellow[4] = {1.0f, 1.0f, 0.0f, 0.75f}; // yellow
  GLfloat purple[4] = {1.0f, 0.0f, 1.0f, 0.75f}; // purple
  GLfloat *color = purple;
  int x, y, z, c;
  float points[5][3];
  IntersectLevel frustumCull = Contained;
  bool occludeCull = false;

  if (CullFrustum != NULL) {
    frustumCull = testAxisBoxInFrustum (mins, maxs, CullFrustum);
    occludeCull = OcclMgr.occludePeek (mins, maxs);
  }

  // choose the color
  switch (frustumCull) {
    case Outside:
      color = purple;
      break;
    case Partial:
      if (!occludeCull) {
        color = blue;
      } else {
        color = green;
      }
      break;
    case Contained:
      if (!occludeCull) {
        color = red;
      } else {
        color = yellow;
      }
      break;
  }
  glColor4fv (color);

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
    glBegin (GL_LINE_STRIP);
    for (int i = 0; i < 5; i++) {
      glVertex3fv (points[i]);
    }
    glEnd ();
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
    glBegin (GL_LINE_STRIP);
    glVertex3fv (points[0]);
    glVertex3fv (points[1]);
    glEnd ();
  }

  // draw the kids
  for (c = 0; c < childCount; c++) {
    children[c]->draw ();
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
