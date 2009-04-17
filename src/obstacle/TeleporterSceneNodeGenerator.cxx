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

#include <math.h>
#include "TeleporterSceneNodeGenerator.h"
#include "Teleporter.h"
#include "bzfgl.h"
#include "QuadWallSceneNode.h"


static const fvec2 texCoords[][4] = {
  { fvec2(0.0f, 0.0f), fvec2(0.5f, 0.0f), fvec2(0.5f, 9.5f), fvec2(0.0f, 9.5f) },
  { fvec2(0.5f, 0.0f), fvec2(1.0f, 0.0f), fvec2(1.0f, 9.5f), fvec2(0.5f, 9.5f) },
  { fvec2(0.0f, 0.0f), fvec2(0.5f, 0.0f), fvec2(0.5f, 9.0f), fvec2(0.0f, 9.0f) },
  { fvec2(0.5f, 0.0f), fvec2(1.0f, 0.0f), fvec2(1.0f, 9.0f), fvec2(0.5f, 9.0f) },
  { fvec2(0.5f, 0.0f), fvec2(1.0f, 0.0f), fvec2(1.0f, 9.0f), fvec2(0.5f, 9.0f) },
  { fvec2(0.0f, 0.0f), fvec2(0.5f, 0.0f), fvec2(0.5f, 9.0f), fvec2(0.0f, 9.0f) },
  { fvec2(0.5f, 0.0f), fvec2(1.0f, 0.0f), fvec2(1.0f, 9.0f), fvec2(0.5f, 9.0f) },
  { fvec2(0.0f, 0.0f), fvec2(0.5f, 0.0f), fvec2(0.5f, 9.0f), fvec2(0.0f, 9.0f) },
  { fvec2(0.0f, 0.0f), fvec2(0.0f, 0.0f), fvec2(0.5f, 5.0f), fvec2(0.5f, 5.0f) },
  { fvec2(0.0f, 0.0f), fvec2(0.0f, 0.0f), fvec2(0.5f, 4.0f), fvec2(0.5f, 4.0f) },
  { fvec2(0.0f, 0.0f), fvec2(5.0f, 0.0f), fvec2(5.0f, 0.5f), fvec2(0.0f, 0.5f) },
  { fvec2(0.0f, 0.5f), fvec2(5.0f, 0.5f), fvec2(5.0f, 1.0f), fvec2(0.0f, 1.0f) }
};


//============================================================================//
//
// TeleporterSceneNodeGenerator
//

TeleporterSceneNodeGenerator::TeleporterSceneNodeGenerator(const Teleporter* tele)
: teleporter(tele)
{
  // do nothing
}


TeleporterSceneNodeGenerator::~TeleporterSceneNodeGenerator()
{
  // do nothing
}


//============================================================================//

WallSceneNode* TeleporterSceneNodeGenerator::getNextNode(float /*uRepeats*/,
                                                         float /*vRepeats*/,
                                                         bool lod)
{
  if (fabsf(teleporter->getBorder()) < 1.0e-6f) {
    return NULL;
  }

  fvec3 base, sEdge, tEdge;
  float u, v, uc, vc;

  if (getNodeNumber() >= 12) {
    return NULL;
  }

  const int partNum = incNodeNumber();

  // NOTE:
  //    1 -  2:  outer sides
  //    3 -  4:  inner sides
  //    5 -  8:  front and back vertical sides
  //    9 - 10:  horizontal bar top and bottom
  //   11 - 12:  front and back horizontal sides
  //   13 - 14:  front and back teleport faces

  base  = fvec3(0.0f, 0.0f, 0.0f);
  sEdge = fvec3(0.0f, 0.0f, 0.0f);
  tEdge = fvec3(0.0f, 0.0f, 0.0f);

  const fvec3& pos = teleporter->getPosition();
  const fvec3& size = teleporter->getSize();
  const float br = teleporter->getBorder();
  const float hb = br * 0.5f; // half border
  const float h = size.y - br;
  const float d = h + hb;
  const float z = size.z - br;

  switch (partNum) {
    case 1: {
      base    = fvec3(+hb, +(d + hb), 0.0f);
      sEdge.x = -br;
      tEdge.z = z + br;
      break;
    }
    case 2: {
      base    = fvec3(-hb, -(d + hb), 0.0f);
      sEdge.x = br;
      tEdge.z = z + br;
      break;
    }
    case 3: {
      base    = fvec3(-hb, +(d - hb), 0.0f);
      sEdge.x = br;
      tEdge.z = z;
      break;
    }
    case 4: {
      base    = fvec3(+hb, -(d - hb), 0.0f);
      sEdge.x = -br;
      tEdge.z = z;
      break;
    }
    case 5: {
      base    = fvec3(+hb, +(d - hb), 0.0f);
      sEdge.y = br;
      tEdge.z = z;
      break;
    }
    case 6: {
      base    = fvec3(-hb, -(d - hb), 0.0f);
      sEdge.y = -br;
      tEdge.z = z;
      break;
    }
    case 7: {
      base    = fvec3(-hb, +(d + hb), 0.0f);
      sEdge.y = -br;
      tEdge.z = z;
      break;
    }
    case 8: {
      base    = fvec3(+hb, -(d + hb), 0.0f);
      sEdge.y = br;
      tEdge.z = z;
      break;
    }
    case 9: {
      base    = fvec3(-hb, -(d + hb), z + br);
      sEdge.x = br;
      tEdge.y = 2.0f * (d + hb);
      break;
    }
    case 10: {
      base    = fvec3(+hb, -(d - hb), z);
      sEdge.x = -br;
      tEdge.y = 2.0f * (d - hb);
      break;
    }
    case 11: {
      base    = fvec3(+hb, -(d + hb), z);
      sEdge.y = 2.0f * (d + hb);
      tEdge.z = br;
      break;
    }
    case 12: {
      base    = fvec3(-hb, +(d + hb), z);
      sEdge.y = -2.0f * (d + hb);
      tEdge.z = br;
      break;
    }
    default: {
      return NULL;
    }
  }

  if ((partNum >= 1) && (partNum <= 12)) {
    u  = texCoords[partNum - 1][0].x;
    v  = texCoords[partNum - 1][0].y;
    uc = texCoords[partNum - 1][1].x - u;
    vc = texCoords[partNum - 1][3].y - v;
  }
  else {
    u  = v  = 0.0f;
    uc = vc = 1.0f;
  }

  // rotate the vectors
  const float radians = teleporter->getRotation();
  fvec3::rotateZ(base,  radians);
  fvec3::rotateZ(sEdge, radians);
  fvec3::rotateZ(tEdge, radians);

  // translate the base
  base += pos;

  return new QuadWallSceneNode(base, sEdge, tEdge, u, v, uc, vc, lod);
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
