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
    void setAngular(float angleVel, const float anglePos[2]);

    void finalize();
    void update(float time);

    const float* getVelocity() const;
    const float getAngularVel() const;
    const float* getAngularPos() const;
    const std::string& getName() const;

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

extern PhysicsDriverManager PHYDRVMGR;


#endif //_PHYSICS_DRIVER_H_
