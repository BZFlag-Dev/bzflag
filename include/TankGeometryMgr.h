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

/* TankSceneNode:
 *	Encapsulates information for rendering a tank
 */

#ifndef	BZF_TANK_GEOMETRY_MGR_H
#define	BZF_TANK_GEOMETRY_MGR_H

#include "common.h"
#include "SceneNode.h"


extern class TankGeometryMgr TANKGEOMMGR;


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


class TankGeometryMgr {

  public:
    TankGeometryMgr();
    ~TankGeometryMgr();

    // rebuild the display lists
    void rebuildLists();

    // grab a list, make dem graphics
    GLuint getPartList(TankGeometryEnums::TankShadow shadow, 
                       TankGeometryEnums::TankPart part, 
                       TankGeometryEnums::TankSize size,
                       TankGeometryEnums::TankLOD lod) const;
    
    // for doVertex3f(), doNormal3f(), and doTexCoord2f()
    static TankGeometryEnums::TankShadow getShadowMode(); // const
    static const float* currentScaleFactor(); // const
    static const float* getScaleFactor(
                          TankGeometryEnums::TankSize size); // const
    
    
  private:
    // delete the display lists
    void deleteLists();
    
    // setup the scaling factors, and a callback BZDB
    static void setupScales(const std::string& name, void *data);
    
    // for the OpenGLGState callback
    static void initContext(void*);

  private:
    // display lists for drawing each part at different
    // sizes and at different Levels Of Detail
    static GLuint displayLists[TankGeometryEnums::LastTankShadow]
                              [TankGeometryEnums::LastTankLOD]
                              [TankGeometryEnums::LastTankSize]
                              [TankGeometryEnums::LastTankPart];
    
    // scaling factors for the different sizes
    static GLfloat scaleFactors[TankGeometryEnums::LastTankSize][3];
    
    // for currentVertexScale() and currentNormalScale()
    static const float* scaleFactor;
    
    // shadow mode
    static TankGeometryEnums::TankShadow shadowMode;
    
    // functions that make the basic tank parts
    typedef void (*PartFunction)(void);
    static const PartFunction partFunctions[TankGeometryEnums::LastTankLOD]
                                           [TankGeometryEnums::BasicTankParts];
                                           
    // BZDB callback status
    bool callbacksInstalled;
};

inline GLuint TankGeometryMgr::getPartList(TankGeometryEnums::TankShadow shadow, 
                                           TankGeometryEnums::TankPart part, 
                                           TankGeometryEnums::TankSize size,
                                           TankGeometryEnums::TankLOD lod) const
{
  return displayLists[shadow][lod][size][part];
}

inline TankGeometryEnums::TankShadow TankGeometryMgr::getShadowMode()
{
  return shadowMode;
}

inline const float* TankGeometryMgr::getScaleFactor(
                                       TankGeometryEnums::TankSize size)
{
  return scaleFactors[size];
}

inline const float* TankGeometryMgr::currentScaleFactor()
{
  return scaleFactor;
}



namespace TankGeometryUtils {

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

