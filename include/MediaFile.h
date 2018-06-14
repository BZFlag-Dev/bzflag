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

#ifndef BZF_MEDIA_FILE_H
#define BZF_MEDIA_FILE_H

#include "common.h"

/* system interface headers */
#include <string>

/* common interface headers */
#include "bzfio.h"


/** This class is a base class for media files, which can be image files or
    audio files. */
class MediaFile
{
public:
    /** Close the media file.  This does *not* destroy the stream. */
    virtual ~MediaFile();

    /** Read an image file.  Use delete[] to release the returned
        image.  Returns NULL on failure.  Images are stored RGBA,
        left to right, bottom to top. */
    static unsigned char* readImage(std::string filename,
                                    int* width, int* height, void(*error_callback)(std::string, bool) = nullptr);

protected:
    MediaFile(std::istream*);

private:
    std::istream*         stream;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
