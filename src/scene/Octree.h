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

#ifndef __OCTREE_H__
#define __OCTREE_H__

#include "SceneNode.h"
#include "Frustum.h"


class OctreeNode;


class Octree {

  public:

    Octree();
    ~Octree();

    void addNodes (SceneNode** list, int listSize, int depth, int elements);

    int getFrustumList (SceneNode** list, int listSize,
                        const Frustum* frustum) const;
    int getShadowList (SceneNode** list, int listSize,
                       const Frustum* frustum, const float* sun) const;
    void clear ();
    void draw () const;


  private: // methods

    void getExtents(float* mins, float* maxs,
                    SceneNode** list, int listSize);

  private: // data

    OctreeNode* root;
};



class OctreeNode {

  public:

    OctreeNode(unsigned char depth,
               const float* mins, const float* maxs,
               SceneNode** list, int listSize);
    ~OctreeNode();

    void getFrustumList () const;
    void getShadowList () const;
    void getFullyVisible () const;
    void getFullyShadow () const;
    OctreeNode* getChild (int child);

    int getCount() const;    // number of nodes in this and subnodes
    int getChildren() const; // number of children
    int getListSize() const; // number of nodes in this node
    SceneNode** getList() const;     // list of nodes
    void getExtents(float* mins, float* maxs) const;

    void tallyStats();
    void draw ();

  private:

    void makeChildren ();
    void resizeCell ();

    enum CullLevel {
      NoCull,
      PartialCull,
      FullCull
    };

    unsigned char depth;
    float mins[3];
    float maxs[3];
    unsigned char childCount;
    OctreeNode* children[8];
    int count;  // number of nodes in this and subnodes
    int listSize;
    SceneNode** list;
};


inline int OctreeNode::getCount() const
{
  return count;
}

inline SceneNode** OctreeNode::getList() const
{
  return list;
}

inline int OctreeNode::getListSize() const
{
  return listSize;
}

inline int OctreeNode::getChildren() const
{
  return childCount;
}

inline OctreeNode* OctreeNode::getChild (int child)
{
  return children[child];
}


#endif // __OCTREE_H__

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
