/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _MSC_VER
#pragma warning( 4: 4786 )
#endif
#include "common.h"
#include "Pack.h"
#include <math.h>
#include <sstream>

#include "WorldFileObject.h"
#include "WorldFileLocation.h"

WorldFileLocation::WorldFileLocation()
{
  pos[0] = pos[1] = pos[2] = 0.0f;
  rotation = 0.0f;
  size[0] = size[1] = size[2] = 1.0f;
  normal[0] = normal[1] = 0.0f;
  normal[2] = 1.0f;
}


bool WorldFileLocation::read(const char *cmd, std::istream& input)
{
  if ((strcasecmp(cmd, "pos") == 0) || 
      (strcasecmp(cmd, "position") == 0)) {
    input >> pos[0] >> pos[1] >> pos[2];
  } else if (strcasecmp(cmd, "size") == 0){
    input >> size[0] >> size[1] >> size[2];
  }
  else if ((strcasecmp(cmd, "rot") == 0) ||
           (strcasecmp(cmd, "rotation") == 0)) {
    
    std::string args;
    std::getline(input, args);
    std::istringstream parms(args);

    if (!(parms >> rotation)) {
      input.putback('\n');
      return false;
    }
    // convert to radians
    rotation = rotation * (M_PI / 180.0f);
    
    // check if we have a rotation normal
    std::string tmpStr;
    if (parms >> tmpStr) {
      if (tmpStr[0] == '#') {
        input.putback('\n');
        return true;
      } else {
	normal[0] = (float)atof(tmpStr.c_str());
      }
      
      if (!((parms >> normal[1]) && (parms >> normal[2]))) {
        input.putback('\n');
        return false;
      }
    }
    input.putback('\n');
  }
  else {
    return WorldFileObject::read(cmd, input);
  }

  return true;
}

void * WorldFileLocation::pack (void *buf) const
{
  buf = nboPackVector (buf, pos);
  buf = nboPackVector (buf, size);
  buf = nboPackFloat (buf, rotation);
  return buf;
}

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
