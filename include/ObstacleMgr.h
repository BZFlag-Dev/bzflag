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

#ifndef	BZF_OBSTACLE_MGR_H
#define	BZF_OBSTACLE_MGR_H

#include "common.h"

// system headers
#include <string>
#include <vector>
#include <iostream>

// common headers
#include "ObstacleList.h"
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
class ObstacleModifier;
class BzMaterial;


//
// Group Instance
// - uses a group definition and a transform to produce obstacles
//

class GroupInstance {

  friend class ObstacleModifier;

  public:
    GroupInstance(const std::string& groupdef);
    GroupInstance();
    ~GroupInstance();
    void init();

    void setName(const std::string& name);
    void setTeam(int team);
    void setTint(const float tint[4]);
    void setPhysicsDriver(int phydrv);
    void setTransform(const MeshTransform&);
    void setMaterial(const BzMaterial*);
    void setDriveThrough();
    void setShootThrough();

    const std::string& getName() const;

    const std::string& getGroupDef() const;
    const MeshTransform& getTransform() const;

    void *pack(void*);
    void *unpack(void*);
    int packSize();

    void print(std::ostream& out, const std::string& indent) const;

  private:
    std::string groupdef;

    std::string name;
    MeshTransform transform;
    bool modifyTeam;
    int team;
    bool modifyColor;
    float tint[4];
    bool modifyPhysicsDriver;
    int phydrv;
    bool modifyMaterial;
    const BzMaterial* material;
    bool driveThrough;
    bool shootThrough;
};


//
// Group Definition
// - defines an obstacle group
//

class GroupDefinition {
  public:
    GroupDefinition(const std::string& name);
    ~GroupDefinition();

    enum ObstacleTypes {
      wallType = 0,
      boxType,
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

    void addObstacle(Obstacle* obstacle);
    void addGroupInstance(GroupInstance* group);

    void clear(); // delete the list and the obstacles
    void tighten(); // reduce memory usage

    void sort(int (*compare)(const void* a, const void* b));

    void makeGroups(const MeshTransform& xform,
		    const ObstacleModifier& obsMod) const;

    void replaceBasesWithBoxes();
    void deleteInvalidObstacles();

    const std::string& getName() const;
    const ObstacleList& getList(int type) const;
    const std::vector<GroupInstance*>& getGroups() const;

    // Get the list of meshes that came from the world file.
    // This includes the meshes in group definitions, even if
    // they have never been instantiated.
    void getSourceMeshes(std::vector<MeshObstacle*>& meshes) const;
    
    int packSize() const;
    void *pack(void*) const;
    void *unpack(void*);

    void printGrouped(std::ostream& out, const std::string& indent) const;
    void printFlatFile(std::ostream& out, const std::string& indent) const;

  public:
    static void clearDepthName();

  private:
    Obstacle* newObstacle(int type);
    void makeTeleName(Obstacle* obs, unsigned int pos) const;
    void appendGroupName(const GroupInstance* group) const;

  private:
    std::string name;

    ObstacleList lists[ObstacleTypeCount];
    std::vector<GroupInstance*>		groups;

    mutable bool active; // for recursion checking

  private:
    static std::string depthName;
};

inline const std::string& GroupDefinition::getName() const
{
  return name;
}
inline const ObstacleList& GroupDefinition::getList(int type) const
{
  return lists[type];
}
inline const std::vector<GroupInstance*>& GroupDefinition::getGroups() const
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

    void clear(); // delete the lists and the obstacles
    void tighten(); // reduce memory usage
    void makeWorld(); // make the local obstacles for the groups
    void replaceBasesWithBoxes();

    void addWorldObstacle(Obstacle* obstacle);
    void addGroupDef(GroupDefinition* groupdef);

    GroupDefinition* findGroupDef(const std::string& name) const;
    
    // Get the list of meshes that came from the world file.
    // This includes the meshes in group definitions, even if
    // they have never been instantiated.
    void getSourceMeshes(std::vector<MeshObstacle*>& meshes) const;

    const GroupDefinition* getWorld() const;

    // convenience functions
    const ObstacleList& getWalls() const;
    const ObstacleList& getBoxes() const;
    const ObstacleList& getPyrs() const;
    const ObstacleList& getBases() const;
    const ObstacleList& getTeles() const;
    const ObstacleList& getMeshes() const;
    const ObstacleList& getArcs() const;
    const ObstacleList& getCones() const;
    const ObstacleList& getSpheres() const;
    const ObstacleList& getTetras() const;

    int packSize() const;
    void *pack(void*) const;
    void *unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  private:
    GroupDefinition world;
    std::vector<GroupDefinition*> list;
};

inline const GroupDefinition* GroupDefinitionMgr::getWorld() const
{
  return &world;
}

inline const ObstacleList& GroupDefinitionMgr::getWalls() const
{
  return world.getList(GroupDefinition::wallType);
}
inline const ObstacleList& GroupDefinitionMgr::getBoxes() const
{
  return world.getList(GroupDefinition::boxType);
}
inline const ObstacleList& GroupDefinitionMgr::getPyrs() const
{
  return world.getList(GroupDefinition::pyrType);
}
inline const ObstacleList& GroupDefinitionMgr::getBases() const
{
  return world.getList(GroupDefinition::baseType);
}
inline const ObstacleList& GroupDefinitionMgr::getTeles() const
{
  return world.getList(GroupDefinition::teleType);
}
inline const ObstacleList& GroupDefinitionMgr::getMeshes() const
{
  return world.getList(GroupDefinition::meshType);
}
inline const ObstacleList& GroupDefinitionMgr::getArcs() const
{
  return world.getList(GroupDefinition::arcType);
}
inline const ObstacleList& GroupDefinitionMgr::getCones() const
{
  return world.getList(GroupDefinition::coneType);
}
inline const ObstacleList& GroupDefinitionMgr::getSpheres() const
{
  return world.getList(GroupDefinition::sphereType);
}
inline const ObstacleList& GroupDefinitionMgr::getTetras() const
{
  return world.getList(GroupDefinition::tetraType);
}


extern GroupDefinitionMgr OBSTACLEMGR;


#endif // BZF_OBSTACLE_MGR_H


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

