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

  GLuint getPartList(TankGeometryEnums::TankShadow shadow,
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
  inline void doVertex3f(float x, float y, float z);
  inline void doNormal3f(float x, float y, float z);
  inline void doTexCoord2f(float x, float y);

  //
  // NOTE:  these all return their triangle count
  //

  // lowtank geometry builder
  int buildLowBody (void);
  int buildLowTurret (void);
  int buildLowLCasing (void);
  int buildLowRCasing (void);
  int buildLowBarrel (void);

  // medtank geometry builder
  int buildMedBody (void);
  int buildMedTurret (void);
  int buildMedLCasing (void);
  int buildMedRCasing (void);
  int buildMedBarrel (void);

  // hightank geometry builder
  int buildHighBody (void);
  int buildHighBarrel (void);
  int buildHighTurret (void);
  int buildHighLCasing (void);
  int buildHighRCasing (void);

  // animated geometry builder
  int buildHighLCasingAnim (void);
  int buildHighRCasingAnim (void);
  int buildHighLTread (int divs);
  int buildHighRTread (int divs);
  int buildHighLWheel (int wheel, float angle, int divs);
  int buildHighRWheel (int wheel, float angle, int divs);

  extern const fvec3* currentScaleFactor;
  extern TankGeometryEnums::TankShadow shadowMode;

  bool buildGeoFromObj ( const char* path, int &count );

}



// TankGeometryUtils Functions
// ---------------------------

inline
void TankGeometryUtils::doVertex3f(float x, float y, float z)
{
  const fvec3* scale = currentScaleFactor;
  const fvec3 pos(x * scale->x, y * scale->y, z * scale->z);
  glVertex3fv(pos);
  return;
}


inline
void TankGeometryUtils::doNormal3f(float x, float y, float z)
{
  if (shadowMode == TankGeometryEnums::ShadowOn) {
    return;
  }
  const fvec3* scale = currentScaleFactor;
  fvec3 normal(x / scale->x, y / scale->y, z / scale->z);
  fvec3::normalize(normal);
  glNormal3fv(normal);
  return;
}


inline
void TankGeometryUtils::doTexCoord2f(float x, float y)
{
  if (shadowMode == TankGeometryEnums::ShadowOn) {
    return;
  }
  glTexCoord2f(x, y);
  return;
}


#endif // BZF_TANK_GEOMETRY_MGR_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
