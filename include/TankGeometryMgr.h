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

/* TankGeometryMgr:
 *	Generates the display lists for TankSceneNodes
 */

#ifndef	BZF_TANK_GEOMETRY_MGR_H
#define	BZF_TANK_GEOMETRY_MGR_H

#include "common.h"
#include "SceneNode.h"
#include "vectors.h"


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

  unsigned int getPartList(TankGeometryEnums::TankShadow shadow,
		           TankGeometryEnums::TankPart part,
		           TankGeometryEnums::TankSize size,
		           TankGeometryEnums::TankLOD lod);

  int getPartTriangleCount(TankGeometryEnums::TankShadow shadow,
			   TankGeometryEnums::TankPart part,
			   TankGeometryEnums::TankSize size,
			   TankGeometryEnums::TankLOD lod);

  const fvec3& getScaleFactor(TankGeometryEnums::TankSize size);
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
  void doVertex3f(float x, float y, float z);
  void doVertex(const fvec3&);
  void doNormal3f(float x, float y, float z);
  void doNormal(const fvec3&);
  void doTexCoord2f(float x, float y);
  void doTexCoord(const fvec2&);

  //
  // NOTE:  these all return their triangle count
  //

  // lowtank geometry builder
  int buildLowBody();
  int buildLowTurret();
  int buildLowLCasing();
  int buildLowRCasing();
  int buildLowBarrel();

  // medtank geometry builder
  int buildMedBody();
  int buildMedTurret();
  int buildMedLCasing();
  int buildMedRCasing();
  int buildMedBarrel();

  // hightank geometry builder
  int buildHighBody();
  int buildHighBarrel();
  int buildHighTurret();
  int buildHighLCasing();
  int buildHighRCasing();

  // animated geometry builder
  int buildHighLCasingAnim();
  int buildHighRCasingAnim();
  int buildHighLTread(int divs);
  int buildHighRTread(int divs);
  int buildHighLWheel(int wheel, float angle, int divs);
  int buildHighRWheel(int wheel, float angle, int divs);

  extern const fvec3* currentScaleFactor;
  extern TankGeometryEnums::TankShadow shadowMode;

  bool buildGeoFromObj(const char* path, int &count);

}


#endif // BZF_TANK_GEOMETRY_MGR_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
