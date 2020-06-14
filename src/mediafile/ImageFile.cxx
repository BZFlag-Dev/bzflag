/* bzflag
 * Copyright (c) 1993-2020 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "ImageFile.h"

//
// ImageFile
//

ImageFile::ImageFile(std::istream* _stream) : MediaFile(_stream), open(false),
    numChannels(), width(), height()
{
    // do nothing
}

ImageFile::~ImageFile()
{
    // do nothing
}

void            ImageFile::init(unsigned int _numChannels,
                                unsigned int _width, unsigned int _height)
{
    open  = true;
    numChannels = _numChannels;
    width       = _width;
    height      = _height;
}

bool            ImageFile::isOpen() const
{
    return open;
}

unsigned int        ImageFile::getNumChannels() const
{
    return numChannels;
}

unsigned int        ImageFile::getWidth() const
{
    return width;
}

unsigned int        ImageFile::getHeight() const
{
    return height;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
