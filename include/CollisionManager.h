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
 
#ifndef	__COLLISION_GRID__
#define	__COLLISION_GRID__

#include "common.h"

/* system interface headers */
#include <vector>

/* common interface headers */
#include "Ray.h"
#include "Obstacle.h"
#include "WallObstacle.h"
#include "BoxBuilding.h"
#include "PyramidBuilding.h"
#include "TetraBuilding.h"
#include "BaseBuilding.h"
#include "Teleporter.h"


typedef struct {
  int count;
  Obstacle** list;
} ObsList;

typedef union {
  ObsList array[6];
  struct {
    ObsList walls;
    ObsList boxes;
    ObsList bases;
    ObsList pyrs;
    ObsList tetras;
    ObsList teles;
  } named;
} SplitObsList;


// well you know my name is Simon, and I like to do drawings
typedef void (*DrawLinesFunc)(int pointCount, const float (*points)[3]);


class CollisionManager {

  public:

    CollisionManager();
    ~CollisionManager();
    
    void load (std::vector<WallObstacle>    &walls,
               std::vector<BoxBuilding>     &boxes,
               std::vector<BaseBuilding>    &bases,
               std::vector<PyramidBuilding> &pyrs,
               std::vector<TetraBuilding>   &tetras,
               std::vector<Teleporter>      &teles);
    
    // some basics
    bool needReload() const;         // octree parameter has changed
    int getObstacleCount() const;    // total number of obstacles
    float getMaxWorldHeight() const; // maximum Z level in the world
    

    // test against an axis aligned bounding box
    const ObsList *axisBoxTest (const float *mins, const float* maxs) const;
    
    // test against a cylinder
    const ObsList *cylinderTest (const float *pos,
                                 float radius, float height) const;
    // test against a box
    const ObsList *boxTest (const float* pos, float angle,
                            float dx, float dy, float dz) const;
    // test against a moving box
    const ObsList *movingBoxTest (const float* oldPos, float oldAngle,
                                  const float* pos, float angle,
                                  float dx, float dy, float dz) const;
    // test against a Ray
    const ObsList *rayTest (const Ray* ray) const;
    
                
    // test against a box and return a split list
    //const SplitObsList *boxTestSplit (const float* pos, float angle,
    //                                  float dx, float dy, float dz) const;

    // drawing function
    void draw (DrawLinesFunc drawLinesFunc);

  private:
  
    void clear ();                  // reset the state 
    void setExtents(ObsList* list); // gather the extents

    class ColDetNode* root;   // the root of the octree

    float mins[3];
    float maxs[3];
    
    float WorldSize;
};


class ColDetNode {
  public:
    ColDetNode(unsigned char depth,
               const float* mins, const float* maxs,
               ObsList *fullList);
    ~ColDetNode();
    
    int getCount();
    
    // these fill in the FullList return list
    void axisBoxTest (const float* mins, const float* maxs) const;
    void boxTest (const float* pos, float angle, float dx, float dy, float dz) const;
    void rayTest (const Ray* ray) const;

    // this fills in the SplitList return list
    // (FIXME: not yet implemented, boxTestSplit might be useful for radar)
    //void boxTestSplit (const float* pos, float angle, float dx, float dy, float dz) const;
    
    // make some pretty drawings
    void draw(DrawLinesFunc drawLinesFunc);

  private:
    void makeChildren ();
    void resizeCell ();
    
    unsigned char depth;
    int count;
    float mins[3];
    float maxs[3];
    unsigned char childCount;
    ColDetNode* children[8];
    ObsList fullList;
};



inline int ColDetNode::getCount()
{
  return count;
}


inline int CollisionManager::getObstacleCount() const
{
  if (root == NULL) {
    return 0;
  } else {
    return root->getCount();
  }
}


inline float CollisionManager::getMaxWorldHeight() const
{
  return maxs[2];
}


#endif /* __COLLISION_GRID__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
