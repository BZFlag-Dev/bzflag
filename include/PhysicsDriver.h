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

#ifndef _PHYSICS_DRIVER_H_
#define _PHYSICS_DRIVER_H_


#include <string>
#include <vector>
#include <iostream>


class PhysicsDriver {
  public:
    PhysicsDriver();
    ~PhysicsDriver();

    bool setName(const std::string& name);
    void setVelocity(const float vel[3]);
    void setAngular(float angleVel, const float pos[2]);
    void setRadial(float radialVel, const float pos[2]);
    void setSlideTime(float slideTime);
    void setDeathMessage(const std::string& msg);

    void finalize();
    void update(float time);

    const std::string& getName() const;
    const float* getVelocity() const;
    const float getAngularVel() const;
    const float* getAngularPos() const;
    const float getRadialVel() const;
    const float* getRadialPos() const;
    const bool getIsSlide() const;
    const float getSlideTime() const;
    const bool getIsDeath() const;
    const std::string& getDeathMsg() const;

    void* pack(void*);
    void* unpack(void*);
    int packSize();

    void print(std::ostream& out, int level);

  private:
    static const float minPeriod;

    std::string name;
    float velocity[3];
    float angularVel;
    float angularPos[2];
    float radialVel;
    float radialPos[2];
    bool slide;
    float slideTime;
    bool death;
    std::string deathMsg;
};

inline const float* PhysicsDriver::getVelocity() const
{
  return velocity;
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

    void* pack(void*);
    void* unpack(void*);
    int packSize();

    void print(std::ostream& out, int level);

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
