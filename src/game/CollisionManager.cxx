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
      // FIXME - make sure there are no duplicates?
      olist.push_back(*oit);
    }
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

  // fake the tank dimensions to do cell detection
  std::string realTankHeight = BZDB.get (StateDatabase::BZDB_TANKHEIGHT);
  BZDB.setFloat (StateDatabase::BZDB_TANKHEIGHT, 1.0e30f); //very tall
  
  const float Heaven = 1.0e30f;

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
  for (std::vector<BoxBuilding>::const_iterator it_box = boxes.begin();
       it_box != boxes.end(); it_box++) {
    const BoxBuilding* box = (const BoxBuilding *) &(*it_box);
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
  for (std::vector<PyramidBuilding>::const_iterator it_pyr = pyrs.begin();
       it_pyr != pyrs.end(); it_pyr++) {
    const PyramidBuilding* pyr = (const PyramidBuilding *) &(*it_pyr);
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
  for (std::vector<Teleporter>::const_iterator it_tele = teles.begin();
       it_tele != teles.end(); it_tele++) {
    const Teleporter* tele = (const Teleporter *) &(*it_tele);
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
  for (std::vector<BaseBuilding>::const_iterator it_base = bases.begin();
       it_base != bases.end(); it_base++) {
    const BaseBuilding* base = (const BaseBuilding *) &(*it_base);
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
      Cells[x][y].count = Cells[x][y].objs.size();
      DEBUG4 ("Cell(%4i)  objs = %2i  [%.3f, %.3f, %.3f]\n",
              GridSizeY*x + y, Cells[x][y].count,
              Cells[x][y].pos[0], Cells[x][y].pos[1], Cells[x][y].pos[2]);
    }
  }

  // put the Tank Height back to normal
  BZDB.set (StateDatabase::BZDB_TANKHEIGHT, realTankHeight);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
