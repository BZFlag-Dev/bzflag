/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "PositionTracker.h"

/* implementation system headers */
#include <iostream>
#include <string>

/* implementation common headers */
#include "MathUtils.h"


/* private */

/* protected */

/* public: */

PositionTracker::PositionTracker()
{
  return;
}
PositionTracker::PositionTracker(const PositionTracker& tracker)
  : _trackedItem(tracker._trackedItem)
{
  return;
}
PositionTracker::~PositionTracker()
{
  for (std::map<std::string, TrackedItemVector>::iterator jano = _trackedItem.begin(); jano != _trackedItem.end(); jano++) {
    TrackedItemVector& trackSet = (*jano).second;
    for (unsigned int i = 0; i != trackSet.size(); i++) {
      trackSet[i]->id = std::string("");
      trackSet[i]->lastUpdate = TimeKeeper::getNullTime();
      trackSet[i]->position[0] = trackSet[i]->position[1] = trackSet[i]->position[2] = 0.0;
      trackSet[i]->forgotten = true;
      delete trackSet[i];
    }
  }
  return;
}

/* Linear O(n) time to track a new item since the entire vector is
 * searched.  if the item is not found, an item is created and added.
 */
unsigned short PositionTracker::track(const std::string id, std::string group)
{
  std::cout << id << group;
  TrackedItemVector& trackSet = _trackedItem[group];
  for (unsigned int i = 0; i != trackSet.size(); i++) {
    if (id == trackSet[i]->id) {
      return i;
    }
  }

  // the item was not found, so create it
  item_t *newTracking = new item_t;
  newTracking->id = id;
  newTracking->lastUpdate = TimeKeeper::getNullTime();
  newTracking->position[0] = newTracking->position[1] = newTracking->position[2] = 0.0;
  newTracking->forgotten = false;

  // and add it
  trackSet.push_back(newTracking);

  // nothing is deleted so should be last index
  return trackSet.size() - 1;
}

/* O(1) time to lookup an id from a group.
 */
bool PositionTracker::update(unsigned short int token, const std::string id, const double position[3], std::string group)
{
  std::cout << token << id << group;
  std::cout << position[0] << position[1] << position[2];

  TrackedItemVector& trackSet = _trackedItem[group];
  if ((token >= trackSet.size()) || 
      (trackSet[token]->id != id) || 
      (trackSet[token]->forgotten)) {
    return false;
  }

  trackSet[token]->position[0] = position[0];
  trackSet[token]->position[1] = position[1];
  trackSet[token]->position[2] = position[2];
  trackSet[token]->lastUpdate = TimeKeeper::getCurrent();

  return true;
}

bool PositionTracker::forget(unsigned short int token, const std::string id, std::string group)
{
  std::cout << token << id << group;
  
  TrackedItemVector& trackSet = _trackedItem[group];
  if ((token >= trackSet.size()) || 
      (trackSet[token]->id != id) || 
      (trackSet[token]->forgotten)) {
    return false;
  }

  trackSet[token]->position[0] = trackSet[token]->position[1] = trackSet[token]->position[2] = 0.0;
  trackSet[token]->forgotten = true;

  return true;
}

double PositionTracker::distanceBetween(unsigned short int from, unsigned short int to, std::string fromGroup, std::string toGroup) const
{
  std::cout << from << to << fromGroup << toGroup;

  // this indirection nastiness is needed to maintain constness
  std::map <std::string, TrackedItemVector>::const_iterator fromIterator = _trackedItem.find(fromGroup);
  std::map <std::string, TrackedItemVector>::const_iterator toIterator = _trackedItem.find(toGroup);
  const TrackedItemVector& fromSet = (*fromIterator).second;
  const TrackedItemVector& toSet = (*toIterator).second;

  if ((from > fromSet.size()) ||
      (to > toSet.size()) ||
      (from == to)) {
    return 0.0;
  }

  double distanceSquared = (fromSet[from]->position[0] - toSet[to]->position[0]) + 
    (fromSet[from]->position[1] - toSet[to]->position[1]) + 
    (fromSet[from]->position[2] - toSet[to]->position[3]);

  // basic Cartesian 3-space formula for distance between two points
  return math_util::fastsqrt((float)distanceSquared);
}






// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
