/* bzflag
  * Copyright (c) 1993 - 2007 Tim Riker
  *
  * This package is free software;  you can redistribute it and/or
  * modify it under the terms of the license found in the file
  * named COPYING that should have accompanied this file.
  *
  * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
  */

#ifndef	__CRACKEDGLASS_H__
#define	__CRACKEDGLASS_H__

/* local client interface headers */
#include "SceneRenderer.h"

/**
 * A CrackedGlass object encapsulates the rendering of cracked glass
 * on a display.
 */
class CrackedGlass {

private:
  static const int NUM_CRACKS = 8;
  static const int NUM_CRACKLEVELS = 4;

public:
  static void InitCracks(int maxMotionSize = 100);

  static void Render(SceneRenderer& renderer);

protected:
  CrackedGlass(); /* unused */
  ~CrackedGlass(); /* unused */

  static void RenderClassicCracks(SceneRenderer& renderer);
  static void RenderHighResCracks(SceneRenderer& renderer);

  static void MakeCrack(int maxMotionSize, float crackpattern[NUM_CRACKS][(1 << NUM_CRACKLEVELS) + 1][2], int n, int l, float a);

private:
  static float cracks[NUM_CRACKS][(1 << NUM_CRACKLEVELS) + 1][2];
};

#else
class CrackedGlass;
#endif /* __CRACKEDGLASS_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
