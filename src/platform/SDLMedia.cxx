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

#include "common.h"

#ifdef HAVE_SDL
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include "SDLMedia.h"
#include "ErrorHandler.h"

/* ugh, more mixing with libCommon */
#include "StateDatabase.h"

//
// SDLMedia
//

SDLMedia::SDLMedia() : BzfMedia()
{
}

void SDLMedia::setMediaDirectory(const std::string& _dir)
{
    struct stat statbuf;
    const char *mdir = _dir.c_str();

#ifdef __APPLE__
    extern char *GetMacOSXDataPath(void);
    if ((stat(mdir, &statbuf) != 0) || !S_ISDIR(statbuf.st_mode))
    {
        /* try the Resource folder if invoked from a .app bundled */
        mdir = GetMacOSXDataPath();
        if (mdir)
        {
            BZDB.set("directory", mdir);
            BZDB.setPersistent("directory", false);
        }
    }
#endif
    if ((stat(mdir, &statbuf) != 0) || !S_ISDIR(statbuf.st_mode))
    {
        /* try a simple 'data' dir in current directory (source build invocation) */
        std::string defaultdir = DEFAULT_MEDIA_DIR;
        mdir = defaultdir.c_str();
        if (mdir)
        {
            BZDB.set("directory", mdir);
            BZDB.setPersistent("directory", false);
        }
    }
    if ((stat(mdir, &statbuf) != 0) || !S_ISDIR(statbuf.st_mode))
    {
        /* give up, revert to passed in directory */
        mdir = _dir.c_str();
    }

    mediaDir = mdir;
}

double          SDLMedia::stopwatch(bool start)
{
    Uint32 currentTick = SDL_GetTicks(); //msec

    if (start)
    {
        stopwatchTime = currentTick;
        return 0.0;
    }
    if (currentTick >= stopwatchTime)
        return (double) (currentTick - stopwatchTime) * 0.001; // sec
    else
        //Clock is wrapped : happens after 49 days
        //Should be "wrap value" - stopwatchtime. Now approx.
        return (double) currentTick * 0.001;
}

#endif //HAVE_SDL
// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
