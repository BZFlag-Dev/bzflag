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

#include "common.h"

/* interface header */
#include "SDLVisual.h"


void SDLVisual::setDoubleBuffer(bool on) {
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, on ? 1 : 0);
}

void SDLVisual::setRGBA(int minRed, int minGreen,
			int minBlue, int minAlpha) {
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   minRed);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, minGreen);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  minBlue);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, minAlpha);
}

void SDLVisual::setDepth(int minDepth) {
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, minDepth);
}

void SDLVisual::setStencil(int minDepth) {
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, minDepth);
}

void SDLVisual::setStereo(bool on) {
  SDL_GL_SetAttribute(SDL_GL_STEREO, on ? 1 : 0);
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
