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

//
// TeleporterSceneNodeGenerator
//

TeleporterSceneNodeGenerator::TeleporterSceneNodeGenerator(
				const Teleporter* _teleporter) :
				teleporter(_teleporter)
{
  // do nothing
}


TeleporterSceneNodeGenerator::~TeleporterSceneNodeGenerator()
{
  // do nothing
}


WallSceneNode* TeleporterSceneNodeGenerator::getNextNode(float /*uRepeats*/,
                                                         float /*vRepeats*/,
                                                         bool lod)
{
  static const fvec2 texCoords[][4] = {
    {fvec2(0.0f, 0.0f), fvec2(0.5f, 0.0f), fvec2(0.5f, 9.5f), fvec2(0.0f, 9.5f)},
    {fvec2(0.5f, 0.0f), fvec2(1.0f, 0.0f), fvec2(1.0f, 9.5f), fvec2(0.5f, 9.5f)},
    {fvec2(0.0f, 0.0f), fvec2(0.5f, 0.0f), fvec2(0.5f, 9.0f), fvec2(0.0f, 9.0f)},
    {fvec2(0.5f, 0.0f), fvec2(1.0f, 0.0f), fvec2(1.0f, 9.0f), fvec2(0.5f, 9.0f)},
    {fvec2(0.5f, 0.0f), fvec2(1.0f, 0.0f), fvec2(1.0f, 9.0f), fvec2(0.5f, 9.0f)},
    {fvec2(0.0f, 0.0f), fvec2(0.5f, 0.0f), fvec2(0.5f, 9.0f), fvec2(0.0f, 9.0f)},
    {fvec2(0.5f, 0.0f), fvec2(1.0f, 0.0f), fvec2(1.0f, 9.0f), fvec2(0.5f, 9.0f)},
    {fvec2(0.0f, 0.0f), fvec2(0.5f, 0.0f), fvec2(0.5f, 9.0f), fvec2(0.0f, 9.0f)},
    {fvec2(0.0f, 0.0f), fvec2(0.0f, 0.0f), fvec2(0.5f, 5.0f), fvec2(0.5f, 5.0f)},
    {fvec2(0.0f, 0.0f), fvec2(0.0f, 0.0f), fvec2(0.5f, 4.0f), fvec2(0.5f, 4.0f)},
    {fvec2(0.0f, 0.0f), fvec2(5.0f, 0.0f), fvec2(5.0f, 0.5f), fvec2(0.0f, 0.5f)},
    {fvec2(0.0f, 0.5f), fvec2(5.0f, 0.5f), fvec2(5.0f, 1.0f), fvec2(0.0f, 1.0f)}
  };

  if (fabsf(teleporter->getBorder()) < 1.0e-6f) {
    return NULL;
  }

  if (teleporter->isHorizontal()) {
    if (getNodeNumber() >= 16)
      return NULL;

    fvec3 base;
    fvec3 sEdge;
    fvec3 tEdge;

    const fvec3& pos = teleporter->getPosition();
    const fvec3& size = teleporter->getSize();
    const float c = cosf(teleporter->getRotation());
    const float s = sinf(teleporter->getRotation());
    const float w = teleporter->getWidth();
    const float d = teleporter->getBreadth();
    const float b = teleporter->getBorder();

    // NOTE -- 1,2,3,4: outer sides
    // 3,4,5,6: inner sides
    // 7,8,9,10: bottom sides
    // 11,12,13,14: top sides
    // 15,16 top and bottom teleport faces

    const int n = incNodeNumber();
    switch (n) {
      case 1:		    // -x outside edge
	base.x = pos.x + ((-w) * c - (-d) * s);
	base.y = pos.y + ((-w) * s + (-d) * c);
	base.z = pos.z + size.z - b;
	sEdge.x = 0.0f;
	sEdge.y = 0.0f;
	sEdge.z = b;
	tEdge.x = -s * (2.0f * d);
	tEdge.y = c * (2.0f * d);
	tEdge.z = 0.0f;
	break;
      case 2:		    // -y outside edge
	base.x = pos.x + ((-w) * c - (-d) * s);
	base.y = pos.y + ((-w) * s + (-d) * c);
	base.z = pos.z + size.z - b;
	sEdge.x = (2.0f * w) * c;
	sEdge.y = (2.0f * w) * s;
	sEdge.z = 0.0f;
	tEdge.x = 0.0f;
	tEdge.y = 0.0f;
	tEdge.z = b;
	break;
      case 3:		    // +x outside edge
	base.x = pos.x + ((w) * c - (d) * s);
	base.y = pos.y + ((w) * s + (d) * c);
	base.z = pos.z + size.z - b;
	sEdge.x = 0.0f;
	sEdge.y = 0.0f;
	sEdge.z = b;
	tEdge.x = -(-2.0f * d) * s;
	tEdge.y = (-2.0f * d) * c;
	tEdge.z = 0.0f;
	break;
      case 4:		    // +y outside edge
	base.x = pos.x + ((w) * c - (d) * s);
	base.y = pos.y + ((w) * s + (d) * c);
	base.z = pos.z + size.z - b;
	sEdge.x = (-2.0f * w) * c;
	sEdge.y = (-2.0f * w) * s;
	sEdge.z = 0.0f;
	tEdge.x = 0.0f;
	tEdge.y = 0.0f;
	tEdge.z = b;
	break;
      case 5:		    // -x inner edge
	base.x = pos.x + ((b - w) * c - (b - d) * s);
	base.y = pos.y + ((b - w) * s + (b - d) * c);
	base.z = pos.z + size.z - b;
	sEdge.x = -s * (2.0f * (d - b));
	sEdge.y = c * (2.0f * (d - b));
	sEdge.z = 0.0f;
	tEdge.x = 0.0f;
	tEdge.y = 0.0f;
	tEdge.z = b;
	break;
      case 6:		    // -y inner edge
	base.x = pos.x + ((b - w) * c - (b - d) * s);
	base.y = pos.y + ((b - w) * s + (b - d) * c);
	base.z = pos.z + size.z - b;
	sEdge.x = 0.0f;
	sEdge.y = 0.0f;
	sEdge.z = b;
	tEdge.x = (2.0f * (w - b)) * c;
	tEdge.y = (2.0f * (w - b)) * s;
	tEdge.z = 0.0f;
	break;
      case 7:		    // +x inner edge
	base.x = pos.x + ((w - b) * c - (d - b) * s);
	base.y = pos.y + ((w - b) * s + (d - b) * c);
	base.z = pos.z + size.z - b;
	sEdge.x = -(-2.0f * (d - b)) * s;
	sEdge.y = (-2.0f * (d - b)) * c;
	sEdge.z = 0.0f;
	tEdge.x = 0.0f;
	tEdge.y = 0.0f;
	tEdge.z = b;
	break;
      case 8:		    // +y inner edge
	base.x = pos.x + ((w - b) * c - (d - b) * s);
	base.y = pos.y + ((w - b) * s + (d - b) * c);
	base.z = pos.z + size.z - b;
	sEdge.x = 0.0f;
	sEdge.y = 0.0f;
	sEdge.z = b;
	tEdge.x = (-2.0f * (w - b)) * c;
	tEdge.y = (-2.0f * (w - b)) * s;
	tEdge.z = 0.0f;
	break;
      case 9:		    // -x bottom edge
	base.x = pos.x + ((-w) * c - (-d) * s);
	base.y = pos.y + ((-w) * s + (-d) * c);
	base.z = pos.z + size.z - b;
	sEdge.x = -s * (2.0f * d);
	sEdge.y = c * (2.0f * d);
	sEdge.z = 0.0f;
	tEdge.x = (b) * c;
	tEdge.y = (b) * s;
	tEdge.z = 0.0f;
	break;
      case 10:		   // -y bottom edge
	base.x = pos.x + ((-w) * c - (-d) * s);
	base.y = pos.y + ((-w) * s + (-d) * c);
	base.z = pos.z + size.z - b;
	sEdge.x = -s * (b);
	sEdge.y = c * (b);
	sEdge.z = 0.0f;
	tEdge.x = c * (2.0f * w);
	tEdge.y = s * (2.0f * w);
	tEdge.z = 0.0f;
	break;
      case 11:		   // +x bottom edge
	base.x = pos.x + ((w) * c - (d) * s);
	base.y = pos.y + ((w) * s + (d) * c);
	base.z = pos.z + size.z - b;
	sEdge.x = -(-2.0f * (d)) * s;
	sEdge.y = (-2.0f * (d)) * c;
	sEdge.z = 0.0f;
	tEdge.x = c * (-b);
	tEdge.y = s * (-b);
	tEdge.z = 0.0f;
	break;
      case 12:		   // +y bottom edge
	base.x = pos.x + ((w) * c - (d) * s);
	base.y = pos.y + ((w) * s + (d) * c);
	base.z = pos.z + size.z - b;
	sEdge.x = -s * (-b);
	sEdge.y = c * (-b);
	sEdge.z = 0.0f;
	tEdge.x = (-2.0f * (w)) * c;
	tEdge.y = (-2.0f * (w)) * s;
	tEdge.z = 0.0f;
	break;
      case 13:		   // -x top edge
	base.x = pos.x + ((-w) * c - (-d) * s);
	base.y = pos.y + ((-w) * s + (-d) * c);
	base.z = pos.z + size.z;
	sEdge.x = c * (b);
	sEdge.y = s * (b);
	sEdge.z = 0.0f;
	tEdge.x = -s * (2.0f * (d));
	tEdge.y = c * (2.0f * (d));
	tEdge.z = 0.0f;
	break;
      case 14:		   // -y top edge
	base.x = pos.x + ((-w) * c - (-d) * s);
	base.y = pos.y + ((-w) * s + (-d) * c);
	base.z = pos.z + size.z;
	sEdge.x = c * (2.0f * (w));
	sEdge.y = s * (2.0f * (w));
	sEdge.z = 0.0f;
	tEdge.x = -s * (b);
	tEdge.y = c * (b);
	tEdge.z = 0.0f;
	break;
      case 15:		   // +x top edge
	base.x = pos.x + ((w) * c - (d) * s);
	base.y = pos.y + ((w) * s + (d) * c);
	base.z = pos.z + size.z;
	sEdge.x = c * (-b);
	sEdge.y = s * (-b);
	sEdge.z = 0.0f;
	tEdge.x = -s * (-2.0f * (d));
	tEdge.y = c * (-2.0f * (d));
	tEdge.z = 0.0f;
	break;
      case 16:		   // +y top edge
	base.x = pos.x + ((w) * c - (d) * s);
	base.y = pos.y + ((w) * s + (d) * c);
	base.z = pos.z + size.z;
	sEdge.x = c * (2.0f * (-w));
	sEdge.y = s * (2.0f * (-w));
	sEdge.z = 0.0f;
	tEdge.x = -s * (-b);
	tEdge.y = c * (-b);
	tEdge.z = 0.0f;
	break;
    }

    float u, v, uc, vc;
    if (n >= 1 && n <= 16) {
      u  = texCoords[0][0].x;
      v  = texCoords[0][0].y;
      uc = texCoords[0][1].x - u;
      vc = texCoords[0][3].y - v;
    }
    else {
      u = v = 0.0f;
      uc = vc = 1.0f;
    }
    return new QuadWallSceneNode(base, sEdge, tEdge, u, v, uc, vc, lod);
  }
  else {
    if (getNodeNumber () >= 12)
      return NULL;

    fvec3 base;
    fvec3 sEdge;
    fvec3 tEdge;
    const fvec3& pos = teleporter->getPosition();
    const float c = cosf(teleporter->getRotation());
    const float s = sinf(teleporter->getRotation());
    const float h = teleporter->getBreadth() - teleporter->getBorder();
    const float b = 0.5f * teleporter->getBorder();
    const float d = h + b;
    const float z = teleporter->getHeight() - teleporter->getBorder();
    fvec2 x, y;
    x.x = c;
    x.y = s;
    y.x = -s;
    y.y = c;
    // NOTE -- 1,2: outer sides
    // 3,4: inner sides
    // 5-8: front and back vertical sides
    // 9,10: horizontal bar top and bottom
    // 11,12: front and back horizontal sides
    // 13,14: front and back teleport faces
    const int n = incNodeNumber();
    switch (n) {
      case 1:
	base.x = pos.x + d * y.x + b * x.x + b * y.x;
	base.y = pos.y + d * y.y + b * x.y + b * y.y;
	base.z = pos.z;
	sEdge.x = -2.0f * b * x.x;
	sEdge.y = -2.0f * b * x.y;
	sEdge.z = 0.0f;
	tEdge.x = 0.0f;
	tEdge.y = 0.0f;
	tEdge.z = z + 2.0f * b;
	break;
      case 2:
	base.x = pos.x - d * y.x - b * x.x - b * y.x;
	base.y = pos.y - d * y.y - b * x.y - b * y.y;
	base.z = pos.z;
	sEdge.x = 2.0f * b * x.x;
	sEdge.y = 2.0f * b * x.y;
	sEdge.z = 0.0f;
	tEdge.x = 0.0f;
	tEdge.y = 0.0f;
	tEdge.z = z + 2.0f * b;
	break;
      case 3:
	base.x = pos.x + d * y.x - b * x.x - b * y.x;
	base.y = pos.y + d * y.y - b * x.y - b * y.y;
	base.z = pos.z;
	sEdge.x = 2.0f * b * x.x;
	sEdge.y = 2.0f * b * x.y;
	sEdge.z = 0.0f;
	tEdge.x = 0.0f;
	tEdge.y = 0.0f;
	tEdge.z = z;
	break;
      case 4:
	base.x = pos.x - d * y.x + b * x.x + b * y.x;
	base.y = pos.y - d * y.y + b * x.y + b * y.y;
	base.z = pos.z;
	sEdge.x = -2.0f * b * x.x;
	sEdge.y = -2.0f * b * x.y;
	sEdge.z = 0.0f;
	tEdge.x = 0.0f;
	tEdge.y = 0.0f;
	tEdge.z = z;
	break;
      case 5:
	base.x = pos.x + d * y.x + b * x.x - b * y.x;
	base.y = pos.y + d * y.y + b * x.y - b * y.y;
	base.z = pos.z;
	sEdge.x = 2.0f * b * y.x;
	sEdge.y = 2.0f * b * y.y;
	sEdge.z = 0.0f;
	tEdge.x = 0.0f;
	tEdge.y = 0.0f;
	tEdge.z = z;
	break;
      case 6:
	base.x = pos.x - d * y.x - b * x.x + b * y.x;
	base.y = pos.y - d * y.y - b * x.y + b * y.y;
	base.z = pos.z;
	sEdge.x = -2.0f * b * y.x;
	sEdge.y = -2.0f * b * y.y;
	sEdge.z = 0.0f;
	tEdge.x = 0.0f;
	tEdge.y = 0.0f;
	tEdge.z = z;
	break;
      case 7:
	base.x = pos.x + d * y.x - b * x.x + b * y.x;
	base.y = pos.y + d * y.y - b * x.y + b * y.y;
	base.z = pos.z;
	sEdge.x = -2.0f * b * y.x;
	sEdge.y = -2.0f * b * y.y;
	sEdge.z = 0.0f;
	tEdge.x = 0.0f;
	tEdge.y = 0.0f;
	tEdge.z = z;
	break;
      case 8:
	base.x = pos.x - d * y.x + b * x.x - b * y.x;
	base.y = pos.y - d * y.y + b * x.y - b * y.y;
	base.z = pos.z;
	sEdge.x = 2.0f * b * y.x;
	sEdge.y = 2.0f * b * y.y;
	sEdge.z = 0.0f;
	tEdge.x = 0.0f;
	tEdge.y = 0.0f;
	tEdge.z = z;
	break;
      case 9:
	base.x = pos.x - d * y.x - b * x.x - b * y.x;
	base.y = pos.y - d * y.y - b * x.y - b * y.y;
	base.z = pos.z + z + 2.0f * b;
	sEdge.x = 2.0f * b * x.x;
	sEdge.y = 2.0f * b * x.y;
	sEdge.z = 0.0f;
	tEdge.x = 2.0f * (d + b) * y.x;
	tEdge.y = 2.0f * (d + b) * y.y;
	tEdge.z = 0.0f;
	break;
      case 10:
	base.x = pos.x - d * y.x + b * x.x + b * y.x;
	base.y = pos.y - d * y.y + b * x.y + b * y.y;
	base.z = pos.z + z;
	sEdge.x = -2.0f * b * x.x;
	sEdge.y = -2.0f * b * x.y;
	sEdge.z = 0.0f;
	tEdge.x = 2.0f * (d - b) * y.x;
	tEdge.y = 2.0f * (d - b) * y.y;
	tEdge.z = 0.0f;
	break;
      case 11:
	base.x = pos.x - d * y.x + b * x.x - b * y.x;
	base.y = pos.y - d * y.y + b * x.y - b * y.y;
	base.z = pos.z + z;
	sEdge.x = 2.0f * (d + b) * y.x;
	sEdge.y = 2.0f * (d + b) * y.y;
	sEdge.z = 0.0f;
	tEdge.x = 0.0f;
	tEdge.y = 0.0f;
	tEdge.z = 2.0f * b;
	break;
      case 12:
	base.x = pos.x + d * y.x - b * x.x + b * y.x;
	base.y = pos.y + d * y.y - b * x.y + b * y.y;
	base.z = pos.z + z;
	sEdge.x = -2.0f * (d + b) * y.x;
	sEdge.y = -2.0f * (d + b) * y.y;
	sEdge.z = 0.0f;
	tEdge.x = 0.0f;
	tEdge.y = 0.0f;
	tEdge.z = 2.0f * b;
	break;
    }
    float u, v, uc, vc;
    if (n >= 1 && n <= 12) {
      u  = texCoords[n - 1][0].x;
      v  = texCoords[n - 1][0].y;
      uc = texCoords[n - 1][1].x - u;
      vc = texCoords[n - 1][3].y - v;
    }
    else {
      u = v = 0.0f;
      uc = vc = 1.0f;
    }
    return new QuadWallSceneNode(base, sEdge, tEdge, u, v, uc, vc, lod);
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
