/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "PositionTracker.h"

/* implementation system headers */
#include <iostream>
#include <string>

/* implementation common headers */

/* private */

/* protected */

/* public: */

PositionTracker::PositionTracker()
{
  return;
}
PositionTracker::PositionTracker(const PositionTracker& tracker)
  : _trackedItem(tracker._trackedItem),
    _waypointDistance(tracker._waypointDistance)
{
  return;
}
PositionTracker::~PositionTracker()
{
  // clear out all the tracked items vectors
  for (std::map<std::string, TrackedItemVector>::iterator jano = _trackedItem.begin(); jano != _trackedItem.end(); jano++) {
    TrackedItemVector& trackSet = (*jano).second;
    for (unsigned int i = 0; i != trackSet.size(); i++) {
      /* sanity clear */
      trackSet[i]->id = UNSET_ID;
      trackSet[i]->intID = 0;
      trackSet[i]->strID = "";
      trackSet[i]->lastUpdate = TimeKeeper::getNullTime();
      trackSet[i]->position[0] = trackSet[i]->position[1] = trackSet[i]->position[2] = 0.0;
      trackSet[i]->forgotten = true;
      delete trackSet[i];
    }
  }
  _trackedItem.clear();

  // clear out any waypoints
  _waypointDistance.clear();

  return;
}


/* Linear O(n) time to track a new item since the entire vector is
 * searched.  if the item is not found, an item is created and added.
 */
unsigned short PositionTracker::track(const std::string id, std::string group)
{
  TrackedItemVector& trackSet = _trackedItem[group];
  for (unsigned int i = 0; i != trackSet.size(); i++) {
    switch (trackSet[i]->id) {
      case STR_ID:
	if (id == trackSet[i]->strID) {
	  return i;
	}
	break;
      case INT_ID:
      case UNSET_ID:
	break;
      default:
	std::cerr << "Unknown tracker ID type (id == " << trackSet[i]->id << ")" << std::endl;
	break;
    }
  }

  // the item was not found, so create it
  item_t *newTracking = new item_t;
  newTracking->id = STR_ID;
  newTracking->intID = 0;
  newTracking->strID = id;
  newTracking->lastUpdate = TimeKeeper::getNullTime();
  newTracking->position[0] = newTracking->position[1] = newTracking->position[2] = 0.0;
  newTracking->forgotten = false;

  // and add it
  trackSet.push_back(newTracking);

  // nothing is deleted so should be last index
  return trackSet.size() - 1;
}


/* alternative track() passing our own id */
unsigned short PositionTracker::track(long int id, std::string group)
{
  TrackedItemVector& trackSet = _trackedItem[group];
  for (unsigned int i = 0; i != trackSet.size(); i++) {
    switch (trackSet[i]->id) {
      case INT_ID:
	if (id == trackSet[i]->intID) {
	  return i;
	}
	break;
      case STR_ID:
      case UNSET_ID:
	break;
      default:
	std::cerr << "Unknown tracker ID type (id == " << trackSet[i]->id << ")" << std::endl;
	break;
    }
  }

  // the item was not found, so create it
  item_t *newTracking = new item_t;
  newTracking->id = INT_ID;
  newTracking->intID = id;
  newTracking->strID = "";
  newTracking->lastUpdate = TimeKeeper::getNullTime();
  newTracking->position[0] = newTracking->position[1] = newTracking->position[2] = 0.0;
  newTracking->forgotten = false;

  // and add it
  trackSet.push_back(newTracking);

  // nothing is deleted so should be last index
  return trackSet.size() - 1;
}


bool PositionTracker::update(unsigned short int token, const std::string id, const double position[3], std::string group)
{
  TrackedItemVector& trackSet = _trackedItem[group];
  if ((token >= trackSet.size()) ||
      (trackSet[token]->id != STR_ID) ||
      (trackSet[token]->strID != id) ||
      (trackSet[token]->forgotten)) {
    return false;
  }

  trackSet[token]->position[0] = position[0];
  trackSet[token]->position[1] = position[1];
  trackSet[token]->position[2] = position[2];
  trackSet[token]->lastUpdate = TimeKeeper::getCurrent();

  return true;
}
bool PositionTracker::update(unsigned short int token, const std::string id, const float position[3], std::string group)
{
  double pos[3];
  pos[0] = (double)position[0];
  pos[1] = (double)position[1];
  pos[2] = (double)position[2];
  return update(token, id, pos, group);
}
bool PositionTracker::update(unsigned short int token, long int id, const double position[3], std::string group)
{
  TrackedItemVector& trackSet = _trackedItem[group];
  if ((token >= trackSet.size()) ||
      (trackSet[token]->id != INT_ID) ||
      (trackSet[token]->intID != id) ||
      (trackSet[token]->forgotten)) {
    return false;
  }

  trackSet[token]->position[0] = position[0];
  trackSet[token]->position[1] = position[1];
  trackSet[token]->position[2] = position[2];
  trackSet[token]->lastUpdate = TimeKeeper::getCurrent();

  return true;
}
bool PositionTracker::update(unsigned short int token, long int id, const float position[3], std::string group)
{
  double pos[3];
  pos[0] = (double)position[0];
  pos[1] = (double)position[1];
  pos[2] = (double)position[2];
  return update(token, id, pos, group);
}


bool PositionTracker::addWaypoint(const double from[3], const double to[3], double distance)
{
  unsigned short int fromToken = 0, toToken = 0;
  bool updated;
  bool foundPoint;
  unsigned int i;

  TrackedItemVector& waypointSet = _trackedItem[std::string("__waypoint__")];

  // see if the first point already exists
  foundPoint = false;
  fromToken  = 0;
  for (i = 0; i != waypointSet.size(); i++) {
    if ((waypointSet[i]->position[0] = from[0]) &&
	(waypointSet[i]->position[1] = from[1]) &&
	(waypointSet[i]->position[2] = from[2])) {
      foundPoint = true;
      fromToken = (unsigned short)waypointSet[i]->intID;
    }
  }
  if (!foundPoint) {
    // add the first point
    unsigned short int nextID = waypointSet.size();
    fromToken = track((long int)nextID, std::string("__waypoint__"));
    updated = update(fromToken, nextID, from, std::string("__waypoint__"));
    if (!updated) {
      std::cerr << "Unable to add waypoint?!" << std::endl;
      return false;
    }
  }

  // see if the second point already exists
  foundPoint = false;
  toToken    = 0;
  for (i = 0; i != waypointSet.size(); i++) {
    if ((waypointSet[i]->position[0] = to[0]) &&
	(waypointSet[i]->position[1] = to[1]) &&
	(waypointSet[i]->position[2] = to[2])) {
      foundPoint = true;
      toToken = (unsigned short)waypointSet[i]->intID;
    }
  }
  if (!foundPoint) {
    // add the second point
    unsigned short int nextID = waypointSet.size();
    toToken = track((long int)nextID, std::string("__waypoint__"));
    updated = update(toToken, nextID, to, std::string("__waypoint__"));
    if (!updated) {
      std::cerr << "Unable to add waypoint?!" << std::endl;
      return false;
    }
  }

  // compute and use real distance if distance passed was negative
  if (distance < 0.0) {
    distance = distanceBetween(fromToken, toToken, std::string("__waypoint__"), std::string("__waypoint__"));
  }
  std::pair<unsigned short int, unsigned short int> waypoint = std::make_pair(fromToken, toToken);

  // see if the waypoint pair have already been added
  /*
  std::map<std::pair<unsigned short int, unsigned short int>, double>::iterator jano = _waypointDistance.find(waypoint);
  if (jano != _waypointDistance.end()) {
    // waypoint already added, but set distance anyways (might be an update)
    (*jano).second = distance;
  }
  */
  _waypointDistance[waypoint] = distance;

  return true;
}
bool PositionTracker::addWaypoint(const float from[3], const float to[3], double distance)
{
  double fromPos[3], toPos[3];
  fromPos[0] = from[0];
  fromPos[1] = from[1];
  fromPos[2] = from[2];
  toPos[0] = to[0];
  toPos[1] = to[1];
  toPos[2] = to[2];
  return addWaypoint(fromPos, toPos, distance);
}


bool PositionTracker::forget(unsigned short int token, const std::string id, std::string group)
{
  TrackedItemVector& trackSet = _trackedItem[group];
  if ((token >= trackSet.size()) ||
      (trackSet[token]->id != STR_ID) ||
      (trackSet[token]->strID != id) ||
      (trackSet[token]->forgotten)) {
    return false;
  }

  trackSet[token]->position[0] = trackSet[token]->position[1] = trackSet[token]->position[2] = 0.0;
  trackSet[token]->forgotten = true;

  return true;
}
bool PositionTracker::forget(unsigned short int token, long int id, std::string group)
{
  TrackedItemVector& trackSet = _trackedItem[group];
  if ((token >= trackSet.size()) ||
      (trackSet[token]->id != INT_ID) ||
      (trackSet[token]->intID != id) ||
      (trackSet[token]->forgotten)) {
    return false;
  }

  trackSet[token]->position[0] = trackSet[token]->position[1] = trackSet[token]->position[2] = 0.0;
  trackSet[token]->forgotten = true;

  return true;
}


double PositionTracker::distanceBetween(unsigned short int fromToken, unsigned short int toToken, std::string fromGroup, std::string toGroup) const
{
  // this indirection nastiness is needed to maintain constness
  std::map <std::string, TrackedItemVector>::const_iterator fromIterator = _trackedItem.find(fromGroup);
  std::map <std::string, TrackedItemVector>::const_iterator toIterator = _trackedItem.find(toGroup);
  const TrackedItemVector& fromSet = (*fromIterator).second;
  const TrackedItemVector& toSet = (*toIterator).second;

  if ((fromToken > fromSet.size()) ||
      (toToken > toSet.size()) ||
      (fromToken == toToken)) {
    return 0.0;
  }

  // basic Cartesian 3-space formula for distance between two points
  double distanceSquared = (((fromSet[fromToken]->position[0] - toSet[toToken]->position[0]) *
			     (fromSet[fromToken]->position[0] - toSet[toToken]->position[0])) +
			    ((fromSet[fromToken]->position[1] - toSet[toToken]->position[1]) *
			     (fromSet[fromToken]->position[1] - toSet[toToken]->position[1])) +
			    ((fromSet[fromToken]->position[2] - toSet[toToken]->position[2]) *
			     (fromSet[fromToken]->position[2] - toSet[toToken]->position[2])));

  return sqrtf((float)distanceSquared);
}


double PositionTracker::waypointDistance(unsigned short int fromToken, unsigned short int toToken, std::string fromGroup, std::string toGroup) const
{
  double separationDistance = distanceBetween(fromToken, toToken, fromGroup, toGroup);
  double shortestDistance = separationDistance;

  // horrible linear search
  std::map<std::pair<unsigned short int, unsigned short int>, double>::const_iterator waypointIterator;
  for (waypointIterator = _waypointDistance.begin(); waypointIterator != _waypointDistance.end(); waypointIterator++) {
    double fromDist, toDist, waypointDist;
    fromDist = distanceBetween(fromToken, (*waypointIterator).first.first, fromGroup, std::string("__waypoint__"));
    toDist = distanceBetween((*waypointIterator).first.second, toToken, std::string("__waypoint__"), toGroup);
    waypointDist = fromDist + (*waypointIterator).second + toDist;
    if (waypointDist < shortestDistance) {
      shortestDistance = waypointDist;
    }
  }

  return shortestDistance;
}


unsigned short int PositionTracker::trackedCount(std::string group) const
{
  std::map <std::string, TrackedItemVector>::const_iterator i = _trackedItem.find(group);
  const TrackedItemVector& trackSet = (*i).second;

  return trackSet.size();
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
