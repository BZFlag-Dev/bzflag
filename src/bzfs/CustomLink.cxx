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

// interface header
#include "CustomLink.h"

// system headers
#include <string.h>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <iostream>

// common headers
#include "TextUtils.h"


//============================================================================//

CustomLink::CustomLink(bool _linkSet)
: linkSet(_linkSet)
{
}


//============================================================================//

static void stripComments(std::string& s)
{
  const std::string::size_type pos = s.find_first_of('#');
  if (pos != std::string::npos) {
    s = s.substr(0, pos); // discard the comments
  }
}


static void getIntList(std::istream& input, std::vector<int>& list)
{
  std::string args;
  int value;

  list.clear();
  std::getline(input, args);
  std::istringstream parms(args);
  input.putback('\n');

  while (parms >> value) {
    list.push_back(value);
  }

  return;
}


static void getStringSet(std::istream& input, std::set<std::string>& list)
{
  std::string args;
  std::string value;

  list.clear();
  std::getline(input, args);
  std::istringstream parms(args);
  input.putback('\n');

  while (parms >> value) {
    list.insert(value);
  }

  return;
}


static void getAllowFlags(std::istream& input, std::set<std::string>& blocked)
{
  std::set<std::string> allowed;
  getStringSet(input, allowed);

  FlagSet::const_iterator it;

  // add the good
  const FlagSet& goodFlags = Flag::getGoodFlags();
  for (it = goodFlags.begin(); it != goodFlags.end(); it++) {
    const std::string& abbv = (*it)->flagAbbv;
    if (allowed.find(abbv) == allowed.end()) {
      blocked.insert(abbv);
    }
  }

  // and the bad
  const FlagSet& badFlags = Flag::getGoodFlags();
  for (it = badFlags.begin(); it != badFlags.end(); it++) {
    const std::string& abbv = (*it)->flagAbbv;
    if (allowed.find(abbv) == allowed.end()) {
      blocked.insert(abbv);
    }
  }
}


static uint8_t parseTeamBits(std::istream& input)
{
  uint8_t teamBits = 0;
  std::vector<int> teams;
  getIntList(input, teams);
  for (size_t i = 0; i < teams.size(); i++) {
    const int team = teams[i];
    if ((team >= 0) && (team < 8)) {
      teamBits |= (1 << team);
    }
  }
  return teamBits;
}


//============================================================================//

bool CustomLink::read(const char *cmd, std::istream& input)
{
  if ((strcasecmp(cmd, "src")  == 0) ||
      (strcasecmp(cmd, "from") == 0)) {
    std::string line;
    std::getline(input, line);
    input.putback('\n');

    stripComments(line);
    linkDef.srcs = TextUtils::tokenize(line, " \t\n\r");
  }
  else if ((strcasecmp(cmd, "dst") == 0) ||
           (strcasecmp(cmd, "to")  == 0)) {
    std::string line;
    std::getline(input, line);
    input.putback('\n');

    stripComments(line);
    linkDef.dsts = TextUtils::tokenize(line, " \t\n\r");
  }
  else if (strcasecmp(cmd, "addSrc") == 0) {
    std::string line;
    std::getline(input, line);
    input.putback('\n');

    stripComments(line);
    std::vector<std::string> srcs = TextUtils::tokenize(line, " \t\n\r");
    for (size_t i = 0; i < srcs.size(); i++) {
      linkDef.addSrc(srcs[i]);
    }
  }
  else if (strcasecmp(cmd, "addDst") == 0) {
    std::string line;
    std::getline(input, line);
    input.putback('\n');

    stripComments(line);
    std::vector<std::string> dsts = TextUtils::tokenize(line, " \t\n\r");
    for (size_t i = 0; i < dsts.size(); i++) {
      linkDef.addDst(dsts[i]);
    }
  }
  else if (strcasecmp(cmd, "addLink") == 0) {
    if (linkSet) {
      linkDefVec.push_back(linkDef);
    }
    else {
      std::cout << "'add' can only be used in 'linkset's" << std::endl;
      return false;
    }
  }
  else if (strcasecmp(cmd, "addBiDir") == 0) {
    if (linkSet) {
      // add the original link
      linkDefVec.push_back(linkDef); // add link

      // swap the srcs and dsts
      const std::vector<std::string> tmpSrcs = linkDef.srcs;
      const std::vector<std::string> tmpDsts = linkDef.dsts;
      linkDef.srcs = tmpDsts;
      linkDef.dsts = tmpSrcs;

      // add the swapped link
      linkDefVec.push_back(linkDef); // add link

      // revert the srcs and dsts
      linkDef.srcs = tmpSrcs;
      linkDef.dsts = tmpDsts;
    }
    else {
      std::cout << "'addBiDir' can only be used in 'linkset's" << std::endl;
      return false;
    }
  }
  //
  //  position control
  //
  else if (strcasecmp(cmd, "srcPosScale") == 0) {
    fvec3 v;
    if (!(input >> v.x >> v.y >> v.z)) {
      std::cout << "missing srcPosScale parameter" << std::endl;
      return false;
    }
    linkDef.physics.shotSrcPosScale = v;
    linkDef.physics.tankSrcPosScale = v;
  }
  else if (strcasecmp(cmd, "shotSrcPosScale") == 0) {
    fvec3 v;
    if (!(input >> v.x >> v.y >> v.z)) {
      std::cout << "missing shotSrcPosScale parameter" << std::endl;
      return false;
    }
    linkDef.physics.shotSrcPosScale = v;
  }
  else if (strcasecmp(cmd, "tankSrcPosScale") == 0) {
    fvec3 v;
    if (!(input >> v.x >> v.y >> v.z)) {
      std::cout << "missing tankSrcPosScale parameter" << std::endl;
      return false;
    }
    linkDef.physics.tankSrcPosScale = v;
  }
  //
  //  velocity control
  //
  else if (strcasecmp(cmd, "sameSpeed") == 0) {
    linkDef.physics.shotSameSpeed = true;
    linkDef.physics.tankSameSpeed = true;
  }
  else if (strcasecmp(cmd, "shotSameSpeed") == 0) {
    linkDef.physics.shotSameSpeed = true;
  }
  else if (strcasecmp(cmd, "tankSameSpeed") == 0) {
    linkDef.physics.tankSameSpeed = true;
  }
  else if (strcasecmp(cmd, "srcVelScale") == 0) {
    fvec3 v;
    if (!(input >> v.x >> v.y >> v.z)) {
      std::cout << "missing srcVelScale parameter" << std::endl;
      return false;
    }
    linkDef.physics.shotSrcVelScale = v;
    linkDef.physics.tankSrcVelScale = v;
  }
  else if (strcasecmp(cmd, "shotSrcVelScale") == 0) {
    fvec3 v;
    if (!(input >> v.x >> v.y >> v.z)) {
      std::cout << "missing shotSrcVelScale parameter" << std::endl;
      return false;
    }
    linkDef.physics.shotSrcVelScale = v;
  }
  else if (strcasecmp(cmd, "tankSrcVelScale") == 0) {
    fvec3 v;
    if (!(input >> v.x >> v.y >> v.z)) {
      std::cout << "missing tankSrcVelScale parameter" << std::endl;
      return false;
    }
    linkDef.physics.tankSrcVelScale = v;
  }
  else if (strcasecmp(cmd, "dstVelOffset") == 0) {
    fvec3 v;
    if (!(input >> v.x >> v.y >> v.z)) {
      std::cout << "missing dstVel parameter" << std::endl;
      return false;
    }
    linkDef.physics.shotDstVelOffset = v;
    linkDef.physics.tankDstVelOffset = v;
  }
  else if (strcasecmp(cmd, "shotDstVelOffset") == 0) {
    fvec3 v;
    if (!(input >> v.x >> v.y >> v.z)) {
      std::cout << "missing shotDstVelOffset parameter" << std::endl;
      return false;
    }
    linkDef.physics.shotDstVelOffset = v;
  }
  else if (strcasecmp(cmd, "tankDstVelOffset") == 0) {
    fvec3 v;
    if (!(input >> v.x >> v.y >> v.z)) {
      std::cout << "missing tankDstVelOffset parameter" << std::endl;
      return false;
    }
    linkDef.physics.tankDstVelOffset = v;
  }
  //
  //  angular control
  //
  else if (strcasecmp(cmd, "tankAngle") == 0) {
    float v;
    if (!(input >> v)) {
      std::cout << "missing tankAngle parameter" << std::endl;
      return false;
    }
    linkDef.physics.tankAngle = (v * DEG2RADf);
    linkDef.physics.tankForceAngle = true;
  }
  else if (strcasecmp(cmd, "tankAngleOffset") == 0) {
    float v;
    if (!(input >> v)) {
      std::cout << "missing tankAngleOffset parameter" << std::endl;
      return false;
    }
    linkDef.physics.tankAngleOffset = (v * DEG2RADf);
  }
  else if (strcasecmp(cmd, "tankAngleScale") == 0) {
    float scale;
    if (!(input >> scale)) {
      std::cout << "missing tankAngleScale parameter" << std::endl;
      return false;
    }
    linkDef.physics.tankAngleScale = scale;
  }
  else if (strcasecmp(cmd, "tankAngVel") == 0) {
    float v;
    if (!(input >> v)) {
      std::cout << "missing tankAngVel parameter" << std::endl;
      return false;
    }
    linkDef.physics.tankAngVel = (v * DEG2RADf);
    linkDef.physics.tankForceAngVel = true;
  }
  else if (strcasecmp(cmd, "tankAngVelOffset") == 0) {
    float v;
    if (!(input >> v)) {
      std::cout << "missing tankAngVelOffset parameter" << std::endl;
      return false;
    }
    linkDef.physics.tankAngVelOffset = (v * DEG2RADf);
  }
  else if (strcasecmp(cmd, "tankAngVelScale") == 0) {
    float scale;
    if (!(input >> scale)) {
      std::cout << "missing tankAngVelScale parameter" << std::endl;
      return false;
    }
    linkDef.physics.tankAngVelScale = scale;
  }
  //
  //  speed blocks
  //
  else if (strcasecmp(cmd, "shotMinSpeed") == 0) {
    float v;
    if (!(input >> v)) {
      std::cout << "missing shotMinSpeed parameter" << std::endl;
      return false;
    }
    linkDef.physics.shotMinSpeed = v;
  }
  else if (strcasecmp(cmd, "tankMinSpeed") == 0) {
    float v;
    if (!(input >> v)) {
      std::cout << "missing tankMinSpeed parameter" << std::endl;
      return false;
    }
    linkDef.physics.tankMinSpeed = v;
  }
  else if (strcasecmp(cmd, "shotMaxSpeed") == 0) {
    float v;
    if (!(input >> v)) {
      std::cout << "missing shotMaxSpeed parameter" << std::endl;
      return false;
    }
    linkDef.physics.shotMaxSpeed = v;
  }
  else if (strcasecmp(cmd, "tankMaxSpeed") == 0) {
    float v;
    if (!(input >> v)) {
      std::cout << "missing tankMaxSpeed parameter" << std::endl;
      return false;
    }
    linkDef.physics.tankMaxSpeed = v;
  }
  //
  //  angle blocks
  //
  else if (strcasecmp(cmd, "shotMinAngle") == 0) {
    float v;
    if (!(input >> v)) {
      std::cout << "missing shotMinAngle parameter" << std::endl;
      return false;
    }
    linkDef.physics.shotMinAngle = (v * DEG2RADf);
  }
  else if (strcasecmp(cmd, "tankMinAngle") == 0) {
    float v;
    if (!(input >> v)) {
      std::cout << "missing tankMinAngle parameter" << std::endl;
      return false;
    }
    linkDef.physics.tankMinAngle = (v * DEG2RADf);
  }
  else if (strcasecmp(cmd, "shotMaxAngle") == 0) {
    float v;
    if (!(input >> v)) {
      std::cout << "missing shotMaxAngle parameter" << std::endl;
      return false;
    }
    linkDef.physics.shotMaxAngle = (v * DEG2RADf);
  }
  else if (strcasecmp(cmd, "tankMaxAngle") == 0) {
    float v;
    if (!(input >> v)) {
      std::cout << "missing tankMaxAngle parameter" << std::endl;
      return false;
    }
    linkDef.physics.tankMaxAngle = (v * DEG2RADf);
  }
  //
  //  team blocks
  //
  else if (strcasecmp(cmd, "blockTeams") == 0) {
    const uint8_t teamBits = parseTeamBits(input);
    if (teamBits == 0) {
      std::cout << "invalid blockTeams parameters" << std::endl;
      return false;
    }
    linkDef.physics.shotBlockTeams = teamBits;
    linkDef.physics.tankBlockTeams = teamBits;
  }
  else if (strcasecmp(cmd, "shotBlockTeams") == 0) {
    const uint8_t teamBits = parseTeamBits(input);
    if (teamBits == 0) {
      std::cout << "invalid shotBlockTeams parameters" << std::endl;
      return false;
    }
    linkDef.physics.shotBlockTeams = teamBits;
  }
  else if (strcasecmp(cmd, "tankBlockTeams") == 0) {
    const uint8_t teamBits = parseTeamBits(input);
    if (teamBits == 0) {
      std::cout << "invalid tankBlockTeams parameters" << std::endl;
      return false;
    }
    linkDef.physics.tankBlockTeams = teamBits;
  }
  else if (strcasecmp(cmd, "allowTeams") == 0) {
    const uint8_t teamBits = parseTeamBits(input);
    if (teamBits == 0) {
      std::cout << "invalid allowTeams parameters" << std::endl;
      return false;
    }
    linkDef.physics.shotBlockTeams = ~teamBits;
    linkDef.physics.tankBlockTeams = ~teamBits;
  }
  else if (strcasecmp(cmd, "shotAllowTeams") == 0) {
    const uint8_t teamBits = parseTeamBits(input);
    if (teamBits == 0) {
      std::cout << "invalid shotAllowTeams parameters" << std::endl;
      return false;
    }
    linkDef.physics.shotBlockTeams = ~teamBits;
  }
  else if (strcasecmp(cmd, "tankAllowTeams") == 0) {
    const uint8_t teamBits = parseTeamBits(input);
    if (teamBits == 0) {
      std::cout << "invalid tankAllowTeams parameters" << std::endl;
      return false;
    }
    linkDef.physics.tankBlockTeams = ~teamBits;
  }
  //
  //  flag blocks
  //
  else if (strcasecmp(cmd, "shotBlockFlags") == 0) {
    getStringSet(input, linkDef.physics.shotBlockFlags);
  }
  else if (strcasecmp(cmd, "tankBlockFlags") == 0) {
    getStringSet(input, linkDef.physics.tankBlockFlags);
  }
  else if (strcasecmp(cmd, "shotAllowFlags") == 0) {
    getAllowFlags(input, linkDef.physics.shotBlockFlags);
  }
  else if (strcasecmp(cmd, "tankAllowFlags") == 0) {
    getAllowFlags(input, linkDef.physics.tankBlockFlags);
  }
  //
  //  BZDB blocks
  //
  else if (strcasecmp(cmd, "blockVar") == 0) {
    std::string value;
    if (!(input >> value)) {
      std::cout << "missing blockVar parameter" << std::endl;
      return false;
    }
    linkDef.physics.shotBlockVar = value;
    linkDef.physics.tankBlockVar = value;
  }
  else if (strcasecmp(cmd, "shotBlockVar") == 0) {
    std::string value;
    if (!(input >> value)) {
      std::cout << "missing shotBlockVar parameter" << std::endl;
      return false;
    }
    linkDef.physics.shotBlockVar = value;
  }
  else if (strcasecmp(cmd, "tankBlockVar") == 0) {
    std::string value;
    if (!(input >> value)) {
      std::cout << "missing tankBlockVar parameter" << std::endl;
      return false;
    }
    linkDef.physics.tankBlockVar = value;
  }
  //
  //  Pass messages
  //
  else if (strcasecmp(cmd, "passText") == 0) {
    std::string line;
    std::getline(input, line);
    input.putback('\n');
    line = TextUtils::trim(line);
    if (line.empty()) {
      std::cout << "missing passText parameter" << std::endl;
      return false;
    }
    linkDef.physics.shotPassText = line;
    linkDef.physics.tankPassText = line;
  }
  else if (strcasecmp(cmd, "shotPassText") == 0) {
    std::string line;
    std::getline(input, line);
    input.putback('\n');
    line = TextUtils::trim(line);
    if (line.empty()) {
      std::cout << "missing shotPassText parameter" << std::endl;
      return false;
    }
    linkDef.physics.shotPassText = line;
  }
  else if (strcasecmp(cmd, "tankPassText") == 0) {
    std::string line;
    std::getline(input, line);
    input.putback('\n');
    line = TextUtils::trim(line);
    if (line.empty()) {
      std::cout << "missing tankPassText parameter" << std::endl;
      return false;
    }
    linkDef.physics.tankPassText = line;
  }
  else {
    return WorldFileObject::read(cmd, input);
  }
  return true;
}


//============================================================================//


void CustomLink::writeToGroupDef(GroupDefinition *groupDef) const
{
  if (!linkSet) {
    groupDef->addLinkDef(new LinkDef(linkDef));
  }
  else {
    for (size_t i = 0; i < linkDefVec.size(); i++) {
      groupDef->addLinkDef(new LinkDef(linkDefVec[i]));
    }
  }
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
