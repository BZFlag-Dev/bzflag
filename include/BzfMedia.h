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

#ifndef __BZFMEDIA_H__
#define __BZFMEDIA_H__

#include "common.h"

#include <string>
#include <stdio.h>

static const std::string    DEFAULT_MEDIA_DIR = "data";

/** BzfMedia is a helper class that will read in audio and image
 * data files.  It's an abstract, platform independant base for
 * media I/O.
 *
 */
class BzfMedia
{
public:
    BzfMedia();
    virtual     ~BzfMedia();

    // get and set default directory to look for media files in
    std::string     getMediaDirectory() const;
    virtual void    setMediaDirectory(const std::string&);

    // images are stored RGBARGBA..., left to right, bottom to top.
    // depth indicates how many channels were in the stored image.
    // use delete[] to release the returned memory.
    unsigned char*  readImage(const std::string& filename,
                              int& width, int& height, int& depth) const;

    std::string     findSound(const std::string name, std::string extension) const;

    // sleep for given number of seconds
    virtual double  stopwatch(bool start);

protected:
    // return default extensions for image and sound files
    virtual std::string getImageExtension() const;
    virtual std::string getSoundExtension() const;

    // return NULL on failure
    virtual unsigned char* doReadImage(const std::string& filename,
                                       int& width, int& height, int& depth) const;

    // concatenate directory to filename
    virtual std::string makePath(const std::string& dir,
                                 const std::string& filename) const;

    // replace (or add) extension to pathname
    std::string     replaceExtension(const std::string& pathname,
                                     const std::string& extension) const;

    // return the position of the extension separator, or zero if
    // it cannot be found.
    virtual int     findExtension(const std::string& pathname) const;

    std::string     mediaDir;
};

#endif // __BZFMEDIA_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
