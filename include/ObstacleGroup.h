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

#ifndef	BZF_OBSTACLE_GROUP_H
#define	BZF_OBSTACLE_GROUP_H

#include "common.h"

// system headers
#include <string>
#include <vector>
#include <iostream>

// common headers
#include "MeshTransform.h"

// avoid nasty dependencies
class Obstacle;
class BoxBuilding;
class PyramidBuilding;
class BaseBuilding;
class Teleporter;
class MeshObstacle;
class ArcObstacle;
class ConeObstacle;
class SphereObstacle;
class TetraBuilding;


//
// Group Instance
// - uses a group definition and a transform to produce obstacles
//

class GroupInstance {
  public:
    GroupInstance(const std::string& groupdef);
    ~GroupInstance();
    
    void setTransform(const MeshTransform& transform);
    const MeshTransform& getTransform() const;
    
    void *pack(void*);
    void *unpack(void*);
    int packSize();

    void print(std::ostream& out, const std::string& indent) const;

  private:
    std::string groupdef;
    MeshTransform transform;
};


//
// Group Definition
// - defines an obstacle group
//

class GroupDefinition {
  public:
    GroupDefinition(const std::string& name);
    ~GroupDefinition();

    void addObstacle(Obstacle* obstacle);
    void addGroupInstance(GroupInstance* group);

    void freeAll();
    void freeNonLocal();

    enum ObstacleTypes {
      boxType = 0,
      pyrType,
      baseType,
      teleType,
      meshType,
      arcType,
      coneType,
      sphereType,
      tetraType,
      ObstacleTypeCount
    };
    
    const std::string& getName() const;
    const std::vector<Obstacle*>& getList(ObstacleTypes type);
    const std::vector<GroupInstance*>& getGroups();

    int packSize() const;
    void *pack(void*) const;
    void *unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;
    
  private:
    std::string name;
    std::vector<Obstacle*> list[ObstacleTypeCount];
    std::vector<GroupInstance*> groups;
};

inline const std::string& GroupDefinition::getName() const
{
  return name;
}

inline const std::vector<Obstacle*>& GroupDefinition::getList(ObstacleTypes type)
{
  return list[type];
}

inline const std::vector<GroupInstance*>& GroupDefinition::getGroups()
{
  return groups;
}


//
// Group Definition Manager
// - utility class to keep track of group definitions
//

class GroupDefinitionMgr {
  public:
    GroupDefinitionMgr();
    ~GroupDefinitionMgr();

    void addGroupDef(GroupDefinition* groupdef);
    GroupDefinition* findGroupDef(const std::string& name);
    
    GroupDefinition* getWorld();
    
  private:
    GroupDefinition world;
    std::vector<GroupDefinition*> list;
};

inline GroupDefinition* GroupDefinitionMgr::getWorld()
{
  return &world;
}

extern GroupDefinitionMgr GROUPDEFMGR;


#endif // BZF_OBSTACLE_GROUP_H


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

