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

/* SDLMedia:
 *  Media I/O on SDL
 */

#ifndef BZF_SDLMEDIA_H
#define BZF_SDLMEDIA_H
#include "BzfMedia.h"
#include "bzfSDL.h"
#include <string>

class SDLMedia : public BzfMedia
{
public:
    SDLMedia();
    ~SDLMedia() {};

    void        setMediaDirectory(const std::string&);
    double      stopwatch(bool);

private:
    Uint32      stopwatchTime;
};

#endif // BZF_SDLMEDIA_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
