/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_PNG_IMAGE_FILE_H
#define BZF_PNG_IMAGE_FILE_H

#include "ImageFile.h"
#include <png.h>
#include <utility>


/** This class represents a PNG image file. It implements the read() function
    from ImageFile. */
class PNGImageFile : public ImageFile
{
public:
    PNGImageFile(std::istream* input, std::string* filename, void(*callback)(std::string, bool) = nullptr);
    virtual ~PNGImageFile();

    /** This function returns the default extension of PNG image files. */
    static std::string    getExtension();

    /** Read image data from a PNG file. */
    virtual bool      read(void* buffer);
private:
    png_structp               png;
    png_infop                 pnginfo;

    // Storage for reading the image
    png_bytep*                rowPtrs;

    png_uint_32               bitDepth;
    png_uint_32               colorType;
    png_uint_32               compressionMethod;
    png_uint_32               filterMethod;
    png_uint_32               interlaceMethod;

    std::pair<std::string, void(*)(std::string, bool)> callback_pair;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
