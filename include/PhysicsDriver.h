/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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

// system headers
#include <string>
#include <vector>
#include <map>
#include <iostream>

// common headers
#include "vectors.h"


class PhysicsDriver {

  friend class PhysicsDriverManager;

  public:
    PhysicsDriver();
    ~PhysicsDriver();

    bool setName(const std::string& name);
    void setLinear(const fvec3& vel);
    void setAngular(float angleVel, const fvec2& pos);
    void setRadial(float radialVel, const fvec2& pos);
    void setSlideTime(float slideTime);
    void setDeathMessage(const std::string& msg);

    void finalize();
    void update(float time);

    inline const std::string& getName() const { return name; }
    inline int                getID()   const { return id; }
    inline const fvec3& getLinearVel()  const { return linearVel; }
    inline float        getAngularVel() const { return angularVel; }
    inline const fvec2& getAngularPos() const { return angularPos; }
    inline float        getRadialVel()  const { return radialVel; }
    inline const fvec2& getRadialPos()  const { return radialPos; }
    inline bool         getIsSlide()    const { return slideTime > 0.0f; }
    inline float        getSlideTime()  const { return slideTime; }
    inline bool         getIsDeath()    const { return !deathMsg.empty(); }
    inline const std::string& getDeathMsg() const { return deathMsg; }

    bool operator<(const PhysicsDriver& pd) const;    

    int packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  private:
    std::string name;
    int id;
    fvec3 linearVel;
    float angularVel;
    fvec2 angularPos;
    float radialVel;
    fvec2 radialPos;
    float slideTime;
    std::string deathMsg;
};


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
    std::map<std::string, int>  nameMap;
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
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
