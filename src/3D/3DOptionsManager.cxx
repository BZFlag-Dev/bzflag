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

#ifdef _MSC_VER
#pragma warning( 4: 4786 )
#endif

#include "3DOptionsManager.h"
#include "TextureManager.h"
#include "StateDatabase.h"
#include "BZDBCache.h"

/*const int NO_VARIANT = (-1); */

// initialize the singleton
template <>
OptionsManager* Singleton<OptionsManager>::_instance = (OptionsManager*)0;

void  OptionsManager::init ( void )
{
  int textureMode = BZDB.evalInt("texture");
  if (textureMode == 0) {
    options.textures[0] = options.textures[1] = options.textures[2] = false;
  } else {
    options.textures[0] = BZDB.isTrue("objectTextures");
    options.textures[1] = BZDB.isTrue("worldTextures");
    options.textures[2] = BZDB.isTrue("environmentTextures");
    options.filterMode = (textureFilterModes)(textureMode - 1);
  }

  options.blending = BZDB.isTrue("blend");
  options.dither = BZDB.isTrue("dither");
  options.smoothing = BZDB.isTrue("smooth");
  options.dethBuffer = BZDBCache::zbuffer;
  options.quality  = (renderQuality)BZDB.evalInt("useQuality");
  options.shadows  = BZDB.isTrue("shadows");
  options.lighting  = BZDBCache::lighting;
}

void  OptionsManager::save ( void )
{
  int textureMode = 0;
  if (options.textures)
    textureMode = options.filterMode+1;

  BZDB.setInt("texture",textureMode);

  BZDB.setBool("blend", options.blending );
  BZDB.setBool("dither", options.dither );
  BZDB.setBool("smooth", options.smoothing );
  BZDB.setBool("zbuffer", options.dethBuffer );
  BZDB.setInt("useQuality", options.quality );
  BZDB.setBool("shadows", options.shadows );
  BZDB.setBool("lighting", options.lighting );
}

// texture stuff
bool  OptionsManager::useObjectTexures ( void )
{
  return options.textures[0];
}

void  OptionsManager::setObjectTextures ( bool on )
{
  options.textures[0] = on;
}

bool  OptionsManager::useWorldTexures ( void )
{
  return options.textures[1];
}

void  OptionsManager::setWorldTextures ( bool on )
{
  options.textures[1] = on;
}

bool  OptionsManager::useEnvironmentTexures ( void )
{
  return options.textures[2];
}

void  OptionsManager::setnvironmentTexures( bool on )
{
  options.textures[2] = on;
}

textureFilterModes  OptionsManager::getFilterMode ( void )
{
  return options.filterMode;
}

void  OptionsManager::setFilterMode ( textureFilterModes mode )
{
  options.filterMode = mode;
}

// overall quality
renderQuality   OptionsManager::getQuality ( void )
{
  return options.quality;
}
void  OptionsManager::setQuality ( renderQuality quality )
{
  options.quality = quality;
}

// GL state modes
bool  OptionsManager::getBlending ( void )
{
  return options.blending;
}

void  OptionsManager::setBlending ( bool on )
{
  options.blending = on;
}

bool  OptionsManager::getDither ( void )
{
  return options.dither;
}

void  OptionsManager::setDither ( bool on )
{
  options.dither = on;
}

bool  OptionsManager::getSmothing ( void )
{
  return options.smoothing;
}

void  OptionsManager::setSmothing ( bool on )
{
  options.smoothing = on;
}

bool  OptionsManager::getLighting ( void )
{
  return options.lighting;
}

void  OptionsManager::setLighting  ( bool on )
{
  options.lighting = on;
}

bool  OptionsManager::getDepthBuffer( void )
{
  return options.dethBuffer;
}

void  OptionsManager::setDepthBuffer  ( bool on )
{
  options.dethBuffer = on;
}

// automated setup
void OptionsManager::computeBestSetings ( void )
{
    // do some tests
}

  OptionsManager::OptionsManager()
{
  options.textures[0] = options.textures[1] = options.textures[2] = true;
  options.filterMode = linearMipmapLinear;
  options.quality = normal;
  options.blending = true;
  options.dither = false;
  options.shadows = true;
  options.smoothing = true;
  options.lighting = true;
  options.dethBuffer = true;
  options.hardware = true;
  options.stencel = true;
  options.hudViewport = true;
}

  OptionsManager::OptionsManager(const OptionsManager &om)
    : Singleton<OptionsManager>()
  {
    options = om.options;
  }
  OptionsManager& OptionsManager::operator=(const OptionsManager &om)
  {
    options = om.options;
    return *this;
  }
  OptionsManager::~OptionsManager()
  {

  }


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

