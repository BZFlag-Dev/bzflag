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


#include "common.h"

// implementation header
#include "ObstacleMgr.h"

// system headers
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <assert.h>

// common headers
#include "Pack.h"
#include "MeshTransform.h"
#include "ObstacleModifier.h"
#include "StateDatabase.h"
#include "PhysicsDriver.h"
#include "BzMaterial.h"
#include "MeshDrawInfo.h"
#include "LinkDef.h"
#include "LinkManager.h"
#include "WorldText.h"
#include "vectors.h"
#include "bzfio.h"

// obstacle headers
#include "Obstacle.h"
#include "WallObstacle.h"
#include "BoxBuilding.h"
#include "PyramidBuilding.h"
#include "BaseBuilding.h"
#include "Teleporter.h"
#include "MeshObstacle.h"
#include "ArcObstacle.h"
#include "ConeObstacle.h"
#include "SphereObstacle.h"


//============================================================================//
//============================================================================//
//
// Group Instance
// - uses a group definition and a transform to produce obstacles
//


void GroupInstance::init()
{
  modifyTeam = false;
  team = 0;
  modifyColor = false;
  tint = fvec4(1.0f, 1.0f, 1.0f, 1.0f);
  modifyPhysicsDriver = false;
  phydrv = -1;
  modifyMaterial = false;
  material = NULL;
  driveThrough = 0;
  shootThrough = 0;
  ricochet = false;

  return;
}


GroupInstance::GroupInstance(const std::string& _groupDef)
{
  groupDef = _groupDef;
  init();
  return;
}


GroupInstance::GroupInstance()
{
  init();
  return;
}


GroupInstance::~GroupInstance()
{
  return;
}


void GroupInstance::setTransform(const MeshTransform& _transform)
{
  transform = _transform;
  return;
}


void GroupInstance::setName(const std::string& _name)
{
  name = _name;
  return;
}


const std::string& GroupInstance::getName() const
{
  return name;
}


void GroupInstance::setTeam(int _team)
{
  team = _team;
  modifyTeam = true;
  return;
}


void GroupInstance::setTint(const fvec4& _tint)
{
  tint = _tint;
  modifyColor = true;
  return;
}


void GroupInstance::setPhysicsDriver(int _phydrv)
{
  phydrv = _phydrv;
  modifyPhysicsDriver = true;
  return;
}


void GroupInstance::setMaterial(const BzMaterial* _material)
{
  material = _material;
  modifyMaterial = true;
  return;
}


void GroupInstance::setDriveThrough()
{
  driveThrough = 0xFF;
  return;
}


void GroupInstance::setShootThrough()
{
  shootThrough = 0xFF;
  return;
}


void GroupInstance::setCanRicochet()
{
  ricochet = true;
  return;
}


void GroupInstance::addMaterialSwap(const BzMaterial* srcMat,
				    const BzMaterial* dstMat)
{
  matMap[srcMat] = dstMat;
  return;
}


void GroupInstance::addTextSwap(const std::string& srcText,
				const std::string& dstText)
{
  textMap[srcText] = dstText;
  return;
}


const std::string& GroupInstance::getGroupDef() const
{
  return groupDef;
}


const MeshTransform& GroupInstance::getTransform() const
{
  return transform;
}


//============================================================================//

int GroupInstance::packSize() const
{
  int fullSize = 0;

  fullSize += nboStdStringPackSize(groupDef);
  fullSize += nboStdStringPackSize(name);

  fullSize += transform.packSize();

  fullSize += sizeof(uint8_t);

  if (modifyTeam) {
    fullSize += sizeof(uint16_t);
  }
  if (modifyColor) {
    fullSize += sizeof(fvec4);
  }
  if (modifyPhysicsDriver) {
    fullSize += sizeof(int32_t);
  }
  if (modifyMaterial) {
    fullSize += sizeof(int32_t);
  }

  fullSize += sizeof(int32_t); // matMap count
  fullSize += matMap.size() * 2 * sizeof(int32_t);

  fullSize += sizeof(int32_t); // textMap count
  TextSwapMap::const_iterator textIt;
  for (textIt = textMap.begin(); textIt != textMap.end(); ++textIt) {
    fullSize += nboStdStringPackSize(textIt->first);
    fullSize += nboStdStringPackSize(textIt->second);
  }

  return fullSize;
}


void* GroupInstance::pack(void* buf) const
{
  buf = nboPackStdString(buf, groupDef);
  buf = nboPackStdString(buf, name);

  buf = transform.pack(buf);

  uint8_t bits = 0;
  if (modifyTeam)          bits |= (1 << 0);
  if (modifyColor)         bits |= (1 << 1);
  if (modifyPhysicsDriver) bits |= (1 << 2);
  if (modifyMaterial)      bits |= (1 << 3);
  if (driveThrough)        bits |= (1 << 4);
  if (shootThrough)        bits |= (1 << 5);
  if (ricochet)            bits |= (1 << 6);
  buf = nboPackUInt8(buf, bits);

  if (modifyTeam) {
    buf = nboPackUInt16(buf, team);
  }
  if (modifyColor) {
    buf = nboPackFVec4(buf, tint);
  }
  if (modifyPhysicsDriver) {
    buf = nboPackInt32(buf, phydrv);
  }
  if (modifyMaterial) {
    int matindex = MATERIALMGR.getIndex(material);
    buf = nboPackInt32(buf, (int32_t) matindex);
  }

  buf = nboPackInt32(buf, matMap.size());
  MaterialMap::const_iterator matIt;
  for (matIt = matMap.begin(); matIt != matMap.end(); matIt++) {
    const int srcIndex = MATERIALMGR.getIndex(matIt->first);
    const int dstIndex = MATERIALMGR.getIndex(matIt->second);
    buf = nboPackInt32(buf, srcIndex);
    buf = nboPackInt32(buf, dstIndex);
  }

  buf = nboPackInt32(buf, textMap.size());
  TextSwapMap::const_iterator textIt;
  for (textIt = textMap.begin(); textIt != textMap.end(); ++textIt) {
    buf = nboPackStdString(buf, textIt->first);
    buf = nboPackStdString(buf, textIt->second);
  }

  return buf;
}


void* GroupInstance::unpack(void* buf)
{
  buf = nboUnpackStdString(buf, groupDef);
  buf = nboUnpackStdString(buf, name);

  buf = transform.unpack(buf);

  uint8_t bits;
  buf = nboUnpackUInt8(buf, bits);
  modifyTeam          = ((bits & (1 << 0)) == 0) ? false : true;
  modifyColor         = ((bits & (1 << 1)) == 0) ? false : true;
  modifyPhysicsDriver = ((bits & (1 << 2)) == 0) ? false : true;
  modifyMaterial      = ((bits & (1 << 3)) == 0) ? false : true;
  driveThrough        = ((bits & (1 << 4)) == 0) ? 0 : 0xFF;
  shootThrough        = ((bits & (1 << 5)) == 0) ? 0 : 0xFF;
  ricochet            = ((bits & (1 << 6)) == 0) ? false : true;

  if (modifyTeam) {
    uint16_t tmpTeam;
    buf = nboUnpackUInt16(buf, tmpTeam);
    team = (int)tmpTeam;
  }
  if (modifyColor) {
    buf = nboUnpackFVec4(buf, tint);
  }
  if (modifyPhysicsDriver) {
    int32_t inPhyDrv;
    buf = nboUnpackInt32(buf, inPhyDrv);
    phydrv = int(inPhyDrv);
  }
  if (modifyMaterial) {
    int32_t matindex;
    buf = nboUnpackInt32(buf, matindex);
    material = MATERIALMGR.getMaterial(matindex);
  }

  int32_t count;

  buf = nboUnpackInt32(buf, count);
  for (int i = 0; i < count; i++) {
    int32_t srcIndex, dstIndex;
    buf = nboUnpackInt32(buf, srcIndex);
    buf = nboUnpackInt32(buf, dstIndex);
    const BzMaterial* srcMat = MATERIALMGR.getMaterial(srcIndex);
    const BzMaterial* dstMat = MATERIALMGR.getMaterial(dstIndex);
    matMap[srcMat] = dstMat;
  }

  buf = nboUnpackInt32(buf, count);
  for (int i = 0; i < count; i++) {
    std::string srcText, dstText;
    buf = nboUnpackStdString(buf, srcText);
    buf = nboUnpackStdString(buf, dstText);
    textMap[srcText] = dstText;
  }

  return buf;
}


//============================================================================//

void GroupInstance::print(std::ostream& out, const std::string& indent) const
{
  out << indent << "group " << groupDef << std::endl;

  if (name.size() > 0) {
    out << indent << "  name " << name << std::endl;
  }

  transform.printTransforms(out, indent);

  if (modifyPhysicsDriver) {
    const PhysicsDriver* driver = PHYDRVMGR.getDriver(phydrv);
    if (driver != NULL) {
      out << indent << "  phydrv ";
      if (driver->getName().size() > 0) {
	out << driver->getName();
      } else {
	out << phydrv;
      }
      out << std::endl;
    }
  }

  if (driveThrough) { out << indent << "  driveThrough" << std::endl; }
  if (shootThrough) { out << indent << "  shootThrough" << std::endl; }
  if (ricochet)     { out << indent << "  ricochet"     << std::endl; }

  if (modifyColor) {
    out << indent << "  tint " << tint[0] << " " << tint[1] << " "
			       << tint[2] << " " << tint[3] << " "
			       << std::endl;
  }

  if (modifyMaterial) {
    out << indent << "  matref ";
    MATERIALMGR.printReference(out, material);
    out << std::endl;
  }
  else if (matMap.size() > 0) {
    MaterialMap::const_iterator it;
    for (it = matMap.begin(); it != matMap.end(); it++) {
      out << indent << "  matswap ";
      MATERIALMGR.printReference(out, it->first);
      out << " ";
      MATERIALMGR.printReference(out, it->second);
      out << std::endl;
    }
  }

  if (modifyTeam) {
    out << indent << "  team " << team << std::endl;
  }

  TextSwapMap::const_iterator textIt;
  for (textIt = textMap.begin(); textIt != textMap.end(); textIt++) {
    out << indent << "  textswap " << textIt->first << " "
                                   << textIt->second << std::endl;
  }

  out << indent << "end" << std::endl << std::endl;

  return;
}


//============================================================================//
//============================================================================//
//
// Group Definition
// - defines an obstacle group
//

std::string GroupDefinition::depthName;


GroupDefinition::GroupDefinition(const std::string& _name)
{
  name = _name;
  active = false;
  return;
}


GroupDefinition::~GroupDefinition()
{
  return;
}


Obstacle* GroupDefinition::newObstacle(int type)
{
  Obstacle* obs = NULL;

  switch (type) {
    case wallType:   { obs = new WallObstacle();    break; }
    case boxType:    { obs = new BoxBuilding();     break; }
    case pyrType:    { obs = new PyramidBuilding(); break; }
    case baseType:   { obs = new BaseBuilding();    break; }
    case meshType:   { obs = new MeshObstacle();    break; }
    case teleType:   { obs = new Teleporter();      break; }
    case arcType:    { obs = new ArcObstacle();     break; }
    case coneType:   { obs = new ConeObstacle();    break; }
    case sphereType: { obs = new SphereObstacle();  break; }
  }

  return obs;
}


void GroupDefinition::addObstacle(Obstacle* obstacle)
{
  const int type = obstacle->getTypeID();
  if ((type < 0) || (type >= ObstacleTypeCount)) { // excludes faces
    printf("GroupDefinition::addObstacle() ERROR: type = %s\n",
           obstacle->getType());
    exit(1);
  }
  lists[type].push_back(obstacle);
  return;
}


void GroupDefinition::addGroupInstance(GroupInstance* group)
{
  groups.push_back(group);
  return;
}


void GroupDefinition::addText(WorldText* text)
{
  texts.push_back(text);
  return;
}


void GroupDefinition::addLinkDef(LinkDef* linkDef)
{
  linkDefs.push_back(linkDef);
  return;
}


static bool isContainer(int type)
{
  switch (type) {
    case teleType:
    case arcType:
    case coneType:
    case sphereType: {
      return true;
    }
    default: {
      return false;
    }
  }
}


void GroupDefinition::makeDefaultName(Obstacle* obs, const std::string& tag,
                                      unsigned int pos) const
{
  std::string fullname = depthName;
  if (obs->getName().size() > 0) {
    fullname += obs->getName();
  } else {
    // make the default name
    fullname += "$";
    fullname += tag;
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%i", pos);
    fullname += buffer;
  }
  obs->setName(fullname);
  return;
}


void GroupDefinition::appendGroupName(const GroupInstance* group) const
{
  std::string newName;
  if (group->getName().size() > 0) {
    newName = group->getName();
  }
  else {
    // make the default name
    int count = 0;
    for (unsigned int i = 0; i < groups.size(); i++) {
      const GroupInstance* g = groups[i];
      if (g == group) {
	break;
      }
      if (g->getGroupDef() == group->getGroupDef()) {
	count++;
      }
    }
    newName = "$g_";
    newName += group->getGroupDef();
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "_%i", count);
    newName += buffer;
  }
  depthName += newName;
  depthName += "/";
  return;
}


static MeshObstacle* makeContainedMesh(int type, const Obstacle* obs)
{
  MeshObstacle* mesh = NULL;
  switch (type) {
    case teleType:   { mesh =     ((Teleporter*)obs)->makeMesh(); break; }
    case arcType:    { mesh =    ((ArcObstacle*)obs)->makeMesh(); break; }
    case coneType:   { mesh =   ((ConeObstacle*)obs)->makeMesh(); break; }
    case sphereType: { mesh = ((SphereObstacle*)obs)->makeMesh(); break; }
  }
  return mesh;
}


void GroupDefinition::makeGroups(const MeshTransform& xform,
				 const ObstacleModifier& obsMod) const
{
  if (active) {
    logDebugMessage(0, "WARNING: avoided recursion, groupDef \"%s\"\n",
                       name.c_str());
    return; // avoid recursion
  }

  active = true;

  const bool isWorld = (this == &OBSTACLEMGR.getWorld());
  char groupDefBit = isWorld ? Obstacle::WorldSource :
			       Obstacle::GroupDefSource;

  for (int type = 0; type < ObstacleTypeCount; type++) {
    const ObstacleList& oList = lists[type];
    for (unsigned int i = 0; i < oList.size(); i++) {
      Obstacle* obs;
      if (isWorld) {
	obs = oList[i]; // no need to copy
      } else {
	obs = oList[i]->copyWithTransform(xform);
      }

      // the tele names are setup with default names if
      // they are not named (even for those in the world
      // groupd def). invalid teleporters are also named
      if (type == teleType) {
	makeDefaultName(obs, "mt", i);
      }
      else if (type == meshType) {
	makeDefaultName(obs, "m", i);
      }

      if (obs->isValid()) {
	if (!isWorld) {
	  // add it to the world
	  // (this will also add container obstacles into the world group)
	  obs->setSource(Obstacle::GroupDefSource);
	  obsMod.execute(obs);
	  OBSTACLEMGR.addWorldObstacle(obs);

	  // add a modified MeshDrawInfo to the new mesh, if applicable
	  if (type == meshType) {
	    const MeshObstacle* source = (const MeshObstacle*) oList[i];
	    const MeshDrawInfo* diSource = source->getDrawInfo();
	    if ((diSource != NULL) &&
		(!diSource->isServerSide()) && diSource->isValid()) {
	      MaterialSet matSet;
	      MaterialMap matMap;
	      diSource->getMaterials(matSet);
	      obsMod.getMaterialMap(matSet, matMap);
	      MeshDrawInfo* diDest = new MeshDrawInfo(diSource, xform, matMap);
	      MeshObstacle* dest = (MeshObstacle*) obs;
	      dest->setDrawInfo(diDest);
	    }
	  }
	}

	// generate contained meshes
	// (always get placed into the world group)
	MeshObstacle* mesh = makeContainedMesh(type, obs);
	if ((mesh != NULL) && mesh->isValid()) {
	  mesh->setSource(Obstacle::ContainerSource | groupDefBit);
	  obsMod.execute(mesh);
	  OBSTACLEMGR.addWorldObstacle(mesh);
	}
      }
    }
  }

  // add the texts
  if (!isWorld) {
    for (size_t i = 0; i < texts.size(); i++) {
      WorldText* text = texts[i]->copyWithTransform(xform);
      text->setFromGroup();
      obsMod.execute(text);
      OBSTACLEMGR.addWorldText(text);
    }
  }

  // add the links
  for (size_t i = 0; i < linkDefs.size(); i++) {
    const LinkDef& linkDef = *(linkDefs[i]);
    if (isWorld) {
      linkManager.addLinkDef(linkDef);
    }
    else {
      LinkDef copyDef(linkDef);
      copyDef.prepend(depthName);
      linkManager.addLinkDef(copyDef);
    }
  }

  // make more groups instances
  for (unsigned int i = 0; i < groups.size(); i++) {
    const GroupInstance* group = groups[i];
    const GroupDefinition* groupDef =
      OBSTACLEMGR.findGroupDef(group->getGroupDef());
    if (groupDef != NULL) {
      // make the new depth name
      std::string tmpDepthName = depthName;
      appendGroupName(group);

      // setup the transform and modifier
      ObstacleModifier newObsMod(obsMod, *group);
      MeshTransform tmpXform = xform;
      tmpXform.prepend(group->getTransform());

      // recurse and make plentiful
      groupDef->makeGroups(tmpXform, newObsMod);

      // revert to the old depth name
      depthName = tmpDepthName;
    }
    else {
      logDebugMessage(1, "warning: group definition \"%s\" is missing\n",
	     group->getGroupDef().c_str());
    }
  }

  active = false;

  return;
}


void GroupDefinition::replaceBasesWithBoxes()
{
  ObstacleList& baseList = lists[baseType];
  for (unsigned int i = 0; i < baseList.size(); i++) {
    BaseBuilding* base = (BaseBuilding*) baseList[i];
    const fvec3& baseSize = base->getSize();
    BoxBuilding* box =
      new BoxBuilding(base->getPosition(), base->getRotation(),
		      baseSize.x, baseSize.y, baseSize.z,
		      base->isDriveThrough(), base->isShootThrough(),
		      base->canRicochet(), false);
    delete base;
    baseList.remove(i);
    i--;
    addObstacle(box);
  }
  ObstacleList& meshList = lists[meshType];
  for (unsigned int i = 0; i < meshList.size(); i++) {
    MeshObstacle* mesh = (MeshObstacle*) meshList[i];
    const int faceCount = mesh->getFaceCount();
    for (int f = 0; f < faceCount; f++) {
      mesh->getFace(f)->clearBaseTeam();
    }
  }
  return;
}


void GroupDefinition::clear()
{
  for (int type = 0; type < ObstacleTypeCount; type++) {
    ObstacleList& oList = lists[type];
    for (unsigned int i = 0; i < oList.size(); i++) {
      delete oList[i];
    }
    oList.clear();
  }

  for (unsigned int i = 0; i < groups.size(); i++) {
    delete groups[i];
  }
  groups.clear();

  for (size_t i = 0; i < texts.size(); i++) {
    delete texts[i];
  }
  texts.clear();

  for (size_t i = 0; i < linkDefs.size(); i++) {
    delete linkDefs[i];
  }
  linkDefs.clear();

  return;
}


void GroupDefinition::tighten()
{
  for (int type = 0; type < ObstacleTypeCount; type++) {
    lists[type].tighten();
  }
  return;
}


void GroupDefinition::sort(int (*compare)(const void* a, const void* b))
{
  if (this != &OBSTACLEMGR.getWorld()) {
    return; // only sort the world groupDef
  }
  for (int type = 0; type < ObstacleTypeCount; type++) {
    lists[type].sort(compare);
  }
  return;
}


void GroupDefinition::deleteInvalidObstacles()
{
  if (this != &OBSTACLEMGR.getWorld()) {
    return; // only delete invalid obstacles in the world groupDef
  }
  for (int type = 0; type < ObstacleTypeCount; type++) {
    ObstacleList& oList = lists[teleType];
    for (unsigned int i = 0; i < oList.size(); i++) {
      Obstacle* obs = oList[i];
      if (!obs->isValid()) {
	logDebugMessage(1, "Deleted invalid %s obstacle\n", obs->getType());
	delete obs;
	oList.remove(i);
	i--; // don't miss the substitute
      }
    }
  }
  return;
}


void GroupDefinition::getSourceMeshes(std::vector<MeshObstacle*>& meshes) const
{
  const bool isWorld = (this == &OBSTACLEMGR.getWorld());

  const ObstacleList& oList = lists[meshType];
  for (unsigned int i = 0; i < oList.size(); i++) {
    MeshObstacle* mesh = (MeshObstacle*) oList[i];
    if (!isWorld || mesh->isFromWorldFile()) {
      const int listSize = (int)meshes.size();
      int j;
      for (j = 0; j < listSize; j++) {
	if (meshes[j] == mesh) {
	  break;
	}
      }
      if (j == listSize) {
	meshes.push_back(mesh); // a new entry
      }
    }
  }

  for (unsigned int i = 0; i < groups.size(); i++) {
    const GroupInstance* group = groups[i];
    const GroupDefinition* groupDef =
      OBSTACLEMGR.findGroupDef(group->getGroupDef());
    if (groupDef != NULL) {
      groupDef->getSourceMeshes(meshes);
    }
  }

  return;
}


void GroupDefinition::clearDepthName()
{
  depthName = "";
  return;
}


//============================================================================//

int GroupDefinition::packSize() const
{
  int fullSize = 0;

  fullSize += nboStdStringPackSize(name);

  // obstacles
  for (int type = 0; type < ObstacleTypeCount; type++) {
    fullSize += sizeof(uint32_t);
    const ObstacleList& oList = getList(type);
    for (unsigned int i = 0; i < oList.size(); i++) {
      if (oList[i]->isFromWorldFile()) {
	fullSize += oList[i]->packSize();
      }
    }
  }

  // group instances
  fullSize += sizeof(uint32_t);
  for (unsigned int i = 0; i < groups.size(); i++) {
    fullSize += groups[i]->packSize();
  }

  // texts
  fullSize += sizeof(uint32_t);
  for (size_t i = 0; i < texts.size(); i++) {
    if (!texts[i]->getFromGroup()) {
      fullSize += texts[i]->packSize();
    }
  }

  // link definitions
  fullSize += sizeof(uint32_t);
  for (size_t i = 0; i < linkDefs.size(); i++) {
    fullSize += linkDefs[i]->packSize();
  }

  return fullSize;
}


void* GroupDefinition::pack(void* buf) const
{
  buf = nboPackStdString(buf, name);

  // obstacles
  unsigned int i;
  for (int type = 0; type < ObstacleTypeCount; type++) {
    const ObstacleList& oList = getList(type);
    int count = 0;
    for (i = 0; i < oList.size(); i++) {
      if (oList[i]->isFromWorldFile()) {
	count++;
      }
    }
    buf = nboPackUInt32(buf, count);
    for (i = 0; i < oList.size(); i++) {
      if (oList[i]->isFromWorldFile()) {
	buf = oList[i]->pack(buf);
      }
    }
  }

  // group instances
  buf = nboPackUInt32(buf, groups.size());
  for (i = 0; i < groups.size(); i++) {
    buf = groups[i]->pack(buf);
  }

  // texts
  uint32_t textCount = 0;
  for (i = 0; i < texts.size(); i++) {
    if (!texts[i]->getFromGroup()) {
      textCount++;
    }
  }
  buf = nboPackUInt32(buf, textCount);
  for (i = 0; i < texts.size(); i++) {
    if (!texts[i]->getFromGroup()) {
      buf = texts[i]->pack(buf);
    }
  }

  // link definitions
  buf = nboPackUInt32(buf, linkDefs.size());
  for (i = 0; i < linkDefs.size(); i++) {
    buf = linkDefs[i]->pack(buf);
  }

  return buf;
}


void* GroupDefinition::unpack(void* buf)
{
  buf = nboUnpackStdString(buf, name);

  uint32_t i, count;

  // obstacles
  for (int type = 0; type < ObstacleTypeCount; type++) {
    buf = nboUnpackUInt32(buf, count);
    for (i = 0; i < count; i++) {
      Obstacle* obs = newObstacle(type);
      if (obs != NULL) {
	buf = obs->unpack(buf);
	if (obs->isValid()) {
	  lists[type].push_back(obs);
	}
      }
    }
  }

  // group instances
  buf = nboUnpackUInt32(buf, count);
  for (i = 0; i < count; i++) {
    GroupInstance* group = new GroupInstance;
    buf = group->unpack(buf);
    addGroupInstance(group);
  }

  // texts
  buf = nboUnpackUInt32(buf, count);
  for (i = 0; i < count; i++) {
    WorldText* text = new WorldText;
    buf = text->unpack(buf);
    addText(text);
  }

  // link definitions
  buf = nboUnpackUInt32(buf, count);
  for (i = 0; i < count; i++) {
    LinkDef* linkDef = new LinkDef;
    buf = linkDef->unpack(buf);
    addLinkDef(linkDef);
  }

  return buf;
}


//============================================================================//

void GroupDefinition::printGrouped(std::ostream& out,
				   const std::string& indent) const
{
  const bool isWorld = (this == &OBSTACLEMGR.getWorld());
  const bool saveAsMeshes = BZDB.isTrue("saveAsMeshes");
  std::string myIndent = indent;

  // deal with indenting
  if (!isWorld) {
    myIndent += "  ";
    out << indent << "define " << name << std::endl;
  }

  // print the obstacles
  for (int type = 0; type < ObstacleTypeCount; type++) {
    const ObstacleList& oList = getList(type);
    for (unsigned int i = 0; i < oList.size(); i++) {
      const Obstacle* obs = oList[i];

      if (!obs->isFromGroupDef() && !obs->isFromContainer()) {
	if (!saveAsMeshes) {
	  if (!obs->isFromContainer()) {
	    obs->print(out, myIndent);
	  }
	}
	else {
	  if (!isContainer(type)) {
	    obs->print(out, myIndent);
	  } else {
	    // rebuild the mesh  (ya, just to print it)
	    MeshObstacle* mesh = makeContainedMesh(type, obs);
	    if ((mesh != NULL) && (mesh->isValid())) {
	      mesh->print(out, myIndent);
	    }
	    delete mesh;
	  }
	}
      }
    }
  }

  // print the texts
  for (unsigned int i = 0; i < texts.size(); i++) {
    if (!isWorld || !texts[i]->getFromGroup()) {
      texts[i]->print(out, myIndent);
    }
  }

  // print the groups
  for (unsigned int i = 0; i < groups.size(); i++) {
    groups[i]->print(out, myIndent);
  }

  // print the links
  for (unsigned int i = 0; i < linkDefs.size(); i++) {
    linkDefs[i]->print(out, myIndent);
  }

  // deal with indenting
  if (!isWorld) {
    out << indent << "enddef" << std::endl << std::endl;
  }

  return;
}


void GroupDefinition::printFlatFile(std::ostream& out,
				    const std::string& indent) const
{
  const bool saveAsOBJ    = BZDB.isTrue("saveAsOBJ");
  const bool saveAsMeshes = BZDB.isTrue("saveAsMeshes");

  // print the obstacles
  for (int type = 0; type < ObstacleTypeCount; type++) {
    const ObstacleList& oList = getList(type);
    for (unsigned int i = 0; i < oList.size(); i++) {
      const Obstacle* obs = oList[i];

      if (!saveAsMeshes) {
	if (!obs->isFromContainer()) {
	  obs->print(out, indent);
	}
      } else {
	if (!isContainer(type)) {
	  if (!saveAsOBJ) {
	    obs->print(out, indent);
	  } else {
	    obs->printOBJ(out, indent);
	  }
	}
      }
    }
  }

  // print the texts
  for (unsigned int i = 0; i < texts.size(); i++) {
    texts[i]->print(out, indent);
  }

  // print the links -- FIXME -- use linkManager ?
  for (unsigned int i = 0; i < linkDefs.size(); i++) {
    linkDefs[i]->print(out, indent);
  }

  return;
}


//============================================================================//
//============================================================================//
//
// Group Definition Manager
// - utility class to keep track of group definitions
//

GroupDefinitionMgr OBSTACLEMGR;


GroupDefinitionMgr::GroupDefinitionMgr() : world(""), worldDef("")
{
  return;
}


GroupDefinitionMgr::~GroupDefinitionMgr()
{
  clear();
  return;
}


void GroupDefinitionMgr::clear()
{
  world.clear();
  for (size_t i = 0; i < groupDefs.size(); i++) {
    groupDefs[i]->clear();
    delete groupDefs[i];
  }
  groupDefs.clear();
  return;
}


void GroupDefinitionMgr::tighten()
{
  for (size_t i = 0; i < groupDefs.size(); i++) {
    groupDefs[i]->tighten();
  }
  world.tighten();
  return;
}


static int compareHeights(const void* a, const void* b)
{
  const Obstacle* obsA = *((const Obstacle**)a);
  const Obstacle* obsB = *((const Obstacle**)b);
  const Extents& eA = obsA->getExtents();
  const Extents& eB = obsB->getExtents();

  const float zMaxA = eA.maxs[2];
  const float zMaxB = eB.maxs[2];
  if (zMaxA > zMaxB) { return -1; }
  if (zMaxA < zMaxB) { return +1; }

  const int listIDA = obsA->getListID();
  const int listIDB = obsB->getListID();
  if (listIDA < listIDB) { return -1; }
  if (listIDA > listIDB) { return +1; }

  assert(false && "this should not happen");
  return 0;
}


void GroupDefinitionMgr::makeWorld()
{
  GroupDefinition::clearDepthName();

  MeshTransform noXform;
  ObstacleModifier noMods;

  world.makeGroups(noXform, noMods);

  world.deleteInvalidObstacles();

  // assign the listIDs
  for (int type = 0; type < ObstacleTypeCount; type++) {
    const ObstacleList& obsList = world.getList(type);
    for (unsigned int i = 0; i < obsList.size(); i++) {
      Obstacle* obs = obsList[i];
      obs->setListID(i);
    }
  }

  // sort from top to bottom for enhanced radar
  for (int type = 0; type < ObstacleTypeCount; type++) {
    world.sort(compareHeights);
  }

  // free unused memory at the end of the arrays
  tighten();

  // link the faces
  linkManager.doLinking();

  // debug printing
  const int dbg = 4;
  if (debugLevel >= dbg) {
    logDebugMessage(dbg, "\n");
    logDebugMessage(dbg, "Named obstacles:\n");
    for (int type = 0; type < ObstacleTypeCount; type++) {
      const ObstacleList& obsList = world.getList(type);
      for (unsigned int i = 0; i < obsList.size(); i++) {
	Obstacle* obs = obsList[i];
	logDebugMessage(dbg, "  '/%-40s  <%s>\n",
			(obs->getName() + "'").c_str(), obs->getType());
	if (obs->getTypeID() == meshType) {
	  const MeshObstacle* mesh = (const MeshObstacle*)obs;
	  const int faceCount = mesh->getFaceCount();
	  for (int f = 0; f < faceCount; f++) {
	    const MeshFace* face = mesh->getFace(f);
	    if (face->isSpecial()) {
	      const MeshFace::SpecialData* sd = face->getSpecialData();
	      if (!sd->linkName.empty()) {
		logDebugMessage(dbg, "  '/%-40s  <MeshFace #%i>\n",
				(face->getLinkName() + "'").c_str(), f);
	      }
	    }
	  }
	}
      }
    }
    logDebugMessage(dbg, "\n");
  }

  return;
}


void GroupDefinitionMgr::replaceBasesWithBoxes()
{
  world.replaceBasesWithBoxes();
}


void GroupDefinitionMgr::addWorldObstacle(Obstacle* obstacle)
{
  world.addObstacle(obstacle);
  return;
}


void GroupDefinitionMgr::addWorldText(WorldText* text)
{
  world.addText(text);
  return;
}


void GroupDefinitionMgr::addWorldLinkDef(LinkDef* linkDef)
{
  world.addLinkDef(linkDef);
  return;
}


void GroupDefinitionMgr::addGroupDef(GroupDefinition* groupDef)
{
  if (!groupDef->getName().empty()) {
    groupDefs.push_back(groupDef);
  } else {
    delete groupDef;
  }
  return;
}


GroupDefinition* GroupDefinitionMgr::findGroupDef(const std::string& name) const
{
  if (name.size() <= 0) {
    return NULL;
  }
  for (size_t i = 0; i < groupDefs.size(); i++) {
    if (name == groupDefs[i]->getName()) {
      return groupDefs[i];
    }
  }
  return NULL;
}


void GroupDefinitionMgr::getSourceMeshes(std::vector<MeshObstacle*>& meshes) const
{
  meshes.clear();
  world.getSourceMeshes(meshes);
  return;
}


//============================================================================//

int GroupDefinitionMgr::packSize() const
{
  int fullSize = 0;

  fullSize += world.packSize();
  fullSize += sizeof(uint32_t);
  for (size_t i = 0; i < groupDefs.size(); i++) {
    fullSize += groupDefs[i]->packSize();
  }
  return fullSize;
}


void* GroupDefinitionMgr::pack(void* buf) const
{
  buf = world.pack(buf);
  buf = nboPackUInt32(buf, groupDefs.size());
  for (size_t i = 0; i < groupDefs.size(); i++) {
    buf = groupDefs[i]->pack(buf);
  }
  return buf;
}


void* GroupDefinitionMgr::unpack(void* buf)
{
  buf = world.unpack(buf);
  uint32_t i, count;
  buf = nboUnpackUInt32(buf, count);
  for (i = 0; i < count; i++) {
    GroupDefinition* groupDef = new GroupDefinition("");
    buf = groupDef->unpack(buf);
    addGroupDef(groupDef);
  }
  return buf;
}


//============================================================================//

void GroupDefinitionMgr::print(std::ostream& out,
			       const std::string& indent) const
{
  const bool saveAsOBJ    = BZDB.isTrue("saveAsOBJ");
  const bool saveFlatFile = BZDB.isTrue("saveFlatFile");
  const bool saveAsMeshes = BZDB.isTrue("saveAsMeshes");
  if (saveAsOBJ) {
    BZDB.set("saveAsMeshes", "1");
    BZDB.set("saveFlatFile", "1");
  }

  // for unique OBJ mesh ids
  Obstacle::resetObjCounter();

  if (!(saveFlatFile || saveAsOBJ)) {
    // print the group definitions
    for (size_t i = 0; i < groupDefs.size(); i++) {
      groupDefs[i]->printGrouped(out, indent);
    }
    // print the world
    world.printGrouped(out, indent);
  }
  else {
    // print the world
    world.printFlatFile(out, indent);
  }

  if (saveAsOBJ) {
    BZDB.set("saveAsMeshes", saveAsMeshes ? "1" : "0");
    BZDB.set("saveFlatFile", saveFlatFile ? "1" : "0");
  }

  return;
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
