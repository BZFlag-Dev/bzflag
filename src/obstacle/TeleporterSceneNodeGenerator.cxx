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

WallSceneNode*		TeleporterSceneNodeGenerator::getNextNode(
				float /*uRepeats*/, float /*vRepeats*/,
				bool lod)
{
  static const float texCoords[][4][2] = {
			 { { 0.0f, 0.0f }, { 0.5f, 0.0f }, { 0.5f, 9.5f }, { 0.0f, 9.5f } },
			 { { 0.5f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 9.5f }, { 0.5f, 9.5f } },
			 { { 0.0f, 0.0f }, { 0.5f, 0.0f }, { 0.5f, 9.0f }, { 0.0f, 9.0f } },
			 { { 0.5f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 9.0f }, { 0.5f, 9.0f } },
			 { { 0.5f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 9.0f }, { 0.5f, 9.0f } },
			 { { 0.0f, 0.0f }, { 0.5f, 0.0f }, { 0.5f, 9.0f }, { 0.0f, 9.0f } },
			 { { 0.5f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 9.0f }, { 0.5f, 9.0f } },
			 { { 0.0f, 0.0f }, { 0.5f, 0.0f }, { 0.5f, 9.0f }, { 0.0f, 9.0f } },
			 { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.5f, 5.0f }, { 0.5f, 5.0f } },
			 { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.5f, 4.0f }, { 0.5f, 4.0f } },
			 { { 0.0f, 0.0f }, { 5.0f, 0.0f }, { 5.0f, 0.5f }, { 0.0f, 0.5f } },
			 { { 0.0f, 0.5f }, { 5.0f, 0.5f }, { 5.0f, 1.0f }, { 0.0f, 1.0f } }
			};

  if (getNodeNumber() == 14) return NULL;

  GLfloat base[3];
  GLfloat sEdge[3];
  GLfloat tEdge[3];
  const float* pos = teleporter->getPosition();
  const float c = cosf(teleporter->getRotation());
  const float s = sinf(teleporter->getRotation());
  const float w = teleporter->getWidth();
  const float h = teleporter->getBreadth() - teleporter->getBorder();
  const float b = 0.5f * teleporter->getBorder();
  const float d = h + b;
  const float z = teleporter->getHeight() - teleporter->getBorder();
  GLfloat x[2], y[2];
  x[0] = c;
  x[1] = s;
  y[0] = -s;
  y[1] = c;
  // NOTE --	1,2:	outer sides
  //		3,4:	inner sides
  //		5-8:	front and back vertical sides
  //		9,10:	horizontal bar top and bottom
  //		11,12:	front and back horizontal sides
  //		13,14:	front and back teleport faces
  const int n = incNodeNumber();
  switch (n) {
    case 1:
      base[0] = pos[0] + d * y[0] + b * x[0] + b * y[0];
      base[1] = pos[1] + d * y[1] + b * x[1] + b * y[1];
      base[2] = pos[2];
      sEdge[0] = -2.0f * b * x[0];
      sEdge[1] = -2.0f * b * x[1];
      sEdge[2] = 0.0f;
      tEdge[0] = 0.0f;
      tEdge[1] = 0.0f;
      tEdge[2] = z + 2.0f * b;
      break;
    case 2:
      base[0] = pos[0] - d * y[0] - b * x[0] - b * y[0];
      base[1] = pos[1] - d * y[1] - b * x[1] - b * y[1];
      base[2] = pos[2];
      sEdge[0] = 2.0f * b * x[0];
      sEdge[1] = 2.0f * b * x[1];
      sEdge[2] = 0.0f;
      tEdge[0] = 0.0f;
      tEdge[1] = 0.0f;
      tEdge[2] = z + 2.0f * b;
      break;
    case 3:
      base[0] = pos[0] + d * y[0] - b * x[0] - b * y[0];
      base[1] = pos[1] + d * y[1] - b * x[1] - b * y[1];
      base[2] = pos[2];
      sEdge[0] = 2.0f * b * x[0];
      sEdge[1] = 2.0f * b * x[1];
      sEdge[2] = 0.0f;
      tEdge[0] = 0.0f;
      tEdge[1] = 0.0f;
      tEdge[2] = z;
      break;
    case 4:
      base[0] = pos[0] - d * y[0] + b * x[0] + b * y[0];
      base[1] = pos[1] - d * y[1] + b * x[1] + b * y[1];
      base[2] = pos[2];
      sEdge[0] = -2.0f * b * x[0];
      sEdge[1] = -2.0f * b * x[1];
      sEdge[2] = 0.0f;
      tEdge[0] = 0.0f;
      tEdge[1] = 0.0f;
      tEdge[2] = z;
      break;
    case 5:
      base[0] = pos[0] + d * y[0] + b * x[0] - b * y[0];
      base[1] = pos[1] + d * y[1] + b * x[1] - b * y[1];
      base[2] = pos[2];
      sEdge[0] = 2.0f * b * y[0];
      sEdge[1] = 2.0f * b * y[1];
      sEdge[2] = 0.0f;
      tEdge[0] = 0.0f;
      tEdge[1] = 0.0f;
      tEdge[2] = z;
      break;
    case 6:
      base[0] = pos[0] - d * y[0] - b * x[0] + b * y[0];
      base[1] = pos[1] - d * y[1] - b * x[1] + b * y[1];
      base[2] = pos[2];
      sEdge[0] = -2.0f * b * y[0];
      sEdge[1] = -2.0f * b * y[1];
      sEdge[2] = 0.0f;
      tEdge[0] = 0.0f;
      tEdge[1] = 0.0f;
      tEdge[2] = z;
      break;
    case 7:
      base[0] = pos[0] + d * y[0] - b * x[0] + b * y[0];
      base[1] = pos[1] + d * y[1] - b * x[1] + b * y[1];
      base[2] = pos[2];
      sEdge[0] = -2.0f * b * y[0];
      sEdge[1] = -2.0f * b * y[1];
      sEdge[2] = 0.0f;
      tEdge[0] = 0.0f;
      tEdge[1] = 0.0f;
      tEdge[2] = z;
      break;
    case 8:
      base[0] = pos[0] - d * y[0] + b * x[0] - b * y[0];
      base[1] = pos[1] - d * y[1] + b * x[1] - b * y[1];
      base[2] = pos[2];
      sEdge[0] = 2.0f * b * y[0];
      sEdge[1] = 2.0f * b * y[1];
      sEdge[2] = 0.0f;
      tEdge[0] = 0.0f;
      tEdge[1] = 0.0f;
      tEdge[2] = z;
      break;
    case 9:
      base[0] = pos[0] - d * y[0] - b * x[0] - b * y[0];
      base[1] = pos[1] - d * y[1] - b * x[1] - b * y[1];
      base[2] = pos[2] + z + 2.0f * b;
      sEdge[0] = 2.0f * b * x[0];
      sEdge[1] = 2.0f * b * x[1];
      sEdge[2] = 0.0f;
      tEdge[0] = 2.0f * (d + b) * y[0];
      tEdge[1] = 2.0f * (d + b) * y[1];
      tEdge[2] = 0.0f;
      break;
    case 10:
      base[0] = pos[0] - d * y[0] + b * x[0] + b * y[0];
      base[1] = pos[1] - d * y[1] + b * x[1] + b * y[1];
      base[2] = pos[2] + z;
      sEdge[0] = -2.0f * b * x[0];
      sEdge[1] = -2.0f * b * x[1];
      sEdge[2] = 0.0f;
      tEdge[0] = 2.0f * (d - b) * y[0];
      tEdge[1] = 2.0f * (d - b) * y[1];
      tEdge[2] = 0.0f;
      break;
    case 11:
      base[0] = pos[0] - d * y[0] + b * x[0] - b * y[0];
      base[1] = pos[1] - d * y[1] + b * x[1] - b * y[1];
      base[2] = pos[2] + z;
      sEdge[0] = 2.0f * (d + b) * y[0];
      sEdge[1] = 2.0f * (d + b) * y[1];
      sEdge[2] = 0.0f;
      tEdge[0] = 0.0f;
      tEdge[1] = 0.0f;
      tEdge[2] = 2.0f * b;
      break;
    case 12:
      base[0] = pos[0] + d * y[0] - b * x[0] + b * y[0];
      base[1] = pos[1] + d * y[1] - b * x[1] + b * y[1];
      base[2] = pos[2] + z;
      sEdge[0] = -2.0f * (d + b) * y[0];
      sEdge[1] = -2.0f * (d + b) * y[1];
      sEdge[2] = 0.0f;
      tEdge[0] = 0.0f;
      tEdge[1] = 0.0f;
      tEdge[2] = 2.0f * b;
      break;
    case 13:
      base[0] = pos[0] - d * y[0] + w * x[0] + b * y[0];
      base[1] = pos[1] - d * y[1] + w * x[1] + b * y[1];
      base[2] = pos[2];
      sEdge[0] = 2.0f * (d - b) * y[0];
      sEdge[1] = 2.0f * (d - b) * y[1];
      sEdge[2] = 0.0f;
      tEdge[0] = 0.0f;
      tEdge[1] = 0.0f;
      tEdge[2] = z;
      break;
    case 14:
      base[0] = pos[0] + d * y[0] - w * x[0] - b * y[0];
      base[1] = pos[1] + d * y[1] - w * x[1] - b * y[1];
      base[2] = pos[2];
      sEdge[0] = -2.0f * (d - b) * y[0];
      sEdge[1] = -2.0f * (d - b) * y[1];
      sEdge[2] = 0.0f;
      tEdge[0] = 0.0f;
      tEdge[1] = 0.0f;
      tEdge[2] = z;
      break;
  }
  float u, v, uc, vc;
  if (n >= 1 && n <= 12) {
    u = texCoords[n-1][0][0];
    v = texCoords[n-1][0][1];
    uc = texCoords[n-1][1][0] - u;
    vc = texCoords[n-1][3][1] - v;
  }
  else {
    u = v = 0.0f;
    uc = vc = 1.0f;
  }
  return new QuadWallSceneNode(base, sEdge, tEdge, u, v, uc, vc, lod);
}