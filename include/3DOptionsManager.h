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
#ifndef _3DOPTIONS_MANAGER_H
#define _3DOPTIONS_MANAGER_H

#include "common.h"
#include "Singleton.h"

typedef enum
{
  nearest = 0,
  linear,
  nearestMipmapNearest,
  linearMipmapNearest,
  nearestMipmapLinear,
  linearMipmapLinear,
  aniIsotropic
}textureFilterModes;

typedef enum
{
    minimum = 0,
    low,
    normal,
    high
}renderQuality;

typedef struct
{
  bool		textures[3];
  textureFilterModes  filterMode;
  renderQuality       quality;
  bool		blending;
  bool		dither;
  bool		smoothing;
  bool		shadows;
  bool		lighting;
  bool		dethBuffer;
  bool		hardware;
  bool		stencel;
  bool		hudViewport;
}renderOptions;

class OptionsManager : public Singleton<OptionsManager>
{
public:
  void  init ( void );
  void  save ( void );

  // texture stuff
  bool  useObjectTexures ( void );
  void  setObjectTextures ( bool on );

  bool  useWorldTexures ( void );
  void  setWorldTextures ( bool on );

  bool  useEnvironmentTexures ( void );
  void  setnvironmentTexures( bool on );

  textureFilterModes  getFilterMode ( void );
  void  setFilterMode ( textureFilterModes mode );

  // overall quality
  renderQuality   getQuality ( void );
  void  setQuality ( renderQuality quality );

  // GL state modes
  bool  getBlending ( void );
  void  setBlending ( bool on );

  bool  getDither ( void );
  void  setDither ( bool on );

  bool  getSmothing ( void );
  void  setSmothing ( bool on );

  bool  getLighting ( void );
  void  setLighting  ( bool on );

  bool  getDepthBuffer( void );
  void  setDepthBuffer  ( bool on );

  // automated setup
  void computeBestSetings ( void );

protected:
  friend class Singleton<OptionsManager>;

private:
  OptionsManager();
  OptionsManager(const OptionsManager &om);
  OptionsManager& operator=(const OptionsManager &om);
  ~OptionsManager();

  renderOptions   options;
 };

#endif//_3DOPTIONS_MANAGER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
