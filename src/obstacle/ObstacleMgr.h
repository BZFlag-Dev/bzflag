/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_OBSTACLE_MGR_H
#define BZF_OBSTACLE_MGR_H

#include "common.h"

// system headers
#include <string>
#include <vector>
#include <map>
#include <iostream>

// common headers
#include "obstacle/ObstacleList.h"
#include "obstacle/Obstacle.h"
#include "game/BzMaterial.h"
#include "game/MeshTransform.h"
#include "vectors.h"

// avoid nasty dependencies
class Obstacle;

class BaseBuilding;
class BoxBuilding;
class MeshObstacle;
class PyramidBuilding;
class ArcObstacle;
class ConeObstacle;
class SphereObstacle;
class Teleporter;

class LinkDef;
class MeshFace;
class ObstacleModifier;
class WorldText;


//============================================================================//
//
//  Group Instance
//  - uses a group definition and a transform to produce obstacles
//

class GroupInstance {

    friend class ObstacleModifier;

  public:
    typedef std::map<int, int>                  IntSwapMap;
    typedef std::map<std::string, std::string> TextSwapMap;

  public:
    GroupInstance(const std::string& groupDef);
    GroupInstance();
    ~GroupInstance();
    void init();

    void setName(const std::string& name);
    void setTeam(int team);
    void setTint(const fvec4& tint);
    void setPhysicsDriver(int phydrv);
    void setTransform(const MeshTransform&);
    void setMaterial(const BzMaterial*);
    void setDriveThrough();
    void setShootThrough();
    void setCanRicochet();
    void addMaterialSwap(const BzMaterial* src,
                         const BzMaterial* dst);
    void addPhydrvSwap(int srcID, int dstID);
    void addTextSwap(const std::string& src,
                     const std::string& dst);
    void addZoneSwap(const std::string& src,
                     const std::string& dst);
    void addWeaponSwap(const std::string& src,
                       const std::string& dst);

    const std::string& getName() const;

    const std::string& getGroupDef() const;
    const MeshTransform& getTransform() const;

    int   packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  private:
    std::string groupDef;

    std::string name;
    MeshTransform transform;
    bool modifyTeam;
    int team;
    bool modifyColor;
    fvec4 tint;
    bool modifyPhysicsDriver;
    int phydrv;
    bool modifyMaterial;
    const BzMaterial* material;
    unsigned char driveThrough;
    unsigned char shootThrough;
    bool          ricochet;

    MaterialMap matMap;
    IntSwapMap  phydrvMap;
    TextSwapMap textMap;
    TextSwapMap zoneMap;
    TextSwapMap weaponMap;
};


//============================================================================//
//
//  Group Definition
//  - defines an obstacle group
//

class GroupDefinition {
  public:
    GroupDefinition(const std::string& name);
    ~GroupDefinition();

    void addObstacle(Obstacle* obstacle);
    void addGroupInstance(GroupInstance* group);
    void addText(WorldText* text);
    void addLinkDef(LinkDef* linkDef);

    void clear(); // delete the list and the obstacles
    void tighten(); // reduce memory usage

    void sort(int (*compare)(const void* a, const void* b));

    void makeGroups(const MeshTransform& xform,
                    const ObstacleModifier& obsMod) const;

    void replaceBasesWithBoxes();
    void deleteInvalidObstacles();

    inline const std::string&  getName()         const { return name; }
    inline const ObstacleList& getList(int type) const { return lists[type]; }

    inline const std::vector<GroupInstance*>& getGroups()   const { return groups;   }
    inline const std::vector<WorldText*>&     getTexts()    const { return texts;    }
    inline const std::vector<LinkDef*>&       getLinkDefs() const { return linkDefs; }

    // Get the list of meshes that came from the world file.
    // This includes the meshes in group definitions, even if
    // they have never been instantiated.
    void getSourceMeshes(std::vector<MeshObstacle*>& meshes) const;

    int   packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void printGrouped(std::ostream& out, const std::string& indent) const;
    void printFlatFile(std::ostream& out, const std::string& indent) const;

  public:
    static void clearDepthName();

  private:
    Obstacle* newObstacle(int type);
    void makeDefaultName(Obstacle* obs, const std::string& tag,
                         unsigned int pos) const;
    void appendGroupName(const GroupInstance* group) const;

  private:
    std::string name;

    ObstacleList lists[ObstacleTypeCount];
    std::vector<GroupInstance*> groups;
    std::vector<WorldText*>     texts;
    std::vector<LinkDef*>       linkDefs;

    mutable bool active; // for recursion checking

  private:
    static std::string depthName;
};


//============================================================================//
//
//  Group Definition Manager
//  - utility class to keep track of group definitions
//

class GroupDefinitionMgr {
  public:
    typedef std::vector<const MeshFace*> FaceList;

  public:
    GroupDefinitionMgr();
    ~GroupDefinitionMgr();

    void clear();     // delete the lists and the obstacles
    void tighten();   // reduce memory usage
    void makeWorld(); // make the local obstacles for the groups
    void replaceBasesWithBoxes();

    void addGroupDef(GroupDefinition* groupDef);
    void addWorldObstacle(Obstacle* obstacle);
    void addWorldText(WorldText* text);
    void addWorldLinkDef(LinkDef* linkDef);

    GroupDefinition* findGroupDef(const std::string& name) const;

    // Get the list of meshes that came from the world file.
    // This includes the meshes in group definitions, even if
    // they have never been instantiated.
    void getSourceMeshes(std::vector<MeshObstacle*>& meshes) const;

    inline Obstacle* getObstacleFromID(unsigned int id);

    inline const GroupDefinition& getWorld()    const { return world;    }
    inline const GroupDefinition& getWorldDef() const { return worldDef; }

    // convenience functions
    inline const ObstacleList& getWalls()   const { return world.getList(wallType);   }
    inline const ObstacleList& getBoxes()   const { return world.getList(boxType);    }
    inline const ObstacleList& getPyrs()    const { return world.getList(pyrType);    }
    inline const ObstacleList& getBases()   const { return world.getList(baseType);   }
    inline const ObstacleList& getMeshes()  const { return world.getList(meshType);   }
    inline const ObstacleList& getTeles()   const { return world.getList(teleType);   }
    inline const ObstacleList& getArcs()    const { return world.getList(arcType);    }
    inline const ObstacleList& getCones()   const { return world.getList(coneType);   }
    inline const ObstacleList& getSpheres() const { return world.getList(sphereType); }
    inline const ObstacleList& getFaces()   const { return meshFaces; }
    inline const std::vector<WorldText*>& getTexts() const { return world.getTexts(); }

    int   packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  private:
    void assignListIDs();

  private:
    GroupDefinition world;    // world instances
    GroupDefinition worldDef; // world definition

    std::vector<GroupDefinition*> groupDefs;

    ObstacleList meshFaces;

    std::set<void*> copies;
};


inline Obstacle* GroupDefinitionMgr::getObstacleFromID(uint32_t guid) {
  const ObstacleType type = Obstacle::getTypeIDFromGUID(guid);

  const ObstacleList* oList;
  switch (type) {
    case wallType:   { oList = &getWalls();   break; }
    case boxType:    { oList = &getBoxes();   break; }
    case pyrType:    { oList = &getPyrs();    break; }
    case baseType:   { oList = &getBases();   break; }
    case meshType:   { oList = &getMeshes();  break; }
    case teleType:   { oList = &getTeles();   break; }
    case arcType:    { oList = &getArcs();    break; }
    case coneType:   { oList = &getCones();   break; }
    case sphereType: { oList = &getSpheres(); break; }
    case faceType:   { oList = &getFaces();   break; }
    default: {
      return NULL;
    }
  }

  const uint32_t listID = Obstacle::getListIDFromGUID(guid);
  if (listID >= oList->size()) {
    return NULL;
  }

  return (*oList)[listID];
}


//============================================================================//


extern GroupDefinitionMgr OBSTACLEMGR;


#endif // BZF_OBSTACLE_MGR_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
