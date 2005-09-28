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

/* TankGeometryMgr:
 *	Generates the display lists for TankSceneNodes
 */

#ifndef	BZF_TANK_GEOMETRY_MGR_H
#define	BZF_TANK_GEOMETRY_MGR_H

#include "common.h"
#include "SceneNode.h"


namespace TankGeometryEnums {

  enum TankShadow {
    ShadowOff = 0,
    ShadowOn,
    LastTankShadow
  };

  enum TankLOD {
    LowTankLOD = 0,
    MedTankLOD,
    HighTankLOD,
    LastTankLOD
  };

  enum TankSize {
    Normal = 0,
    Obese,
    Tiny,
    Narrow,
    Thief,
    LastTankSize
  };

  enum TankPart {
    Body = 0,
    Barrel,
    Turret,
    LeftCasing,
    RightCasing,

    LeftTread, // animated parts
    RightTread,
    LeftWheel0,
    LeftWheel1,
    LeftWheel2,
    LeftWheel3,
    RightWheel0,
    RightWheel1,
    RightWheel2,
    RightWheel3,

    LastTankPart,
    BasicTankParts = LeftTread,
    LowTankParts = LeftTread,
    MedTankParts = LeftTread,
    HighTankParts = LastTankPart
  };
}


namespace TankGeometryMgr {

  void init();
  void kill();
  void buildLists();
  void deleteLists();

  GLuint getPartList(TankGeometryEnums::TankShadow shadow,
		     TankGeometryEnums::TankPart part,
		     TankGeometryEnums::TankSize size,
		     TankGeometryEnums::TankLOD lod);

  const float* getScaleFactor(TankGeometryEnums::TankSize size);
}


namespace TankGeometryUtils {

  enum TreadStyle {
    Covered = 0,
    Exposed = 1
  };
  void setTreadStyle(int style);

  // degrees / meter
  float getWheelScale();
  // texcoords / meter
  float getTreadScale();
  // texcoords
  float getTreadTexLen();

  // help to scale vertices and normals
  void doVertex3f(GLfloat x, GLfloat y, GLfloat z);
  void doNormal3f(GLfloat x, GLfloat y, GLfloat z);
  void doTexCoord2f(GLfloat x, GLfloat y);

  // lowtank geometry builder
  void buildLowBody (void);
  void buildLowTurret (void);
  void buildLowLCasing (void);
  void buildLowRCasing (void);
  void buildLowBarrel (void);

  // medtank geometry builder
  void buildMedBody (void);
  void buildMedTurret (void);
  void buildMedLCasing (void);
  void buildMedRCasing (void);
  void buildMedBarrel (void);

  // hightank geometry builder
  void buildHighBody (void);
  void buildHighBarrel (void);
  void buildHighTurret (void);
  void buildHighLCasing (void);
  void buildHighRCasing (void);

  // animated geometry builder
  void buildHighLCasingAnim (void);
  void buildHighRCasingAnim (void);
  void buildHighLTread (int divs);
  void buildHighRTread (int divs);
  void buildHighLWheel (int wheel, float angle, int divs);
  void buildHighRWheel (int wheel, float angle, int divs);
}


#endif // BZF_TANK_GEOMETRY_MGR_H


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

