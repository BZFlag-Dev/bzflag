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

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <string>
#include "common.h"
#include "global.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "TankGeometryMgr.h"
#include "OpenGLGState.h"


// the one and only
TankGeometryMgr TANKGEOMMGR;

// handy dandy casted value
static const GLuint InvalidList = (GLuint) -1;

// use the namespaces
using namespace TankGeometryEnums;
using namespace TankGeometryUtils;

GLfloat TankGeometryMgr::scaleFactors[LastTankSize][3] = {
  {1.0f, 1.0f, 1.0f},   // Normal
  {1.0f, 1.0f, 1.0f},   // Obese
  {1.0f, 1.0f, 1.0f},   // Tiny
  {1.0f, 0.001f, 1.0f}, // Narrow
  {1.0f, 1.0f, 1.0f}    // Thief
};
const float* TankGeometryMgr::scaleFactor = scaleFactors[Normal];

TankShadow TankGeometryMgr::shadowMode = ShadowOn;

GLuint TankGeometryMgr::displayLists
         [LastTankShadow][LastTankLOD][LastTankSize][LastTankPart];

const TankGeometryMgr::PartFunction 
  TankGeometryMgr::partFunctions[LastTankLOD][BasicTankParts] = {
  { buildLowBody, 
    buildLowBarrel,
    buildLowTurret,
    buildLowLCasing,
    buildLowRCasing
  },
  { buildMedBody,
    buildMedBarrel,
    buildMedTurret,
    buildMedLCasing,
    buildMedRCasing
  },
  { buildHighBody,
    buildHighBarrel,
    buildHighTurret,
    buildHighLCasingOld,
    buildHighRCasingOld
  }
};
    

TankGeometryMgr::TankGeometryMgr()
{
  // initialize the lists to invalid
  for (int shadow = 0; shadow < LastTankShadow; shadow++) {
    for (int lod = 0; lod < LastTankLOD; lod++) {
      for (int size = 0; size < LastTankSize; size++) {
        for (int part = 0; part < LastTankPart; part++) {
          displayLists[shadow][lod][size][part] = InvalidList;
        }
      }
    }
  }
  callbacksInstalled = false;
  OpenGLGState::registerContextInitializer (initContext, (void*)this);
  
  return;
}


TankGeometryMgr::~TankGeometryMgr()
{
  // FIXME: do we still have GL?  
  deleteLists();
  
  // FIXME: really worth doing?
  if (callbacksInstalled) {
    OpenGLGState::unregisterContextInitializer(initContext, (void*)this);
  }
  return;
}


void TankGeometryMgr::deleteLists()
{
  // delete the lists that have been aquired
  for (int shadow = 0; shadow < LastTankShadow; shadow++) {
    for (int lod = 0; lod < LastTankLOD; lod++) {
      for (int size = 0; size < LastTankSize; size++) {
        for (int part = 0; part < LastTankPart; part++) {
          GLuint list = displayLists[shadow][lod][size][part];
          if (list != InvalidList) {
            glDeleteLists(list, 1);
            list = InvalidList;
          }
        }
      }
    }
  }
  return;
}


void TankGeometryMgr::setupScales(const std::string& /*name*/, void * /*data*/)
{
  float scale;
  
  scaleFactors[Normal][0] = BZDB.eval(StateDatabase::BZDB_TANKLENGTH);
  scale = (float)atof(BZDB.getDefault(StateDatabase::BZDB_TANKLENGTH).c_str());
  scaleFactors[Normal][0] /= scale;

  scaleFactors[Normal][1] = BZDB.eval(StateDatabase::BZDB_TANKWIDTH);
  scale = (float)atof(BZDB.getDefault(StateDatabase::BZDB_TANKWIDTH).c_str());
  scaleFactors[Normal][1] /= scale;

  scaleFactors[Normal][2] = BZDB.eval(StateDatabase::BZDB_TANKHEIGHT);
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


void TankGeometryMgr::initContext(void * /*data*/)
{
  // initialize the lists to invalid
  for (int shadow = 0; shadow < LastTankShadow; shadow++) {
    for (int lod = 0; lod < LastTankLOD; lod++) {
      for (int size = 0; size < LastTankSize; size++) {
        for (int part = 0; part < LastTankPart; part++) {
          displayLists[shadow][lod][size][part] = InvalidList;
        }
      }
    }
  }
  TANKGEOMMGR.rebuildLists();
  return;
}


void TankGeometryMgr::rebuildLists()
{
  // install the calllbacks
  if (!callbacksInstalled) {
    // the BZDB callbacks (at least), cannot be installed at
    // global statup because BZDB is not initialized until 
    // main() is called
    BZDB.addCallback (StateDatabase::BZDB_OBESEFACTOR, setupScales, NULL);
    BZDB.addCallback (StateDatabase::BZDB_TINYFACTOR, setupScales, NULL);
    BZDB.addCallback (StateDatabase::BZDB_THIEFTINYFACTOR, setupScales, NULL);
    callbacksInstalled = true;
  }

  // setup the scale factors
  std::string junk;
  setupScales(junk, NULL);
  scaleFactor = scaleFactors[Normal];
  
  // delete any old lists
  deleteLists();

  for (int shadow = 0; shadow < LastTankShadow; shadow++) {
    for (int lod = 0; lod < LastTankLOD; lod++) {
      for (int size = 0; size < LastTankSize; size++) {

        // only do the basics, unless we're making a HighTank
        int lastPart = BasicTankParts;
        if (lod == HighTankLOD) {
          lastPart = HighTankParts;
        }
        
        // set the shadow mode for the doNormal3f() and doTexcoord2f()
        shadowMode = (TankShadow) shadow;
        
        for (int part = 0; part < lastPart; part++) {

          // get a new OpenGL display list
          GLuint& list = displayLists[shadow][lod][size][part];
          list = glGenLists(1);
          glNewList(list, GL_COMPILE);
          
          // setup the scale factor
          scaleFactor = scaleFactors[size];
          
          if (part < MedTankParts) {
            // the basic parts
            if (!(lod == HighTankLOD)) {
              partFunctions[lod][part]();
            } else {
              if (part == LeftCasing) {
                buildHighLCasing(30);
              }
              else if (part == RightCasing) {
                buildHighRCasing(30);
              }
              else {
                partFunctions[lod][part]();
              }
            }
          } 
          else {
            // the animated parts
            switch (part) {
              case LeftTread: {
                buildHighLTread(30);
                break;
              }
              case RightTread: {
                buildHighRTread(30);
                break;
              }
              case LeftWheel0:
              case LeftWheel1:
              case LeftWheel2:
              case LeftWheel3: {
                int wheel = part - LeftWheel0;
                buildHighLWheel(wheel, (float)wheel * M_PI/2.0f, 12);
                break; 
              }
              case RightWheel0:
              case RightWheel1:
              case RightWheel2:
              case RightWheel3: {
                int wheel = part - RightWheel0;
                buildHighRWheel(wheel, (float)wheel * M_PI/2.0f, 12);
                break; 
              }
            } // end part switch
          }

          // end of the list        
          glEndList();

        } // part
      } // size
    } // lod
  } // shadow
  
  return;  
}


void TankGeometryUtils::doVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
  const float* scale = TankGeometryMgr::currentScaleFactor();
  x = x * scale[0];
  y = y * scale[1];
  z = z * scale[2];
  glVertex3f(x, y, z);
  return;
}


void TankGeometryUtils::doNormal3f(GLfloat x, GLfloat y, GLfloat z)
{
  if (TankGeometryMgr::getShadowMode() == ShadowOn) {
    return;
  }
  const float* scale = TankGeometryMgr::currentScaleFactor();
  GLfloat sx = x * scale[0];
  GLfloat sy = y * scale[1];
  GLfloat sz = z * scale[2];
  const GLfloat d = sqrtf ((sx * sx) + (sy * sy) + (sz * sz));
  if (d > 1.0e-5f) {
    x *= scale[0] / d;
    y *= scale[1] / d;
    z *= scale[2] / d;
  }
  glNormal3f(x, y, z);
  return;
}


void TankGeometryUtils::doTexCoord2f(GLfloat x, GLfloat y)
{
  if (TankGeometryMgr::getShadowMode() == ShadowOn) {
    return;
  }
  glTexCoord2f(x, y);
  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

