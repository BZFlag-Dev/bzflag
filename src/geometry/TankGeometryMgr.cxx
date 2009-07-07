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

// bzflag common headers
#include "common.h"
#include "global.h"
#include "TextUtils.h"

// interface header
#include "TankGeometryMgr.h"

// system headers
#include <stdlib.h>
#include <math.h>
#include <string>
#include <fstream>

// common headers
#include "bzfgl.h"
#include "SceneRenderer.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "OpenGLGState.h"
#include "Model.h"
#include "vectors.h"

#include "PlatformFactory.h"
#include "BzfMedia.h"


// use the namespaces
using namespace TankGeometryMgr;
using namespace TankGeometryEnums;
using namespace TankGeometryUtils;


// Local Variables
// ---------------

// the display lists
static GLuint displayLists[LastTankShadow][LastTankLOD]
			  [LastTankSize][LastTankPart];

// triangle counds
static int partTriangles[LastTankShadow][LastTankLOD]
			[LastTankSize][LastTankPart];

// the scaling factors
static fvec3 scaleFactors[LastTankSize] = {
  fvec3(1.0f, 1.0f,   1.0f), // Normal
  fvec3(1.0f, 1.0f,   1.0f), // Obese
  fvec3(1.0f, 1.0f,   1.0f), // Tiny
  fvec3(1.0f, 0.001f, 1.0f), // Narrow
  fvec3(1.0f, 1.0f,   1.0f)  // Thief
};
// the current scaling factors
const fvec3* TankGeometryUtils::currentScaleFactor = &scaleFactors[Normal];

// the current shadow mode (used to remove glNormal3f and glTexcoord2f calls)
TankShadow TankGeometryUtils::shadowMode = ShadowOn;

// arrays of functions to avoid large switch statements
typedef int (*partFunction)(void);
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
static void freeContext(void *data);
static void initContext(void *data);
static void bzdbCallback(const std::string& str, void *data);


//============================================================================//

// TankGeometryMgr Functions
// -------------------------


void TankGeometryMgr::init()
{
  // initialize the lists to invalid
  for (int shadow = 0; shadow < LastTankShadow; shadow++) {
    for (int lod = 0; lod < LastTankLOD; lod++) {
      for (int size = 0; size < LastTankSize; size++) {
	for (int part = 0; part < LastTankPart; part++) {
	  displayLists[shadow][lod][size][part] = INVALID_GL_LIST_ID;
	  partTriangles[shadow][lod][size][part] = 0;
	}
      }
    }
  }

  // install the BZDB callbacks
  // This MUST be done after BZDB has been initialized in main()
  BZDB.addCallback (BZDBNAMES.TANKWIDTH,       bzdbCallback, NULL);
  BZDB.addCallback (BZDBNAMES.TANKHEIGHT,      bzdbCallback, NULL);
  BZDB.addCallback (BZDBNAMES.TANKLENGTH,      bzdbCallback, NULL);
  BZDB.addCallback (BZDBNAMES.OBESEFACTOR,     bzdbCallback, NULL);
  BZDB.addCallback (BZDBNAMES.TINYFACTOR,      bzdbCallback, NULL);
  BZDB.addCallback (BZDBNAMES.THIEFTINYFACTOR, bzdbCallback, NULL);
  BZDB.addCallback ("animatedTreads",          bzdbCallback, NULL);
  BZDB.addCallback ("treadStyle",              bzdbCallback, NULL);

  // install the context initializer
  OpenGLGState::registerContextInitializer (freeContext, initContext, NULL);

  // setup the scaleFactors
  setupScales();

  return;
}


void TankGeometryMgr::kill()
{
  // remove the BZDB callbacks
  BZDB.removeCallback (BZDBNAMES.TANKWIDTH,       bzdbCallback, NULL);
  BZDB.removeCallback (BZDBNAMES.TANKHEIGHT,      bzdbCallback, NULL);
  BZDB.removeCallback (BZDBNAMES.TANKLENGTH,      bzdbCallback, NULL);
  BZDB.removeCallback (BZDBNAMES.OBESEFACTOR,     bzdbCallback, NULL);
  BZDB.removeCallback (BZDBNAMES.TINYFACTOR,      bzdbCallback, NULL);
  BZDB.removeCallback (BZDBNAMES.THIEFTINYFACTOR, bzdbCallback, NULL);
  BZDB.removeCallback ("animatedTreads",          bzdbCallback, NULL);
  BZDB.removeCallback ("treadStyle",              bzdbCallback, NULL);

  // remove the context initializer callback
  OpenGLGState::unregisterContextInitializer(freeContext, initContext, NULL);

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
	  if (list != INVALID_GL_LIST_ID) {
	    glDeleteLists(list, 1);
	    list = INVALID_GL_LIST_ID;
	  }
	}
      }
    }
  }
  return;
}


void TankGeometryMgr::buildLists()
{
  // setup the tread style
  setTreadStyle(BZDB.evalInt("treadStyle"));

  // setup the scale factors
  setupScales();
  currentScaleFactor = &scaleFactors[Normal];
  const bool animated = BZDBCache::animatedTreads;

  // setup the quality level
  const int divisionLevels[4][2] = { // wheel divs, tread divs
    {  4,  4 }, // low
    {  8, 16 }, // med
    { 12, 24 }, // high
    { 16, 32 }  // experimental
  };
  int quality = RENDERER.useQuality();
  if (quality < _LOW_QUALITY) {
    quality = _LOW_QUALITY;
  } else if (quality > _EXPERIMENTAL_QUALITY) {
    quality = _EXPERIMENTAL_QUALITY;
  }

  int wheelDivs = divisionLevels[quality][0];
  int treadDivs = divisionLevels[quality][1];

  for (int shadow = 0; shadow < LastTankShadow; shadow++) {
    for (int lod = 0; lod < LastTankLOD; lod++) {
      for (int size = 0; size < LastTankSize; size++) {

	// only do the basics, unless we're making an animated tank
	int lastPart = BasicTankParts;
	if (animated) {
	  lastPart = HighTankParts;
	}

	// set the shadow mode for the doNormal3f() and doTexcoord2f() calls
	shadowMode = (TankShadow) shadow;

	for (int part = 0; part < lastPart; part++) {

	  GLuint& list = displayLists[shadow][lod][size][part];
	  int& count = partTriangles[shadow][lod][size][part];

	  // get a new OpenGL display list
	  list = glGenLists(1);
	  glNewList(list, GL_COMPILE);

	  // setup the scale factor
	  currentScaleFactor = &scaleFactors[size];

	  if ((part <= Turret)  || (!animated)) {
	    // the basic parts
	    count = partFunctions[lod][part]();
	  }
	  else if (lod == HighTankLOD){
	    // the animated parts
	    if (part == LeftCasing) {
	      count = buildHighLCasingAnim();
	    }
	    else if (part == RightCasing) {
	      count = buildHighRCasingAnim();
	    }
	    else if (part == LeftTread) {
	      count = buildHighLTread(treadDivs);
	    }
	    else if (part == RightTread) {
	      count = buildHighRTread(treadDivs);
	    }
	    else if ((part >= LeftWheel0) && (part <= LeftWheel3)) {
	      int wheel = part - LeftWheel0;
	      count = buildHighLWheel(wheel, (float)wheel * (float)(M_PI / 2.0),
				      wheelDivs);
	    }
	    else if ((part >= RightWheel0) && (part <= RightWheel3)) {
	      int wheel = part - RightWheel0;
	      count = buildHighRWheel(wheel, (float)wheel * (float)(M_PI / 2.0),
				      wheelDivs);
	    }
	  }

	  // end of the list
	  glEndList();

	} // part
      } // size
    } // lod
  } // shadow

  return;
}


unsigned int TankGeometryMgr::getPartList(TankGeometryEnums::TankShadow shadow,
                                          TankGeometryEnums::TankPart part,
                                          TankGeometryEnums::TankSize size,
                                          TankGeometryEnums::TankLOD lod)
{
  return displayLists[shadow][lod][size][part];
}


int TankGeometryMgr::getPartTriangleCount(TankGeometryEnums::TankShadow sh,
					  TankGeometryEnums::TankPart part,
					  TankGeometryEnums::TankSize size,
					  TankGeometryEnums::TankLOD lod)
{
  return partTriangles[sh][lod][size][part];
}


const fvec3& TankGeometryMgr::getScaleFactor(TankSize size)
{
  return scaleFactors[size];
}

std::map<std::string,OBJModel> modelMap;

bool TankGeometryUtils::buildGeoFromObj ( const char* path, int &count  )
{
  std::string mediaPath = PlatformFactory::getMedia()->getMediaDirectory();
  mediaPath += "/models/";
  mediaPath += BZDB.get("playerModel") + path;
      count = 0;

  if (modelMap.find(mediaPath) != modelMap.end())
  {
    count = modelMap[mediaPath].draw();
    return count > 0;
  }

  OBJModel model;
  if (model.read(mediaPath))
  {
    modelMap[mediaPath] = model;
    count = model.draw();
  }
  return count > 0;
}

//============================================================================//
//
//  Utility functions
//

void TankGeometryUtils::doVertex3f(float x, float y, float z)
{
  const fvec3* scale = currentScaleFactor;
  const fvec3 pos(x * scale->x, y * scale->y, z * scale->z);
  glVertex3fv(pos);
  return;
}


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


void TankGeometryUtils::doTexCoord2f(float x, float y)
{
  if (shadowMode == TankGeometryEnums::ShadowOn) {
    return;
  }
  glTexCoord2f(x, y);
  return;
}


void TankGeometryUtils::doVertex(const fvec3& v)
{
  doVertex3f(v.x, v.y, v.z);
}


void TankGeometryUtils::doNormal(const fvec3& n)
{
  doNormal3f(n.x, n.y, n.z);
}


void TankGeometryUtils::doTexCoord(const fvec2& t)
{
  doTexCoord2f(t.x, t.y);
}


//============================================================================//
//
// Local Functions
//


static void bzdbCallback(const std::string& /*name*/, void * /*data*/)
{
  deleteLists();
  buildLists();
  return;
}


static void freeContext(void * /*data*/)
{
  // delete all of the lists
  deleteLists();
  return;
}


static void initContext(void * /*data*/)
{
  buildLists();
  return;
}


static void setupScales()
{
  float scale;

  scaleFactors[Normal].x = BZDBCache::tankLength;
  scale = (float)atof(BZDB.getDefault(BZDBNAMES.TANKLENGTH).c_str());
  scaleFactors[Normal].x /= scale;

  scaleFactors[Normal].y = BZDBCache::tankWidth;
  scale = (float)atof(BZDB.getDefault(BZDBNAMES.TANKWIDTH).c_str());
  scaleFactors[Normal].y /= scale;

  scaleFactors[Normal].z = BZDBCache::tankHeight;
  scale = (float)atof(BZDB.getDefault(BZDBNAMES.TANKHEIGHT).c_str());
  scaleFactors[Normal].z /= scale;

  scale = BZDB.eval(BZDBNAMES.OBESEFACTOR);
  scaleFactors[Obese].x = scale * scaleFactors[Normal].x;
  scaleFactors[Obese].y = scale * scaleFactors[Normal].y;
  scaleFactors[Obese].z = scaleFactors[Normal].z;

  scale = BZDB.eval(BZDBNAMES.TINYFACTOR);
  scaleFactors[Tiny].x = scale * scaleFactors[Normal].x;
  scaleFactors[Tiny].y = scale * scaleFactors[Normal].y;
  scaleFactors[Tiny].z = scaleFactors[Normal].z;

  scale = BZDB.eval(BZDBNAMES.THIEFTINYFACTOR);
  scaleFactors[Thief].x = scale * scaleFactors[Normal].x;
  scaleFactors[Thief].y = scale * scaleFactors[Normal].y;
  scaleFactors[Thief].z = scaleFactors[Normal].z;

  scaleFactors[Narrow].x = scaleFactors[Normal].x;
  scaleFactors[Narrow].y = 0.001f;
  scaleFactors[Narrow].z = scaleFactors[Normal].z;

  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
