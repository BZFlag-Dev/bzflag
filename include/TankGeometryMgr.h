/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
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
 *  Generates the display lists for TankSceneNodes
 */

#ifndef BZF_TANK_GEOMETRY_MGR_H
#define BZF_TANK_GEOMETRY_MGR_H

#include "common.h"
#include "SceneNode.h"


namespace TankGeometryEnums
{

enum TankShadow
{
    ShadowOff = 0,
    ShadowOn,
    LastTankShadow
};

enum TankLOD
{
    LowTankLOD = 0,
    MedTankLOD,
    HighTankLOD,
    LastTankLOD
};

enum TankSize
{
    Normal = 0,
    Obese,
    Tiny,
    Narrow,
    Thief,
    LastTankSize
};

enum TankPart
{
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


namespace TankGeometryMgr
{

void init();
void kill();
void buildLists();
void deleteLists();

int drawPart(TankGeometryEnums::TankShadow shadow,
             TankGeometryEnums::TankPart part,
             TankGeometryEnums::TankSize size);
void drawLights(bool colorOverride, TankGeometryEnums::TankSize size);
void drawJet();

const float* getScaleFactor(TankGeometryEnums::TankSize size);
}


namespace TankGeometryUtils
{

enum TreadStyle
{
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

//
// NOTE:  these all return their triangle count
//

// hightank geometry builder
void DrawTankSides();
void DrawCentralBody();
void DrawRearExaust2();
void buildHighBarrelHole();
void buildHighBarrelGun();
void buildHighTurret1();
void buildHighTurret5();
void buildHighTurret6();
void buildHighTurret7();
void buildHighTurret13();
void buildHighTurret14();
void buildHighLCasing1();
void buildHighLCasing2();
void buildHighLCasing3();
void buildHighLCasing4();
void buildHighLCasing5();
void buildHighLCasing6();
void buildHighLCasing7();
void buildHighRCasing1();
void buildHighRCasing2();
void buildHighRCasing3();
void buildHighRCasing4();
void buildHighRCasing5();
void buildHighRCasing6();
void buildHighRCasing7();

void buildLCasingR();
void buildRCasingR();
void buildLCasingL();
void buildRCasingL();
// animated geometry builder
void buildHighLCasingAnim1();
void buildHighLCasingAnim2();
void buildHighLCasingAnim3();
void buildHighLCasingAnim4();
void buildHighLCasingAnim5();
void buildHighLCasingAnim6();
void buildHighLCasingAnim7();
void buildHighRCasingAnim1();
void buildHighRCasingAnim2();
void buildHighRCasingAnim3();
void buildHighRCasingAnim4();
void buildHighRCasingAnim5();
void buildHighRCasingAnim6();
void buildHighRCasingAnim7();
void buildTread1(bool right);
void buildTread2(bool right);
void buildTread3(bool right);
void buildWheel1(int wheel, bool right);
void buildWheel2(int wheel, bool right);
void buildWheel3(int wheel, bool right);
}


#endif // BZF_TANK_GEOMETRY_MGR_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
