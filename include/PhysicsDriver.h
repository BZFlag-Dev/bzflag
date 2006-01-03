/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _PHYSICS_DRIVER_H_
#define _PHYSICS_DRIVER_H_

#include "common.h"

/* system interface headers */
#include <string>
#include <vector>
#include <iostream>


class PhysicsDriver {
  public:
    PhysicsDriver();
    ~PhysicsDriver();

    bool setName(const std::string& name);
    void setLinear(const float vel[3]);
    void setAngular(float angleVel, const float pos[2]);
    void setRadial(float radialVel, const float pos[2]);
    void setSlideTime(float slideTime);
    void setDeathMessage(const std::string& msg);

    void finalize();
    void update(float time);

    const std::string& getName() const;
    const float* getLinearVel() const;
    const float getAngularVel() const;
    const float* getAngularPos() const;
    const float getRadialVel() const;
    const float* getRadialPos() const;
    const bool getIsSlide() const;
    const float getSlideTime() const;
    const bool getIsDeath() const;
    const std::string& getDeathMsg() const;

    int packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  private:
    static const float minPeriod;

    std::string name;
    float linear[3];
    float angularVel;
    float angularPos[2];
    float radialVel;
    float radialPos[2];
    bool slide;
    float slideTime;
    bool death;
    std::string deathMsg;
};

inline const float* PhysicsDriver::getLinearVel() const
{
  return linear;
}
inline const float PhysicsDriver::getAngularVel() const
{
  return angularVel;
}
inline const float* PhysicsDriver::getAngularPos() const
{
  return angularPos;
}
inline const float PhysicsDriver::getRadialVel() const
{
  return radialVel;
}
inline const float* PhysicsDriver::getRadialPos() const
{
  return radialPos;
}
inline const bool PhysicsDriver::getIsSlide() const
{
  return slide;
}
inline const float PhysicsDriver::getSlideTime() const
{
  return slideTime;
}
inline const bool PhysicsDriver::getIsDeath() const
{
  return death;
}
inline const std::string& PhysicsDriver::getDeathMsg() const
{
  return deathMsg;
}


class PhysicsDriverManager {
  public:
    PhysicsDriverManager();
    ~PhysicsDriverManager();
    void update();
    void clear();
    int addDriver(PhysicsDriver* driver);
    int findDriver(const std::string& name) const;
    const PhysicsDriver* getDriver(int id) const;

    int packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  private:
    std::vector<PhysicsDriver*> drivers;
};

inline const PhysicsDriver* PhysicsDriverManager::getDriver(int id) const
{
  if ((id >= 0) && (id < (int)drivers.size())) {
    return drivers[id];
  } else {
    return NULL;
  }
}


extern PhysicsDriverManager PHYDRVMGR;


#endif //_PHYSICS_DRIVER_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

