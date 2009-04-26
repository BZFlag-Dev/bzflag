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

// common headers
#include "TextUtils.h"


//============================================================================//

static void stripComments(std::string& s)
{
  const std::string::size_type pos = s.find_first_of('#');
  if (pos != std::string::npos) {
    s = s.substr(0, pos); // discard the comments
  }
}


//============================================================================//

CustomLink::CustomLink()
{
}


bool CustomLink::read(const char *cmd, std::istream& input)
{
  if ((strcasecmp(cmd, "src")  == 0) ||
      (strcasecmp(cmd, "from") == 0)) {
    std::string line;
    std::getline(input, line);
    input.putback('\n');

    stripComments(line);
    std::vector<std::string> srcs = TextUtils::tokenize(line, " \t\n\r");
    for (size_t i = 0; i < srcs.size(); i++) {
      linkDef.addSrc(srcs[i]);
    }
  }
  else if ((strcasecmp(cmd, "dst") == 0) ||
           (strcasecmp(cmd, "to")  == 0)) {
    std::string line;
    std::getline(input, line);
    input.putback('\n');

    stripComments(line);
    std::vector<std::string> dsts = TextUtils::tokenize(line, " \t\n\r");
    for (size_t i = 0; i < dsts.size(); i++) {
      linkDef.addDst(dsts[i]);
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
  else if (strcasecmp(cmd, "dstVel") == 0) {
    fvec3 v;
    if (!(input >> v.x >> v.y >> v.z)) {
      std::cout << "missing dstVel parameter" << std::endl;
      return false;
    }
    linkDef.physics.shotDstVel = v;
    linkDef.physics.tankDstVel = v;
  }
  else if (strcasecmp(cmd, "shotDstVel") == 0) {
    fvec3 v;
    if (!(input >> v.x >> v.y >> v.z)) {
      std::cout << "missing shotDstVel parameter" << std::endl;
      return false;
    }
    linkDef.physics.shotDstVel = v;
  }
  else if (strcasecmp(cmd, "tankDstVel") == 0) {
    fvec3 v;
    if (!(input >> v.x >> v.y >> v.z)) {
      std::cout << "missing tankDstVel parameter" << std::endl;
      return false;
    }
    linkDef.physics.tankDstVel = v;
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
    float v;
    if (!(input >> v)) {
      std::cout << "missing tankAngVelScale parameter" << std::endl;
      return false;
    }
    linkDef.physics.tankAngVelScale = v;
  }
  else {
    return WorldFileObject::read(cmd, input);
  }
  return true;
}


void CustomLink::writeToGroupDef(GroupDefinition *groupdef) const
{
  groupdef->addLinkDef(new LinkDef(linkDef));
}


//============================================================================//


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
