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

/* interface header */
#include "CollisionManager.h"

/* system implementation headers */
#include <vector>
#include <math.h>

/* common implementation headers */
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "Obstacle.h"




CollisionManager::CollisionManager ()
{
  WorldSize = 0.0f;
}

CollisionManager::~CollisionManager ()
{
  clear();
}

void CollisionManager::clear ()
{
  WorldSize = 0.0f;
  int x, y;
  for (x=0; x<GridSizeX; x++) {
    for (y=0; y<GridSizeY; y++) {
      Cells[x][y].count = 0;
      Cells[x][y].objs.clear();
    }
  }
  return;
}

float CollisionManager::getWorldSize () const
{
  return WorldSize;
}

CollisionManager::CellList
CollisionManager::getCells (const float *pos, float radius) const
{
  // We're going to play this one fast and loose.
  // Just see which cells the maximal box intersects,
  // no fancy math to figure out where an actual 
  // cylinder would intersect.
  
  int minX = (int) ((pos[0] - radius + (WorldSize / 2.0f)) / Sx);
  int maxX = (int) ((pos[0] + radius + (WorldSize / 2.0f)) / Sx);
 
  int minY = (int) ((pos[1] - radius + (WorldSize / 2.0f)) / Sy);
  int maxY = (int) ((pos[1] + radius + (WorldSize / 2.0f)) / Sy);

  if (minX < 0) {
    minX = 0;
  }
  if (minY < 0) {
    minY = 0;
  }
  if (maxX >= GridSizeX) {
    maxX = (GridSizeX - 1);
  }
  if (maxY >= GridSizeY) {
    maxY = (GridSizeY - 1);
  }
  
  // make the list
  CellList clist;
  
  for (int x = minX; x <= maxX; x++) {
    for (int y = minY; y <= maxY; y++) {
      if (Cells[x][y].count > 0) {
        clist.push_back(&Cells[x][y]);
      }
    }
  }
  
  return clist;
}

ObstacleList CollisionManager::getObstacles (const float *pos, float radius) const
{
  ObstacleList olist;

  CellList clist = getCells (pos, radius);
  
  for (CellList::const_iterator cit = clist.begin();
       cit != clist.end(); cit++) {
    const GridCell* cell = *cit;
    for (ObstacleList::const_iterator oit = cell->objs.begin();
         oit != cell->objs.end(); oit++) {
      Obstacle* obs = *oit;
      // if collisionState is true, then this obstacle
      // has already been added to the return list.
      if (!obs->collisionState) {
        olist.push_back(obs);
        obs->collisionState = true;
      }
    }
  }
  
  // clear the collisionState on the obstacles
  for (ObstacleList::iterator clear_it = olist.begin();
       clear_it != olist.end(); clear_it++) {
    (*clear_it)->collisionState = false;
  }

  return olist;  
}

ObstacleList CollisionManager::getObstacles (const float* pos, float angle,
                                             float dx, float dy) const
{
  angle = angle;  // remove warnings

  float radius = sqrtf (dx*dx + dy*dy);
  return getObstacles (pos, radius);
}

ObstacleList CollisionManager::getObstacles (const float* oldPos, float oldAngle,
                                             const float* pos, float angle,
                                             float dx, float dy) const
{
  // FIXME - sort of a bogus call, this particular
  // style of collision detection is only used to 
  // check for Z variations (check obstacles/BoxBuilding.cxx)
  
  angle = angle;  // remove warnings
  oldPos = oldPos;
  oldAngle = oldAngle;
  
  float radius = sqrtf (dx*dx + dy*dy);
  return getObstacles (pos, radius);
}                                  

void CollisionManager::load (std::vector<BoxBuilding>     &boxes,
                             std::vector<PyramidBuilding> &pyrs,
                             std::vector<Teleporter>      &teles,
                             std::vector<BaseBuilding>    &bases)
{
  clear(); // clean out the cell lists

  const float Heaven = 1.0e30f; // very tall

  WorldSize = BZDB.eval (StateDatabase::BZDB_WORLDSIZE);
  Sx = WorldSize / float (GridSizeX);
  Sy = WorldSize / float (GridSizeY);
  Hx = Sx / 2.0f;  // half the cell size
  Hy = Sy / 2.0f;  // half the cell size
  Offx = -(WorldSize / 2.0f) + Hx;
  Offy = -(WorldSize / 2.0f) + Hy;
  
  // setup the cell positions
  int x, y;
  for (x=0 ; x < GridSizeX; x++) {
    for (y=0 ; y < GridSizeY; y++) {
      Cells[x][y].pos[0] = Offx + (Sx * (float)x);
      Cells[x][y].pos[1] = Offy + (Sy * (float)y);
      Cells[x][y].pos[2] = 0.0f;
      Cells[x][y].count = 1;    // fake the count for getCells()
    }
  }
  
  CellList::const_iterator cit;

  // Boxes
  for (std::vector<BoxBuilding>::iterator it_box = boxes.begin();
       it_box != boxes.end(); it_box++) {
    BoxBuilding* box = (BoxBuilding *) &(*it_box);
    box->collisionState = false;
    float dx = box->getWidth();
    float dy = box->getBreadth();
    float radius = sqrtf (dx*dx + dy*dy);
    CellList list = getCells (box->getPosition(), radius);
    for (cit = list.begin(); cit != list.end(); cit++) {
      GridCell* cell = (GridCell*) (*cit);
      if (box->inBox(cell->pos, 0.0f, Hx, Hy, Heaven)) {
        cell->objs.push_back( &(*it_box) );
      }
    }
  }

  // Pyramids  
  for (std::vector<PyramidBuilding>::iterator it_pyr = pyrs.begin();
       it_pyr != pyrs.end(); it_pyr++) {
    PyramidBuilding* pyr = (PyramidBuilding *) &(*it_pyr);
    pyr->collisionState = false;
    float dx = pyr->getWidth();
    float dy = pyr->getBreadth();
    float radius = sqrtf (dx*dx + dy*dy);
    CellList list = getCells (pyr->getPosition(), radius);
    for (cit = list.begin(); cit != list.end(); cit++) {
      GridCell* cell = (GridCell*) (*cit);
      if (pyr->inBox(cell->pos, 0.0f, Hx, Hy, Heaven)) {
        cell->objs.push_back( &(*it_pyr) );
      }
    }
  }
  
  // Teleporters
  for (std::vector<Teleporter>::iterator it_tele = teles.begin();
       it_tele != teles.end(); it_tele++) {
    Teleporter* tele = (Teleporter *) &(*it_tele);
    tele->collisionState = false;
    float dx = tele->getWidth();
    float dy = tele->getBreadth();
    float radius = sqrtf (dx*dx + dy*dy);
    CellList list = getCells (tele->getPosition(), radius);
    for (cit = list.begin(); cit != list.end(); cit++) {
      GridCell* cell = (GridCell*) (*cit);
      if (tele->inBox(cell->pos, 0.0f, Hx, Hy, Heaven)) {
        cell->objs.push_back( &(*it_tele) );
      }
    }
  }
  
  // Bases
  for (std::vector<BaseBuilding>::iterator it_base = bases.begin();
       it_base != bases.end(); it_base++) {
    BaseBuilding* base = (BaseBuilding *) &(*it_base);
    base->collisionState = false;
    float dx = base->getWidth();
    float dy = base->getBreadth();
    float radius = sqrtf (dx*dx + dy*dy);
    CellList list = getCells (base->getPosition(), radius);
    for (cit = list.begin(); cit != list.end(); cit++) {
      GridCell* cell = (GridCell*) (*cit);
      if (base->inBox(cell->pos, 0.0f, Hx, Hy, Heaven)) {
        cell->objs.push_back( &(*it_base) );
      }
    }
  }
  
  for (x=0; x<GridSizeX; x++) {
    for (y=0; y<GridSizeY; y++) {
      Cells[x][y].count = (int)Cells[x][y].objs.size();
      DEBUG4 ("Cell(%4i)  objs = %2i  [%.3f, %.3f, %.3f]\n",
              GridSizeY*x + y, Cells[x][y].count,
              Cells[x][y].pos[0], Cells[x][y].pos[1], Cells[x][y].pos[2]);
    }
  }
  
  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
