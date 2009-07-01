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


// common goes first
#include "common.h"

// implementation header
#include "LinkManager.h"

// system headers
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>

// common headers
#include "bzglob.h"
#include "BZDBCache.h"
#include "Pack.h"
#include "MeshObstacle.h"
#include "MeshFace.h"
#include "ObstacleMgr.h"
#include "TextUtils.h"


static BZDB_int debugLinks("debugLinks");


LinkManager linkManager;


//============================================================================//

bool LinkManager::DstData::operator<(const DstData& dd) const
{
  const unsigned short myMeshID = face->getMesh()->getListID();
  const unsigned short ddMeshID = dd.face->getMesh()->getListID();
  if (myMeshID < ddMeshID) { return true;  }
  if (ddMeshID < myMeshID) { return false; }

  const int myFaceID = face->getFaceID();
  const int ddFaceID = dd.face->getFaceID();
  if (myFaceID < ddFaceID) { return true;  }
  if (ddFaceID < myFaceID) { return false; }

  if (physics < dd.physics) { return true;  }
  if (dd.physics < physics) { return false; }

  return false;
}


//============================================================================//

LinkManager::LinkManager()
{
  // do nothing
  return;
}


LinkManager::~LinkManager()
{
  clear();
  return;
}


void LinkManager::clear()
{
  linkDefs.clear();

  linkMap.clear();

  linkSrcs.clear();
  linkSrcMap.clear();

  linkDsts.clear();
  linkDstMap.clear();

  linkSrcSet.clear();
  linkDstSet.clear();
  linkFaceSet.clear();

  nameFaceVec.clear();
  return;
}


//============================================================================//

void LinkManager::addLinkDef(const LinkDef& linkDef)
{
  if (!linkDef.getSrcs().empty() &&
      !linkDef.getDsts().empty()) {
    linkDefs.push_back(linkDef);
  }
  return;
}


int LinkManager::getLinkSrcID(const MeshFace* face) const
{
  FaceIntMap::const_iterator it = linkSrcMap.find(face);
  if (it == linkSrcMap.end()) {
    return -1;
  }
  return it->second;
}


int LinkManager::getLinkDstID(const MeshFace* face,
                              const LinkPhysics& lp) const
{
  const DstData data(face, lp);
  DstDataIntMap::const_iterator it = linkDstMap.find(data);
  if (it == linkDstMap.end()) {
    return -1;
  }
  return it->second;
}


const MeshFace* LinkManager::getLinkSrcFace(int linkSrcID) const
{
  if ((linkSrcID < 0) || (linkSrcID >= (int)linkSrcs.size())) {
    return NULL;
  }
  return linkSrcs[linkSrcID];
}


const MeshFace* LinkManager::getLinkDstFace(int linkDstID) const
{
  if ((linkDstID < 0) || (linkDstID >= (int)linkDsts.size())) {
    return NULL;
  }
  return linkDsts[linkDstID].face;
}


const LinkManager::DstData* LinkManager::getLinkDstData(int linkDstID) const
{
  if ((linkDstID < 0) || (linkDstID >= (int)linkDsts.size())) {
    return NULL;
  }
  return &linkDsts[linkDstID];
}


//============================================================================//

const MeshFace* LinkManager::getShotLinkDst(const MeshFace* srcLink,
                                            unsigned int seed,
                                            int& linkSrcID, int& linkDstID,
                                            const LinkPhysics*& physics,
                                            const fvec3& pos,
                                            const fvec3& vel, int team,
                                            const FlagType* flagType) const
{
  LinkMap::const_iterator it = linkMap.find(srcLink);
  if (it == linkMap.end()) {
    return NULL;
  }

  const DstIndexList& dstList = it->second;
  const IntVec* dstIDs = &dstList.dstIDs;

  // use a simpler method for single entries
  if (dstIDs->size() == 1) {
    const int dstIndex = (*dstIDs)[0];
    const DstData& dstData = linkDsts[dstIndex];
    if (dstList.needTest) {
      if (!srcLink->shotCanCross(dstData.physics, pos, vel, team, flagType)) {
        return NULL;
      }
    }
    linkSrcID = linkSrcMap.find(srcLink)->second;
    linkDstID = dstIndex;
    physics = &dstData.physics;
    return dstData.face;
  }

  // construct the vector of possible destinations
  IntVec testIDs;
  if (dstList.needTest) {
    for (size_t i = 0; i < dstIDs->size(); i++) {
      const int dstIndex = (*dstIDs)[i];
      const DstData& dstData = linkDsts[dstIndex];
      if (srcLink->shotCanCross(dstData.physics, pos, vel, team, flagType)) {
        testIDs.push_back(dstIndex);
      }
    }
    dstIDs = &testIDs;
  }
  if (dstIDs->empty()) {
    return NULL;
  }

  seed = (seed * 1103515245 + 12345) >> 8; // from POSIX rand() example
  seed = seed % dstIDs->size();

  // assign the output variables
  linkSrcID = linkSrcMap.find(srcLink)->second;
  linkDstID = (*dstIDs)[seed];
  const DstData& dstData = linkDsts[linkDstID];
  physics = &dstData.physics;
  return dstData.face;
}


const MeshFace* LinkManager::getTankLinkDst(const MeshFace* srcLink,
                                            int& linkSrcID, int& linkDstID,
                                            const LinkPhysics*& physics,
                                            const fvec3& pos,
                                            const fvec3& vel, int team,
                                            const FlagType* flagType) const
{
  LinkMap::const_iterator it = linkMap.find(srcLink);
  if (it == linkMap.end()) {
    return NULL;
  }

  const DstIndexList& dstList = it->second;
  const IntVec* dstIDs = &dstList.dstIDs;

  // use a simpler method for single entries
  if (dstIDs->size() == 1) {
    const int dstIndex = (*dstIDs)[0];
    const DstData& dstData = linkDsts[dstIndex];
    if (dstList.needTest) {
      if (!srcLink->tankCanCross(dstData.physics, pos, vel, team, flagType)) {
        return NULL;
      }
    }
    linkSrcID = linkSrcMap.find(srcLink)->second;
    linkDstID = dstIndex;
    physics = &dstData.physics;
    return dstData.face;
  }

  // construct the vector of possible destinations
  IntVec testIDs;
  if (dstList.needTest) {
    for (size_t i = 0; i < dstIDs->size(); i++) {
      const int dstIndex = (*dstIDs)[i];
      const DstData& dstData = linkDsts[dstIndex];
      if (srcLink->tankCanCross(dstData.physics, pos, vel, team, flagType)) {
        testIDs.push_back(dstIndex);
      }
    }
    dstIDs = &testIDs;
  }
  if (dstIDs->empty()) {
    return NULL;
  }

  const int randIndex = rand() % (int)dstIDs->size();

  // assign the output variables
  linkSrcID = linkSrcMap.find(srcLink)->second;
  linkDstID = (*dstIDs)[randIndex];
  const DstData& dstData = linkDsts[linkDstID];
  physics = &dstData.physics;
  return dstData.face;
}


//============================================================================//

void LinkManager::doLinking()
{
  // get the potential named faces
  buildNameMap();

  // build the link map
  for (size_t i = 0; i < linkDefs.size(); i++) {
    const LinkDef& linkDef = linkDefs[i];
    FaceSet srcs;
    FaceSet dsts;
    if (matchLinks(linkDef.getSrcs(), srcs) &&
        matchLinks(linkDef.getDsts(), dsts)) {
      FaceSet::const_iterator srcIt;
      FaceSet::const_iterator dstIt;
      for (srcIt = srcs.begin(); srcIt != srcs.end(); ++srcIt) {
        for (dstIt = dsts.begin(); dstIt != dsts.end(); ++dstIt) {
          createLink(*srcIt, *dstIt, linkDef.physics);
        }
      }
    }
  }

  // make sure that old style teleporters are always linked
  // by adding links from front-to-back (or back-to-front),
  // when a given link source has no destination
  crossLink();

  // setup the LinkPhysics::needTest() values
  for (size_t i = 0; i < linkDsts.size(); i++) {
    linkDsts[i].physics.finalize();
  }

  // setup the 'needTest' parameters
  LinkMap::iterator mapIt;
  for (mapIt = linkMap.begin(); mapIt != linkMap.end(); ++mapIt) {
    const IntVec& dstIDs = mapIt->second.dstIDs;
    bool& needTest       = mapIt->second.needTest;
    needTest = false;
    for (size_t i = 0; i < dstIDs.size(); i++) {
      const DstData& dstData = linkDsts[i];
      if (dstData.physics.getTestBits() != 0) {
        needTest = true;
        break;
      }
    }
  }

  // assign the LinkSrcFace bits
  for (size_t i = 0; i < linkSrcs.size(); i++) {
    const MeshFace* face = linkSrcs[i];
    MeshFace::SpecialData* sd =
      const_cast<MeshFace::SpecialData*>(face->getSpecialData());
    sd->stateBits |= MeshFace::LinkSrcFace;
    linkSrcSet.insert(face);
    linkFaceSet.insert(face);
  }

  // assign the LinkDstFace bits
  for (size_t i = 0; i < linkDsts.size(); i++) {
    const MeshFace* face = linkDsts[i].face;
    MeshFace::SpecialData* sd =
      const_cast<MeshFace::SpecialData*>(face->getSpecialData());
    sd->stateBits |= MeshFace::LinkDstFace;
    linkDstSet.insert(face);
    linkFaceSet.insert(face);
  }

  if (debugLinks >= 1) {
    printDebug();
  }
  return;
}


void LinkManager::buildNameMap()
{
  // get the potential named faces
  const ObstacleList& meshes = OBSTACLEMGR.getMeshes();
  for (unsigned int m = 0; m < meshes.size(); m++) {
    const MeshObstacle* mesh = (const MeshObstacle*) meshes[m];
    if (!mesh->getHasSpecialFaces()) {
      continue;
    }
    const int faceCount = mesh->getFaceCount();
    for (int f = 0; f < faceCount; f++) {
      const MeshFace* face = mesh->getFace(f);
      if (face->isSpecial()) {
        MeshFace::SpecialData* sd =
          const_cast<MeshFace::SpecialData*>(face->getSpecialData());
        // clear the LinkSrcFace and LinkDstFace bits
        sd->stateBits &= ~(MeshFace::LinkSrcFace | MeshFace::LinkDstFace);
        if (!sd->linkName.empty()) {
          nameFaceVec.push_back(NameFace(face->getLinkName(), face));
        }
      }
    }
  }
}


//============================================================================//

static std::string numberLinkName(int number)
{
  std::string name = "$mt";
  char buffer[8];
  snprintf(buffer, sizeof(buffer), "%i", (number / 2));
  name += buffer;
  name += ":";
  name += ((number % 2) == 0) ? "f" : "b";
  return name;
}


static bool allDigits(const std::string& s)
{
  for (size_t i = 0; i < s.size(); i++) {
    if ((s[i] < '0') || (s[i] > '9')) {
      return false;
    }
  }
  return true;
}



bool LinkManager::matchLinks(const StringVec& patterns, FaceSet& faces) const
{
  faces.clear();

  // no chars, no service
  if (patterns.size() <= 0) {
    return false;
  }

  for (size_t i = 0; i < patterns.size(); i++) {
    std::string glob = patterns[i];
    if (glob.empty() || glob == "/") {
      continue;
    }
    if (glob[0] == '/') {
      glob = glob.substr(1);
    }

    if (allDigits(glob)) {
      // convert to a default link name
      glob = numberLinkName(atoi(glob.c_str()));
    }
    else {
      // make the teleporter face specification case-independent
      const size_t last = (size_t)glob.size() - 1;
      if ((glob.size() > 2) && (glob[last - 1] == ':') &&
          ((glob[last] == 'F') || (glob[last] == 'B'))) {
        glob[last] = tolower(glob[last]);
      }
    }

    // insert the matches
    NameFaceVec::const_iterator it;
    for (it = nameFaceVec.begin(); it != nameFaceVec.end(); ++it) {
      if (debugLinks >= 3) {
        logDebugMessage(0, "link glob_match:  %s  vs.  %s\n",
                           glob.c_str(), it->name.c_str());
      }
      if (glob_match(glob, it->name)) {
        faces.insert(it->face);
      }
    }
  }

  return !faces.empty();
}


//============================================================================//

void LinkManager::crossLink()
{
  NameFaceVec::const_iterator it;
  for (it = nameFaceVec.begin(); it != nameFaceVec.end(); ++it) {
    const std::string&   srcName = it->name;
    const MeshFace* srcFace = it->face;
    // already linked?
    if (linkMap.find(srcFace) != linkMap.end()) {
      continue;
    }

    // a teleporter link?
    const MeshObstacle* srcMesh = srcFace->getMesh();
    if ((srcMesh == NULL) ||
        ((srcMesh->getSource() & Obstacle::ContainerSource) == 0)) {
      continue;
    }
    // at least 2 chars are required  (:f or :b)
    if (srcName.size() < 2) {
      continue;
    }

    // setup the destination link name
    const char type = srcName[srcName.size() - 1];
    std::string dstName = srcName.substr(0, srcName.size() - 1);
    switch (type) {
      case 'f': { dstName += 'b';  break; }
      case 'b': { dstName += 'f';  break; }
      default:  { dstName.clear(); break; }
    }
    if (dstName.empty()) {
      continue;
    }

    // find the dstFace, and create the link
    const int faceCount = srcMesh->getFaceCount();
    for (int f = 0; f < faceCount; f++) {
      const MeshFace* dstFace = srcMesh->getFace(f);
      if (dstFace->getLinkName() == dstName) {
        createLink(srcFace, dstFace, LinkPhysics());
        break;
      }
    }
  }
}


//============================================================================//

void LinkManager::createLink(const MeshFace* linkSrc,
                             const MeshFace* linkDst,
                             const LinkPhysics& physics)
{
  FaceIntMap::const_iterator srcIt = linkSrcMap.find(linkSrc);
  if (srcIt == linkSrcMap.end()) {
    linkSrcMap[linkSrc] = (int)linkSrcs.size();
    linkSrcs.push_back(linkSrc);
  }

  int dstID;
  DstData data(linkDst, physics);
  DstDataIntMap::const_iterator dstIt = linkDstMap.find(data);
  if (dstIt != linkDstMap.end()) {
    dstID = dstIt->second;
  }
  else {
    dstID = (int)linkDsts.size();
    linkDstMap[data] = dstID;
    linkDsts.push_back(data);
  }

  linkMap[linkSrc].dstIDs.push_back(dstID);
}


//============================================================================//

void LinkManager::getVariables(std::set<std::string>& vars) const
{
  for (size_t i = 0; i < linkDefs.size(); i++) {
    const LinkDef& linkDef = linkDefs[i];
    if (!linkDef.physics.shotBlockVar.empty()) {
      vars.insert(linkDef.physics.shotBlockVar);
    }
    if (!linkDef.physics.tankBlockVar.empty()) {
      vars.insert(linkDef.physics.tankBlockVar);
    }
  }
}


//============================================================================//

void LinkManager::printDebug()
{
  if (debugLinks < 2) {
    logDebugMessage(0, "\n");
  }
  else {
    logDebugMessage(0, "\n");
    for (size_t i = 0; i < linkDefs.size(); i++) {
      linkDefs[i].print(std::cout, "linkdefs: ");
    }

    logDebugMessage(0, "LinkSrcIDs:\n");
    for (size_t i = 0; i < linkSrcs.size(); i++) {
      logDebugMessage(0, "  linkSrc %i: /%s\n", i,
                      linkSrcs[i]->getLinkName().c_str());
    }
    logDebugMessage(0, "\n");

    logDebugMessage(0, "LinkDstIDs:\n");
    for (size_t i = 0; i < linkDsts.size(); i++) {
      logDebugMessage(0, "  linkDst %i: /%s\n", i,
                      linkDsts[i].face->getLinkName().c_str());
    }
    logDebugMessage(0, "\n");
  }

  logDebugMessage(0, "Potential Links:\n");
  NameFaceVec::const_iterator nameIt;
  for (nameIt = nameFaceVec.begin(); nameIt != nameFaceVec.end(); ++nameIt) {
    logDebugMessage(0, "  /%s\n", nameIt->name.c_str());
  }
  logDebugMessage(0, "\n");

  logDebugMessage(0, "LinkMap:\n");
  LinkMap::const_iterator mapIt;
  for (mapIt = linkMap.begin(); mapIt != linkMap.end(); ++mapIt) {
    const MeshFace* linkSrc = mapIt->first;
    logDebugMessage(0, "  src %s\n", linkSrc->getLinkName().c_str());
    const IntVec& dstIDs = mapIt->second.dstIDs;
    for (size_t d = 0; d < dstIDs.size(); d++) {
      const int dstID = dstIDs[d];
      const DstData* dd = getLinkDstData(dstID);
      const MeshFace* linkDst = dd->face;
      logDebugMessage(0, "    dst %s\n", linkDst->getLinkName().c_str());
    }
  }
  logDebugMessage(0, "\n");
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
