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

// bzflag common headers
#include "common.h"
#include "global.h"

// interface header
#include "TankGeometryMgr.h"

// system headers
#include <stdlib.h>
#include <math.h>
#include <string>

// common implementation headers
#include "SceneRenderer.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "OpenGLGState.h"
#include "VBO_Handler.h"

// use the namespaces
using namespace TankGeometryMgr;
using namespace TankGeometryEnums;
using namespace TankGeometryUtils;


// Local Variables
// ---------------

// the scaling factors
static GLfloat scaleFactors[LastTankSize][3] =
{
    {1.0f, 1.0f, 1.0f},   // Normal
    {1.0f, 1.0f, 1.0f},   // Obese
    {1.0f, 1.0f, 1.0f},   // Tiny
    {1.0f, 0.001f, 1.0f}, // Narrow
    {1.0f, 1.0f, 1.0f}    // Thief
};
// the current scaling factors
static const float* currentScaleFactor = scaleFactors[Normal];

// the current shadow mode (used to remove glNormal3f and glTexcoord2f calls)
static TankShadow shadowMode = ShadowOn;

// arrays of functions to avoid large switch statements
typedef void (*elemFunction)(void);
static const elemFunction turretFunctions[6] =
{
    buildHighTurret1,
    buildHighTurret5,
    buildHighTurret6,
    buildHighTurret7,
    buildHighTurret13,
    buildHighTurret14
};

static const elemFunction lCasingsFunctions[7] =
{
    buildHighLCasing1,
    buildHighLCasing2,
    buildHighLCasing3,
    buildHighLCasing4,
    buildHighLCasing5,
    buildHighLCasing6,
    buildHighLCasing7
};

static const elemFunction rCasingsFunctions[7] =
{
    buildHighRCasing1,
    buildHighRCasing2,
    buildHighRCasing3,
    buildHighRCasing4,
    buildHighRCasing5,
    buildHighRCasing6,
    buildHighRCasing7
};

static const elemFunction lCasingsAFunctions[7] =
{
    buildHighLCasingAnim1,
    buildHighLCasingAnim2,
    buildHighLCasingAnim3,
    buildHighLCasingAnim4,
    buildHighLCasingAnim5,
    buildHighLCasingAnim6,
    buildHighLCasingAnim7
};

static const elemFunction rCasingsAFunctions[7] =
{
    buildHighRCasingAnim1,
    buildHighRCasingAnim2,
    buildHighRCasingAnim3,
    buildHighRCasingAnim4,
    buildHighRCasingAnim5,
    buildHighRCasingAnim6,
    buildHighRCasingAnim7
};

typedef void (*treadFunction)(bool);
static const treadFunction Tread[4] =
{
    buildTread1,
    buildTread2,
    buildTread3
};

typedef void (*wheelFunction)(int, bool);
static const wheelFunction wheel[3] =
{
    buildWheel1,
    buildWheel2,
    buildWheel3
};


// Local Function Prototypes
// -------------------------

static void setupScales();
static void freeContext(void *data);
static void initContext(void *data);
static void bzdbCallback(const std::string& str, void *data);


class TankVBOHandler : public VBOclient
{
public:
    TankVBOHandler();
    virtual ~TankVBOHandler();

    void initVBO();
    void rebuildVBO(bool reserve);

    int drawBody(bool shadow, int size);
    int drawBarrel(bool shadow, int size);
    int drawTurret(bool shadow, int size);
    int drawLCase(bool shadow, int size);
    int drawRCase(bool shadow, int size);
    int drawALCase(bool shadow, int size);
    int drawARCase(bool shadow, int size);
    int drawTread(bool shadow, int size, int right);
    int drawWheel(bool shadow, int size, int number, int right);
    void drawLights(bool colorOverride, int size);
    void drawJet();
private:
    int barrelHoleIndex[LastTankSize];
    int barrelHoleCount;
    int barrelGunIndex[LastTankSize];
    int barrelGunCount;
    int bodySidesIndex[LastTankSize];
    int bodySidesCount;
    int bodyCentralIndex[LastTankSize];
    int bodyCentralCount;
    int exhaust2Index[LastTankSize];
    int exhaust2Count;
    int leftExhaust1Index[LastTankSize];
    int leftExhaust1Count;
    int turretIndex[LastTankSize][6];
    int turretCount[6];
    int lcaseIndex[LastTankSize][7];
    int lcaseCount[7];
    int rcaseIndex[LastTankSize][7];
    int rcaseCount[7];
    int lcaseAIndex[LastTankSize][7];
    int lcaseACount[7];
    int rcaseAIndex[LastTankSize][7];
    int rcaseACount[7];
    int LCasingRIndex[LastTankSize];
    int LCasingRCount;
    int LCasingLIndex[LastTankSize];
    int LCasingLCount;
    int RCasingRIndex[LastTankSize];
    int RCasingRCount;
    int RCasingLIndex[LastTankSize];
    int RCasingLCount;
    int TreadIndex[LastTankSize][4][2];
    int TreadCount[4];
    int wheelIndex[LastTankSize][4][3][2];
    int wheelCount[3];
    int lightsIndex;
    int jetIndex;
};

static TankVBOHandler *tankVBO;

/****************************************************************************/

// TankGeometryMgr Functions
// -------------------------


void TankGeometryMgr::init()
{
    // install the BZDB callbacks
    // This MUST be done after BZDB has been initialized in main()
    BZDB.addCallback (StateDatabase::BZDB_OBESEFACTOR, bzdbCallback, NULL);
    BZDB.addCallback (StateDatabase::BZDB_TINYFACTOR, bzdbCallback, NULL);
    BZDB.addCallback (StateDatabase::BZDB_THIEFTINYFACTOR, bzdbCallback, NULL);
    BZDB.addCallback ("animatedTreads", bzdbCallback, NULL);

    // install the context initializer
    OpenGLGState::registerContextInitializer (freeContext, initContext, NULL);

    // setup the scaleFactors
    setupScales();

    tankVBO = new TankVBOHandler;
    return;
}


void TankGeometryMgr::kill()
{
    delete tankVBO;

    // remove the BZDB callbacks
    BZDB.removeCallback (StateDatabase::BZDB_OBESEFACTOR, bzdbCallback, NULL);
    BZDB.removeCallback (StateDatabase::BZDB_TINYFACTOR, bzdbCallback, NULL);
    BZDB.removeCallback (StateDatabase::BZDB_THIEFTINYFACTOR, bzdbCallback, NULL);
    BZDB.removeCallback ("animatedTreads", bzdbCallback, NULL);

    // remove the context initializer callback
    OpenGLGState::unregisterContextInitializer(freeContext, initContext, NULL);

    return;
}


void TankGeometryMgr::deleteLists()
{
    return;
}

static int treadStyle = TankGeometryUtils::Covered;

void TankGeometryMgr::buildLists()
{
    // setup the tread style
    int style = BZDB.evalInt("treadStyle");
    if (style == TankGeometryUtils::Exposed)
        treadStyle = TankGeometryUtils::Exposed;
    else
        treadStyle = TankGeometryUtils::Covered;

    setTreadStyle(style);

    // setup the scale factors
    setupScales();
    currentScaleFactor = scaleFactors[Normal];
    tankVBO->rebuildVBO(false);
    return;
}


void TankGeometryMgr::drawLights(bool colorOverride, TankGeometryEnums::TankSize size)
{
    tankVBO->drawLights(colorOverride, size);
}

void TankGeometryMgr::drawJet()
{
    tankVBO->drawJet();
}

int TankGeometryMgr::drawPart(TankGeometryEnums::TankShadow shadow,
                              TankGeometryEnums::TankPart part,
                              TankGeometryEnums::TankSize size)
{
    const bool animated = BZDBCache::animatedTreads;
    if (part == Body)
        return tankVBO->drawBody(shadow, size);
    else if (part == Barrel)
        return tankVBO->drawBarrel(shadow, size);
    else if (part == Turret)
        return tankVBO->drawTurret(shadow, size);
    else if (!animated)
    {
        if (part == LeftCasing)
            return tankVBO->drawLCase(shadow, size);
        else if (part == RightCasing)
            return tankVBO->drawRCase(shadow, size);
    }
    else if (part == LeftCasing)
        return tankVBO->drawALCase(shadow, size);
    else if (part == RightCasing)
        return tankVBO->drawARCase(shadow, size);
    else if ((part == LeftTread) || (part == RightTread))
        return tankVBO->drawTread(shadow, size, part - LeftTread);
    else if ((part >= LeftWheel0) && (part <= RightWheel3))
    {
        int myWheel = part - LeftWheel0;
        int right   = 0;
        if (myWheel > 3)
        {
            myWheel -= 4;
            right    = 1;
        }
        return tankVBO->drawWheel(shadow, size, myWheel, right);
    }
    return 0;
}


const float* TankGeometryMgr::getScaleFactor(TankSize size)
{
    return scaleFactors[size];
}


/****************************************************************************/

// Local Functions
// ---------------


static void bzdbCallback(const std::string& UNUSED(name), void * UNUSED(data))
{
    deleteLists();
    buildLists();
    return;
}


static void freeContext(void * UNUSED(data))
{
    // delete all of the lists
    deleteLists();
    return;
}


static void initContext(void * UNUSED(data))
{
    buildLists();
    return;
}


static void setupScales()
{
    float scale;

    scaleFactors[Normal][0] = BZDBCache::tankLength;
    scale = (float)atof(BZDB.getDefault(StateDatabase::BZDB_TANKLENGTH).c_str());
    scaleFactors[Normal][0] /= scale;

    scaleFactors[Normal][1] = BZDBCache::tankWidth;
    scale = (float)atof(BZDB.getDefault(StateDatabase::BZDB_TANKWIDTH).c_str());
    scaleFactors[Normal][1] /= scale;

    scaleFactors[Normal][2] = BZDBCache::tankHeight;
    scale = (float)atof(BZDB.getDefault(StateDatabase::BZDB_TANKHEIGHT).c_str());
    scaleFactors[Normal][2] /= scale;

    scale = BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
    scaleFactors[Obese][0] = scale * scaleFactors[Normal][0];
    scaleFactors[Obese][1] = scale * scaleFactors[Normal][1];
    scaleFactors[Obese][2] = scaleFactors[Normal][2];

    scale = BZDB.eval(StateDatabase::BZDB_TINYFACTOR);
    scaleFactors[Tiny][0] = scale * scaleFactors[Normal][0];
    scaleFactors[Tiny][1] = scale * scaleFactors[Normal][1];
    scaleFactors[Tiny][2] = scaleFactors[Normal][2];

    scale = BZDB.eval(StateDatabase::BZDB_THIEFTINYFACTOR);
    scaleFactors[Thief][0] = scale * scaleFactors[Normal][0];
    scaleFactors[Thief][1] = scale * scaleFactors[Normal][1];
    scaleFactors[Thief][2] = scaleFactors[Normal][2];

    scaleFactors[Narrow][0] = scaleFactors[Normal][0];
    scaleFactors[Narrow][1] = 0.001f;
    scaleFactors[Narrow][2] = scaleFactors[Normal][2];

    return;
}


/****************************************************************************/

// TankGeometryUtils Functions
// ---------------------------

static int  loadIndex;

glm::vec3 tankVertex[146];
glm::vec3 tankNormal[146];
glm::vec2 tankTextur[146];

void TankGeometryUtils::doVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    const float* scale = currentScaleFactor;
    x = x * scale[0];
    y = y * scale[1];
    z = z * scale[2];
    tankVertex[loadIndex++] = glm::vec3(x, y, z);
    return;
}


void TankGeometryUtils::doNormal3f(GLfloat x, GLfloat y, GLfloat z)
{
    if (shadowMode == ShadowOn)
        return;
    const float* scale = currentScaleFactor;
    GLfloat sx = x * scale[0];
    GLfloat sy = y * scale[1];
    GLfloat sz = z * scale[2];
    const GLfloat d = sqrtf ((sx * sx) + (sy * sy) + (sz * sz));
    if (d > 1.0e-5f)
    {
        x *= scale[0] / d;
        y *= scale[1] / d;
        z *= scale[2] / d;
    }
    tankNormal[loadIndex] = glm::vec3(x, y, z);
    return;
}


void TankGeometryUtils::doTexCoord2f(GLfloat x, GLfloat y)
{
    if (shadowMode == ShadowOn)
        return;
    tankTextur[loadIndex] = glm::vec2(x, y);
    return;
}


TankVBOHandler::TankVBOHandler()
{
    for (int size = 0; size < LastTankSize; size++)
    {
        barrelHoleIndex[size] = -1;
        barrelGunIndex[size] = -1;
        bodySidesIndex[size] = -1;
        bodyCentralIndex[size] = -1;
        exhaust2Index[size] = -1;
        for (int i = 0; i < 6; i++)
            turretIndex[size][i] = -1;
        for (int i = 0; i < 7; i++)
        {
            lcaseIndex[size][i] = -1;
            rcaseIndex[size][i] = -1;
        }
        LCasingRIndex[size] = -1;
        LCasingLIndex[size] = -1;
        RCasingRIndex[size] = -1;
        RCasingLIndex[size] = -1;
        for (int i = 0; i < 7; i++)
        {
            lcaseAIndex[size][i] = -1;
            rcaseAIndex[size][i] = -1;
        }
        for (int i = 0; i < 3; i++)
            for (int k = 0; k < 2; k++)
            {
                TreadIndex[size][i][k] = -1;
                for (int j = 0; j < 4; j++)
                    wheelIndex[size][j][i][k] = -1;
            }
    }
    lightsIndex = -1;
    jetIndex = -1;
    vboManager.registerClient(this);
}


TankVBOHandler::~TankVBOHandler()
{
    for (int size = 0; size < LastTankSize; size++)
    {
        vboV.vboFree(barrelHoleIndex[size]);
        vboVN.vboFree(barrelGunIndex[size]);
        vboVTN.vboFree(bodySidesIndex[size]);
        vboVTN.vboFree(bodyCentralIndex[size]);
        vboVT.vboFree(exhaust2Index[size]);
        for (int i = 0; i < 6; i++)
            if ((i == 1) || (i == 2))
                vboVT.vboFree(turretIndex[size][i]);
            else
                vboVTN.vboFree(turretIndex[size][i]);
        for (int i = 0; i < 7; i++)
            if (i)
            {
                vboVT.vboFree(lcaseIndex[size][i]);
                vboVT.vboFree(rcaseIndex[size][i]);
            }
            else
            {
                vboVTN.vboFree(lcaseIndex[size][i]);
                vboVTN.vboFree(rcaseIndex[size][i]);
            }
        vboVT.vboFree(LCasingRIndex[size]);
        vboVT.vboFree(LCasingLIndex[size]);
        vboVT.vboFree(RCasingRIndex[size]);
        vboVT.vboFree(RCasingLIndex[size]);
        for (int i = 0; i < 7; i++)
            if (i)
            {
                vboVT.vboFree(lcaseAIndex[size][i]);
                vboVT.vboFree(rcaseAIndex[size][i]);
            }
            else
            {
                vboVTN.vboFree(lcaseAIndex[size][i]);
                vboVTN.vboFree(rcaseAIndex[size][i]);
            }
        for (int i = 0; i < 3; i++)
            for (int k = 0; k < 2; k++)
            {
                if (i)
                    vboVT.vboFree(TreadIndex[size][i][k]);
                else
                    vboVTN.vboFree(TreadIndex[size][i][k]);
                for (int j = 0; j < 4; j++)
                    if (i)
                        vboVT.vboFree(wheelIndex[size][j][i][k]);
                    else
                        vboVTN.vboFree(wheelIndex[size][j][i][k]);
            }
    }
    vboVC.vboFree(lightsIndex);
    vboVT.vboFree(jetIndex);
    vboManager.unregisterClient(this);
}


void TankVBOHandler::initVBO()
{
    rebuildVBO(true);
}

void TankVBOHandler::rebuildVBO(bool reserve)
{
    int indexVBO;
    shadowMode = ShadowOff;
    for (int size = 0; size < LastTankSize; size++)
    {
        currentScaleFactor = scaleFactors[size];

        loadIndex = 0;
        buildHighBarrelHole();
        barrelHoleCount = loadIndex;
        if (reserve)
        {
            indexVBO = vboV.vboAlloc(loadIndex);
            barrelHoleIndex[size] = indexVBO;
        }
        else
            indexVBO = barrelHoleIndex[size];
        vboV.vertexData(indexVBO, loadIndex, tankVertex);

        loadIndex = 0;
        buildHighBarrelGun();
        barrelGunCount = loadIndex;
        if (reserve)
        {
            indexVBO = vboVN.vboAlloc(loadIndex);
            barrelGunIndex[size] = indexVBO;
        }
        else
            indexVBO = barrelGunIndex[size];
        vboVN.vertexData(indexVBO, loadIndex, tankVertex);
        vboVN.normalData(indexVBO, loadIndex, tankNormal);

        loadIndex = 0;
        DrawTankSides();
        bodySidesCount = loadIndex;
        if (reserve)
        {
            indexVBO = vboVTN.vboAlloc(loadIndex);
            bodySidesIndex[size] = indexVBO;
        }
        else
            indexVBO = bodySidesIndex[size];
        vboVTN.vertexData(indexVBO,  loadIndex, tankVertex);
        vboVTN.normalData(indexVBO,  loadIndex, tankNormal);
        vboVTN.textureData(indexVBO, loadIndex, tankTextur);

        loadIndex = 0;
        DrawCentralBody();
        bodyCentralCount = loadIndex;
        if (reserve)
        {
            indexVBO = vboVTN.vboAlloc(loadIndex);
            bodyCentralIndex[size] = indexVBO;
        }
        else
            indexVBO = bodyCentralIndex[size];
        vboVTN.vertexData(indexVBO,  loadIndex, tankVertex);
        vboVTN.normalData(indexVBO,  loadIndex, tankNormal);
        vboVTN.textureData(indexVBO, loadIndex, tankTextur);

        loadIndex = 0;
        DrawRearExaust2();
        exhaust2Count = loadIndex;
        if (reserve)
        {
            indexVBO = vboVT.vboAlloc(loadIndex);
            exhaust2Index[size] = indexVBO;
        }
        else
            indexVBO = exhaust2Index[size];
        vboVT.vertexData(indexVBO,  loadIndex, tankVertex);
        vboVT.textureData(indexVBO, loadIndex, tankTextur);

        for (int i = 0; i < 6; i++)
        {
            loadIndex = 0;
            turretFunctions[i]();
            if (reserve)
            {
                if ((i == 1) || (i == 2))
                    indexVBO = vboVT.vboAlloc(loadIndex);
                else
                    indexVBO = vboVTN.vboAlloc(loadIndex);
                turretIndex[size][i] = indexVBO;
                turretCount[i]       = loadIndex;
            }
            else
                indexVBO = turretIndex[size][i];
            if ((i == 1) || (i == 2))
            {
                vboVT.vertexData(indexVBO,  loadIndex, tankVertex);
                vboVT.textureData(indexVBO, loadIndex, tankTextur);
            }
            else
            {
                vboVTN.vertexData(indexVBO,  loadIndex, tankVertex);
                vboVTN.normalData(indexVBO,  loadIndex, tankNormal);
                vboVTN.textureData(indexVBO, loadIndex, tankTextur);
            }
        }
        for (int i = 0; i < 7; i++)
        {
            loadIndex = 0;
            lCasingsFunctions[i]();
            if (reserve)
            {
                if (i)
                    indexVBO = vboVT.vboAlloc(loadIndex);
                else
                    indexVBO = vboVTN.vboAlloc(loadIndex);
                lcaseIndex[size][i] = indexVBO;
                lcaseCount[i]       = loadIndex;
            }
            else
                indexVBO = lcaseIndex[size][i];
            if (i)
            {
                vboVT.vertexData(indexVBO,  loadIndex, tankVertex);
                vboVT.textureData(indexVBO, loadIndex, tankTextur);
            }
            else
            {
                vboVTN.vertexData(indexVBO,  loadIndex, tankVertex);
                vboVTN.normalData(indexVBO,  loadIndex, tankNormal);
                vboVTN.textureData(indexVBO, loadIndex, tankTextur);
            }
        }
        for (int i = 0; i < 7; i++)
        {
            loadIndex = 0;
            rCasingsFunctions[i]();
            if (reserve)
            {
                if (i)
                    indexVBO = vboVT.vboAlloc(loadIndex);
                else
                    indexVBO = vboVTN.vboAlloc(loadIndex);
                rcaseIndex[size][i] = indexVBO;
                rcaseCount[i]       = loadIndex;
            }
            else
                indexVBO = rcaseIndex[size][i];
            if (i)
            {
                vboVT.vertexData(indexVBO,  loadIndex, tankVertex);
                vboVT.textureData(indexVBO, loadIndex, tankTextur);
            }
            else
            {
                vboVTN.vertexData(indexVBO,  loadIndex, tankVertex);
                vboVTN.normalData(indexVBO,  loadIndex, tankNormal);
                vboVTN.textureData(indexVBO, loadIndex, tankTextur);
            }
        }
        loadIndex = 0;
        buildLCasingR();
        if (reserve)
        {
            indexVBO = vboVT.vboAlloc(loadIndex);
            LCasingRIndex[size] = indexVBO;
            LCasingRCount       = loadIndex;
        }
        else
            indexVBO = LCasingRIndex[size];
        vboVT.vertexData(indexVBO,  loadIndex, tankVertex);
        vboVT.textureData(indexVBO, loadIndex, tankTextur);
        loadIndex = 0;
        buildLCasingL();
        if (reserve)
        {
            indexVBO = vboVT.vboAlloc(loadIndex);
            LCasingLIndex[size] = indexVBO;
            LCasingLCount       = loadIndex;
        }
        else
            indexVBO = LCasingLIndex[size];
        vboVT.vertexData(indexVBO,  loadIndex, tankVertex);
        vboVT.textureData(indexVBO, loadIndex, tankTextur);
        loadIndex = 0;
        buildRCasingR();
        if (reserve)
        {
            indexVBO = vboVT.vboAlloc(loadIndex);
            RCasingRIndex[size] = indexVBO;
            RCasingRCount       = loadIndex;
        }
        else
            indexVBO = RCasingRIndex[size];
        vboVT.vertexData(indexVBO,  loadIndex, tankVertex);
        vboVT.textureData(indexVBO, loadIndex, tankTextur);
        loadIndex = 0;
        buildRCasingL();
        if (reserve)
        {
            indexVBO = vboVT.vboAlloc(loadIndex);
            RCasingLIndex[size] = indexVBO;
            RCasingLCount       = loadIndex;
        }
        else
            indexVBO = RCasingLIndex[size];
        vboVT.vertexData(indexVBO,  loadIndex, tankVertex);
        vboVT.textureData(indexVBO, loadIndex, tankTextur);
        for (int i = 0; i < 7; i++)
        {
            loadIndex = 0;
            lCasingsAFunctions[i]();
            if (reserve)
            {
                if (i)
                    indexVBO = vboVT.vboAlloc(loadIndex);
                else
                    indexVBO = vboVTN.vboAlloc(loadIndex);
                lcaseAIndex[size][i] = indexVBO;
                lcaseACount[i]       = loadIndex;
            }
            else
                indexVBO = lcaseAIndex[size][i];
            if (i)
            {
                vboVT.vertexData(indexVBO,  loadIndex, tankVertex);
                vboVT.textureData(indexVBO, loadIndex, tankTextur);
            }
            else
            {
                vboVTN.vertexData(indexVBO,  loadIndex, tankVertex);
                vboVTN.normalData(indexVBO,  loadIndex, tankNormal);
                vboVTN.textureData(indexVBO, loadIndex, tankTextur);
            }
        }
        for (int i = 0; i < 7; i++)
        {
            loadIndex = 0;
            rCasingsAFunctions[i]();
            if (reserve)
            {
                if (i)
                    indexVBO = vboVT.vboAlloc(loadIndex);
                else
                    indexVBO = vboVTN.vboAlloc(loadIndex);
                rcaseAIndex[size][i] = indexVBO;
                rcaseACount[i]       = loadIndex;
            }
            else
                indexVBO = rcaseAIndex[size][i];
            if (i)
            {
                vboVT.vertexData(indexVBO,  loadIndex, tankVertex);
                vboVT.textureData(indexVBO, loadIndex, tankTextur);
            }
            else
            {
                vboVTN.vertexData(indexVBO,  loadIndex, tankVertex);
                vboVTN.normalData(indexVBO,  loadIndex, tankNormal);
                vboVTN.textureData(indexVBO, loadIndex, tankTextur);
            }
        }
        for (int i = 0; i < 3; i++)
        {
            loadIndex = 0;
            Tread[i](false);
            if (reserve)
            {
                if (i)
                    indexVBO = vboVT.vboAlloc(loadIndex);
                else
                    indexVBO = vboVTN.vboAlloc(loadIndex);
                TreadIndex[size][i][0] = indexVBO;
                TreadCount[i]          = loadIndex;
            }
            else
                indexVBO = TreadIndex[size][i][0];
            if (i)
            {
                vboVT.vertexData(indexVBO,  loadIndex, tankVertex);
                vboVT.textureData(indexVBO, loadIndex, tankTextur);
            }
            else
            {
                vboVTN.vertexData(indexVBO,  loadIndex, tankVertex);
                vboVTN.normalData(indexVBO,  loadIndex, tankNormal);
                vboVTN.textureData(indexVBO, loadIndex, tankTextur);
            }
        }
        for (int i = 0; i < 3; i++)
        {
            loadIndex = 0;
            Tread[i](true);
            if (reserve)
            {
                if (i)
                    indexVBO = vboVT.vboAlloc(loadIndex);
                else
                    indexVBO = vboVTN.vboAlloc(loadIndex);
                TreadIndex[size][i][1] = indexVBO;
                TreadCount[i]          = loadIndex;
            }
            else
                indexVBO = TreadIndex[size][i][1];
            if (i)
            {
                vboVT.vertexData(indexVBO,  loadIndex, tankVertex);
                vboVT.textureData(indexVBO, loadIndex, tankTextur);
            }
            else
            {
                vboVTN.vertexData(indexVBO,  loadIndex, tankVertex);
                vboVTN.normalData(indexVBO,  loadIndex, tankNormal);
                vboVTN.textureData(indexVBO, loadIndex, tankTextur);
            }
        }
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 4; j++)
            {
                loadIndex = 0;
                wheel[i](j, false);
                if (reserve)
                {
                    if (i)
                        indexVBO = vboVT.vboAlloc(loadIndex);
                    else
                        indexVBO = vboVTN.vboAlloc(loadIndex);
                    wheelIndex[size][j][i][0] = indexVBO;
                    wheelCount[i]             = loadIndex;
                }
                else
                    indexVBO = wheelIndex[size][j][i][0];
                if (i)
                {
                    vboVT.vertexData(indexVBO,  loadIndex, tankVertex);
                    vboVT.textureData(indexVBO, loadIndex, tankTextur);
                }
                else
                {
                    vboVTN.vertexData(indexVBO,  loadIndex, tankVertex);
                    vboVTN.normalData(indexVBO,  loadIndex, tankNormal);
                    vboVTN.textureData(indexVBO, loadIndex, tankTextur);
                }
            }
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 4; j++)
            {
                loadIndex = 0;
                wheel[i](j, true);
                if (reserve)
                {
                    if (i)
                        indexVBO = vboVT.vboAlloc(loadIndex);
                    else
                        indexVBO = vboVTN.vboAlloc(loadIndex);
                    wheelIndex[size][j][i][1] = indexVBO;
                    wheelCount[i]             = loadIndex;
                }
                else
                    indexVBO = wheelIndex[size][j][i][1];
                if (i)
                {
                    vboVT.vertexData(indexVBO,  loadIndex, tankVertex);
                    vboVT.textureData(indexVBO, loadIndex, tankTextur);
                }
                else
                {
                    vboVTN.vertexData(indexVBO,  loadIndex, tankVertex);
                    vboVTN.normalData(indexVBO,  loadIndex, tankNormal);
                    vboVTN.textureData(indexVBO, loadIndex, tankTextur);
                }
            }
    }

    static const glm::vec4 colorLights[3] =
    {
        { 1.0f, 1.0f, 1.0f, 1.0f },
        { 1.0f, 0.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f, 1.0f }
    };
    static const glm::vec3 positionLights[3] =
    {
        { -1.53f,  0.00f, 2.1f },
        {  0.10f,  0.75f, 2.1f },
        {  0.10f, -0.75f, 2.1f }
    };
    if (reserve)
    {
        indexVBO    = vboVC.vboAlloc(3);
        lightsIndex = indexVBO;
    }
    else
        indexVBO = lightsIndex;
    vboVC.colorData(indexVBO, 3, colorLights);
    vboVC.vertexData(indexVBO, 3, positionLights);

    static const glm::vec3 jetVertex[3] =
    {
        {+0.3f,  0.0f, 0.0f},
        {-0.3f,  0.0f, 0.0f},
        { 0.0f, -1.0f, 0.0f},
    };
    static const glm::vec2 jetTexture[3] =
    {
        {0.0f, 1.0f},
        {1.0f, 1.0f},
        {0.5f, 0.0f}
    };

    if (reserve)
    {
        indexVBO    = vboVT.vboAlloc(3);
        jetIndex = indexVBO;
    }
    else
        indexVBO = jetIndex;
    vboVT.textureData(indexVBO, 3, jetTexture);
    vboVT.vertexData(indexVBO, 3, jetVertex);
}


int TankVBOHandler::drawBody(bool shadow, int size)
{
    int count = 0;
    glShadeModel(GL_FLAT);
    if (shadow)
        vboVTN.enableArrays(false, false, false);
    else
        vboVTN.enableArrays();
    glDrawArrays(GL_TRIANGLES, bodySidesIndex[size], bodySidesCount);
    count += 20;
    glDrawArrays(GL_TRIANGLE_STRIP, bodyCentralIndex[size], bodyCentralCount);
    count += 40;
    if (shadow)
        vboVT.enableArrays(false, false, false);
    else
    {
        glNormal3f(-1.000000f, 0.000000f, 0.000000f);
        vboVT.enableArrays();
    }
    glDrawArrays(GL_TRIANGLE_STRIP, exhaust2Index[size], exhaust2Count);
    count += 4;
    return count;
}


int TankVBOHandler::drawBarrel(bool shadow, int size)
{
    int count = 0;

    glShadeModel(GL_FLAT);
    if (!shadow)
        glNormal3f(1.000000f, 0.000000f, 0.000000f);
    vboV.enableArrays();
    glDrawArrays(GL_TRIANGLE_FAN, barrelHoleIndex[size], barrelHoleCount);
    count += 6;

    glShadeModel(GL_SMOOTH);
    if (shadow)
        vboVN.enableArrays(false, false, false);
    else
        vboVN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, barrelGunIndex[size], barrelGunCount);
    count += 16;
    return count;
}


int TankVBOHandler::drawTurret(bool shadow, int size)
{
    int count = 0;

    glShadeModel(GL_FLAT);
    if (shadow)
        vboVTN.enableArrays(false, false, false);
    else
        vboVTN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, turretIndex[size][0], turretCount[0]);
    count += 22;
    if (shadow)
        vboVT.enableArrays(false, false, false);
    else
        vboVT.enableArrays();
    glNormal3f(-1.000000f, 0.000000f, 0.000000f);
    glDrawArrays(GL_TRIANGLE_STRIP, turretIndex[size][1], turretCount[1]);
    count += 2;
    glNormal3f(0.000000f, 0.000000f, 1.000000f);
    glDrawArrays(GL_TRIANGLE_STRIP, turretIndex[size][2], turretCount[2]);
    count += 10;

    glShadeModel(GL_SMOOTH);
    if (shadow)
        vboVTN.enableArrays(false, false, false);
    else
        vboVTN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, turretIndex[size][3], turretCount[3]);
    count += 60;
    glDrawArrays(GL_TRIANGLE_FAN, turretIndex[size][4], turretCount[4]);
    count += 5;
    glDrawArrays(GL_TRIANGLE_FAN, turretIndex[size][5], turretCount[5]);
    count += 5;
    return count;
}


int TankVBOHandler::drawLCase(bool shadow, int size)
{
    int count = 0;

    glShadeModel(GL_FLAT);
    if (shadow)
        vboVTN.enableArrays(false, false, false);
    else
        vboVTN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, lcaseIndex[size][0], lcaseCount[0]);
    count += 28;
    if (shadow)
        vboVT.enableArrays(false, false, false);
    else
        vboVT.enableArrays();

    glNormal3f(0.000000f, -1.000000f, 0.000000f);
    glDrawArrays(GL_TRIANGLE_FAN, lcaseIndex[size][1], lcaseCount[1]);
    count += 3;
    glDrawArrays(GL_TRIANGLE_FAN, lcaseIndex[size][2], lcaseCount[2]);
    count += 6;
    glDrawArrays(GL_TRIANGLE_FAN, lcaseIndex[size][3], lcaseCount[3]);
    count += 3;
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glDrawArrays(GL_TRIANGLE_FAN, lcaseIndex[size][4], lcaseCount[4]);
    count += 3;
    glDrawArrays(GL_TRIANGLE_FAN, lcaseIndex[size][5], lcaseCount[5]);
    count += 6;
    glDrawArrays(GL_TRIANGLE_FAN, lcaseIndex[size][6], lcaseCount[6]);
    count += 3;
    return count;
}


int TankVBOHandler::drawRCase(bool shadow, int size)
{
    int count = 0;

    glShadeModel(GL_FLAT);
    if (shadow)
        vboVTN.enableArrays(false, false, false);
    else
        vboVTN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, rcaseIndex[size][0], rcaseCount[0]);
    count += 28;
    if (shadow)
        vboVT.enableArrays(false, false, false);
    else
        vboVT.enableArrays();

    glNormal3f(0.000000f, -1.000000f, 0.000000f);
    glDrawArrays(GL_TRIANGLE_FAN, rcaseIndex[size][1], rcaseCount[1]);
    count += 3;
    glDrawArrays(GL_TRIANGLE_FAN, rcaseIndex[size][2], rcaseCount[2]);
    count += 6;
    glDrawArrays(GL_TRIANGLE_FAN, rcaseIndex[size][3], rcaseCount[3]);
    count += 3;
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glDrawArrays(GL_TRIANGLE_FAN, rcaseIndex[size][4], rcaseCount[4]);
    count += 3;
    glDrawArrays(GL_TRIANGLE_FAN, rcaseIndex[size][5], rcaseCount[5]);
    count += 6;
    glDrawArrays(GL_TRIANGLE_FAN, rcaseIndex[size][6], rcaseCount[6]);
    count += 3;
    return count;
}

int TankVBOHandler::drawALCase(bool shadow, int size)
{
    int count = 0;

    glShadeModel(GL_FLAT);
    if (shadow)
        vboVT.enableArrays(false, false, false);
    else
        vboVT.enableArrays();
    glNormal3f(0.0f, -1.0f, 0.0f);
    glDrawArrays(GL_TRIANGLE_STRIP, LCasingRIndex[size], LCasingRCount);
    count += 4;
    glNormal3f(0.0f, +1.0f, 0.0f);
    glDrawArrays(GL_TRIANGLE_STRIP, LCasingLIndex[size], LCasingLCount);
    count += 4;
    if (treadStyle == TankGeometryUtils::Covered)
    {
        if (shadow)
            vboVTN.enableArrays(false, false, false);
        else
            vboVTN.enableArrays();
        glDrawArrays(GL_TRIANGLE_STRIP, lcaseAIndex[size][0], lcaseACount[0]);
        count += 28;
        if (shadow)
            vboVT.enableArrays(false, false, false);
        else
            vboVT.enableArrays();
        glNormal3f(0.000000f, -1.000000f, 0.000000f);
        glDrawArrays(GL_TRIANGLE_FAN, lcaseAIndex[size][1], lcaseACount[1]);
        count += 3;
        glDrawArrays(GL_TRIANGLE_FAN, lcaseAIndex[size][2], lcaseACount[2]);
        count += 6;
        glDrawArrays(GL_TRIANGLE_FAN, lcaseAIndex[size][3], lcaseACount[3]);
        count += 3;
        glNormal3f(0.000000f, +1.000000f, 0.000000f);
        glDrawArrays(GL_TRIANGLE_FAN, lcaseAIndex[size][4], lcaseACount[4]);
        count += 3;
        glDrawArrays(GL_TRIANGLE_FAN, lcaseAIndex[size][5], lcaseACount[5]);
        count += 6;
        glDrawArrays(GL_TRIANGLE_FAN, lcaseAIndex[size][6], lcaseACount[6]);
        count += 3;
    }
    glShadeModel(GL_SMOOTH);

    return count;
}

int TankVBOHandler::drawARCase(bool shadow, int size)
{
    int count = 0;

    glShadeModel(GL_FLAT);
    if (shadow)
        vboVT.enableArrays(false, false, false);
    else
        vboVT.enableArrays();
    glNormal3f(0.0f, -1.0f, 0.0f);
    glDrawArrays(GL_TRIANGLE_STRIP, RCasingRIndex[size], RCasingRCount);
    count += 4;
    glNormal3f(0.0f, +1.0f, 0.0f);
    glDrawArrays(GL_TRIANGLE_STRIP, RCasingLIndex[size], RCasingLCount);
    count += 4;
    if (treadStyle == TankGeometryUtils::Covered)
    {
        if (shadow)
            vboVTN.enableArrays(false, false, false);
        else
            vboVTN.enableArrays();
        glDrawArrays(GL_TRIANGLE_STRIP, rcaseAIndex[size][0], rcaseACount[0]);
        count += 28;
        if (shadow)
            vboVT.enableArrays(false, false, false);
        else
            vboVT.enableArrays();
        glNormal3f(0.000000f, -1.000000f, 0.000000f);
        glDrawArrays(GL_TRIANGLE_FAN, rcaseAIndex[size][1], rcaseACount[1]);
        count += 3;
        glDrawArrays(GL_TRIANGLE_FAN, rcaseAIndex[size][2], rcaseACount[2]);
        count += 6;
        glDrawArrays(GL_TRIANGLE_FAN, rcaseAIndex[size][3], rcaseACount[3]);
        count += 3;
        glNormal3f(0.000000f, +1.000000f, 0.000000f);
        glDrawArrays(GL_TRIANGLE_FAN, rcaseAIndex[size][4], rcaseACount[4]);
        count += 3;
        glDrawArrays(GL_TRIANGLE_FAN, rcaseAIndex[size][5], rcaseACount[5]);
        count += 6;
        glDrawArrays(GL_TRIANGLE_FAN, rcaseAIndex[size][6], rcaseACount[6]);
        count += 3;
    }
    glShadeModel(GL_SMOOTH);

    return count;
}

int TankVBOHandler::drawTread(bool shadow, int size, int right)
{
    if (shadow)
        vboVTN.enableArrays(false, false, false);
    else
        vboVTN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, TreadIndex[size][0][right], TreadCount[0]);
    glShadeModel(GL_FLAT);
    if (shadow)
        vboVT.enableArrays(false, false, false);
    else
        vboVT.enableArrays();
    glNormal3f(0.0f, -1.0f, 0.0f);
    glDrawArrays(GL_TRIANGLE_STRIP, TreadIndex[size][1][right], TreadCount[1]);
    glNormal3f(0.0f, +1.0f, 0.0f);
    glDrawArrays(GL_TRIANGLE_STRIP, TreadIndex[size][2][right], TreadCount[2]);
    glShadeModel(GL_SMOOTH);

    return 2 * 4 * (32 + 2);
}

int TankVBOHandler::drawWheel(bool shadow, int size, int number, int right)
{
    int count = 0;

    if (shadow)
        vboVTN.enableArrays(false, false, false);
    else
        vboVTN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, wheelIndex[size][number][0][right], wheelCount[0]);
    count += 32;
    glShadeModel(GL_FLAT);
    if (shadow)
        vboVT.enableArrays(false, false, false);
    else
        vboVT.enableArrays();
    glNormal3f(0.0f, +1.0f, 0.0f);
    glDrawArrays(GL_TRIANGLE_FAN, wheelIndex[size][number][1][right], wheelCount[1]);
    count += 14;
    glNormal3f(0.0f, -1.0f, 0.0f);
    glDrawArrays(GL_TRIANGLE_FAN, wheelIndex[size][number][2][right], wheelCount[2]);
    count += 14;
    glShadeModel(GL_SMOOTH);

    return count;
}

void TankVBOHandler::drawLights(bool colorOverride, int size)
{
    const float *scale = scaleFactors[size];

    glPointSize(2.0f);
    glPushMatrix();
    glScalef(scale[0], scale[1], scale[2]);
    if (colorOverride)
        vboVC.enableArrays(false, false, false);
    else
        vboVC.enableArrays();
    glDrawArrays(GL_POINTS, lightsIndex, 3);
    glPopMatrix();
    glPointSize(1.0f);
}

void TankVBOHandler::drawJet()
{
    vboVT.enableArrays();
    glDrawArrays(GL_TRIANGLES, jetIndex, 3);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
