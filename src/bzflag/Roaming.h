/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* bzflag common header */
#include "common.h"

/* system headers */
#include <string>

/* common headers */
#include "Singleton.h"
#include "Player.h"
#include "Flag.h"

/* local headers */
#include "World.h"
#include "Roster.h"

#define ROAM (Roaming::instance())

class Roaming : public Singleton<Roaming> {
public:
  Roaming(); // c'tor

  enum RoamingView {
    roamViewDisabled = 0,
    roamViewFree,
    roamViewTrack,
    roamViewFollow,
    roamViewFP,
    roamViewFlag,
    roamViewCount
  };
  bool isRoaming(void) const;
  RoamingView getMode(void) const;
  void setMode(RoamingView newView);

  enum RoamingTarget {
    next = 0,
    previous,
    explicitSet
  };
  void changeTarget(RoamingTarget target, int explicitIndex = 0);
  /* if view is in any mode in which they are not valid, 
     getTargetTank and getTargetFlag will return NULL.  Otherwise
     they return the index of the object that you're
     tracking/following/driving with */
  Player* getTargetTank(void) const;
  Flag*   getTargetFlag(void) const;

  std::string getRoamingLabel(void) const;

  struct RoamingCamera {
    float pos[3];
    float theta;
    float phi;
    float zoom;
  };
  void setCamera(RoamingCamera* newCam);
  void resetCamera(void);
  /* note that dc is a camera structure of *changes* (thus dc)
     not new values */
  void updatePosition(RoamingCamera* dc, float dt);
  const RoamingCamera* const getCamera(void) const;
  void setZoom(float newZoom);
  float getZoom(void) const;

protected:
  friend class Singleton<Roaming>;

private:
  void findWinner(void);
  void buildRoamingLabel(void);

private:
  RoamingView view;
  RoamingCamera camera;
  int targetManual;
  int targetWinner;
  int targetFlag;
  std::string roamingLabel;
};

inline bool Roaming::isRoaming(void) const {
  return (view > roamViewDisabled);
}

inline Roaming::RoamingView Roaming::getMode(void) const {
  return view;
}

inline float Roaming::getZoom() const {
  return camera.zoom;
}

inline void Roaming::setZoom(float newZoom) {
  camera.zoom = newZoom;
}

inline std::string Roaming::getRoamingLabel(void) const {
  return roamingLabel;
}

inline Player* Roaming::getTargetTank() const {
  return getPlayerByIndex(targetWinner);
}

inline Flag* Roaming::getTargetFlag() const {
  return &(World::getWorld()->getFlag(targetFlag));
}

inline const Roaming::RoamingCamera* const Roaming::getCamera() const {
  return &camera;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
