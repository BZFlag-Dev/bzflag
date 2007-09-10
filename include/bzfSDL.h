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

#ifndef __BZFSDL_H__
#define __BZFSDL_H__

/** this file contains headers necessary for SDL */

#ifdef HAVE_SDL
#  ifdef HAVE_SDL_SDL_H
#    include <SDL/SDL.h>
#    include <SDL/SDL_thread.h>
#    include <SDL/SDL_getenv.h>
#  else // autotools adds an SDL-specific include path
#    include "SDL.h"
#    include "SDL_thread.h"
#    include "SDL_getenv.h"
#  endif //HAVE_SDL_SDL_H
#endif //HAVE_SDL

#endif /* __BZFSDL_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
