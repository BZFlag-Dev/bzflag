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
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "OpenGLGState.h"


// use the namespaces
using namespace TankGeometryMgr;
using namespace TankGeometryEnums;
using namespace TankGeometryUtils;


// Local Variables
// ---------------

// handy dandy casted value
static const GLuint InvalidList = (GLuint) -1;

// the display lists
static GLuint displayLists[LastTankShadow][LastTankLOD]
                          [LastTankSize][LastTankPart];
         
// the scaling factors
static GLfloat scaleFactors[LastTankSize][3] = {
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
typedef void (*partFunction)(void);
static const partFunction partFunctions[LastTankLOD][BasicTankParts] = {
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
    buildHighLCasing,
    buildHighRCasing
  }
};


// Local Function Prototypes
// -------------------------

static void setupScales();
static void initContext(void *data);
static void bzdbCallback(const std::string& str, void *data);


/****************************************************************************/

// TankGeometryMgr Functions
// -------------------------
    

void TankGeometryMgr::init()
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
  
  // install the BZDB callbacks
  // This MUST be done after BZDB has been initialized in main()
  BZDB.addCallback (StateDatabase::BZDB_OBESEFACTOR, bzdbCallback, NULL);
  BZDB.addCallback (StateDatabase::BZDB_TINYFACTOR, bzdbCallback, NULL);
  BZDB.addCallback (StateDatabase::BZDB_THIEFTINYFACTOR, bzdbCallback, NULL);
  
  // install the context initializer
  OpenGLGState::registerContextInitializer (initContext, NULL);
  
  // setup the scaleFactors
  setupScales();

  return;
}


void TankGeometryMgr::kill()
{
  OpenGLGState::unregisterContextInitializer(initContext, NULL);
  return;
}


void TankGeometryMgr::deleteLists()
{
  // delete the lists that have been aquired
  for (int shadow = 0; shadow < LastTankShadow; shadow++) {
    for (int lod = 0; lod < LastTankLOD; lod++) {
      for (int size = 0; size < LastTankSize; size++) {
        for (int part = 0; part < LastTankPart; part++) {
          GLuint& list = displayLists[shadow][lod][size][part];
          if (list != InvalidList) {
            if (glIsList(list) == GL_FALSE) {
              DEBUG3("TankGeometryMgr: "
                     "tried to delete an invalid list (%i)\n", list);
            } else {
              glDeleteLists(list, 1);
            }
            list = InvalidList;
          }
        }
      }
    }
  }
  return;
}


void TankGeometryMgr::buildLists()
{
  // setup the scale factors
  setupScales();
  currentScaleFactor = scaleFactors[Normal];
  
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
          currentScaleFactor = scaleFactors[size];
          
          if (part < MedTankParts) {
            // the basic parts
            partFunctions[lod][part]();
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


GLuint TankGeometryMgr::getPartList(TankGeometryEnums::TankShadow shadow,
                                    TankGeometryEnums::TankPart part,
                                    TankGeometryEnums::TankSize size,
                                    TankGeometryEnums::TankLOD lod)
{                   
  return displayLists[shadow][lod][size][part];
}


const float* TankGeometryMgr::getScaleFactor(TankSize size)
{
  return scaleFactors[size];
}


/****************************************************************************/

// Local Functions
// ---------------


static void bzdbCallback(const std::string& name, void * /*data*/)
{
  deleteLists();
  buildLists();
  DEBUG3 ("TankGeometryMgr bzdbCallback(%s)\n", name.c_str());
  return;
}


static void initContext(void * /*data*/)
{
  // we have to assume that the lists can not be deleted
  buildLists();
  DEBUG3 ("TankGeometryMgr initContext()\n");
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


void TankGeometryUtils::doVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
  const float* scale = currentScaleFactor;
  x = x * scale[0];
  y = y * scale[1];
  z = z * scale[2];
  glVertex3f(x, y, z);
  return;
}


void TankGeometryUtils::doNormal3f(GLfloat x, GLfloat y, GLfloat z)
{
  if (shadowMode == ShadowOn) {
    return;
  }
  const float* scale = currentScaleFactor;
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
  if (shadowMode == ShadowOn) {
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

