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

/* interface header */
#include "MediaFile.h"

/* system implementation headers */
#include <iostream>
#include <string>
#include <algorithm>

/* common implementation headers */
#include "CacheManager.h"


#ifdef WIN32
static void ConvertPath(std::string &path)
{
    std::replace(path.begin(), path.end(), '/', '\\');
}
#endif

//
// MediaFile
//

MediaFile::MediaFile(std::istream* _stream) : stream(_stream)
{
    // do nothing
}

MediaFile::~MediaFile()
{
    // do nothing
}

//
// utility methods to read various media files in any supported format
//

#include "FileManager.h"
#include "PNGImageFile.h"


std::istream*  OpenPNG(const std::string filename, ImageFile** file, void(*error_callback)(std::string, bool))
{
    std::string extension = "";
    std::istream* stream = FILEMGR.createDataInStream(filename, true);
    if (stream == nullptr)
    {
        extension = PNGImageFile::getExtension();
        stream = FILEMGR.createDataInStream(filename + extension, true);
    }

    if (stream != nullptr)
    {
        std::string tmp = filename + extension;
        *file = new PNGImageFile(stream, &tmp, error_callback);
        if (!(*file)->isOpen())
        {
            *file = nullptr;
            delete stream;
            stream = nullptr;
        }
    }

    return stream;
}

unsigned char*      MediaFile::readImage( std::string filename, int* width, int* height,
        void(*error_callback)(std::string, bool))
{
    // get the absolute filename for cache textures
    if (CACHEMGR.isCacheFileType(filename))
        filename = CACHEMGR.getLocalName(filename);

#ifdef WIN32
    // cheat and make sure the file is a windows file path
    ConvertPath(filename);
#endif //WIN32

    // try opening file as an image
    std::istream* stream = nullptr;
    ImageFile* file = nullptr;
    if (file == nullptr)
        stream = OpenPNG(filename, &file, error_callback);

    // read the image
    unsigned char* image = NULL;
    if (file != NULL)
    {
        // get the image size
        unsigned int dx = *width  = file->getWidth();
        unsigned int dy = *height = file->getHeight();
        unsigned int dz = file->getNumChannels();

        // make buffer for final image
        image = new unsigned char[dx * dy * 4];

        // make buffer to read image.  if the image file has 4 channels
        // then read directly into the final image buffer.
        unsigned char* buffer = (dz == 4) ? image : new unsigned char[dx * dy * dz];

        // read the image
        if (image != NULL && buffer != NULL)
        {
            if (!file->read(buffer))
            {
                // failed to read image.  clean up.
                if (buffer != image)
                    delete[] buffer;
                delete[] image;
                image  = NULL;
                buffer = NULL;
            }
            else
            {
                // expand image into 4 channels
                int n = dx * dy;
                const unsigned char* src = buffer;
                unsigned char* dst = image;
                if (dz == 1)
                {
                    // r=g=b=i, a=max
                    for (; n > 0; --n)
                    {
                        dst[0] = dst[1] = dst[2] = src[0];
                        dst[3] = 0xff;
                        src += 1;
                        dst += 4;
                    }
                }
                else if (dz == 2)
                {
                    // r=g=b=i
                    for (; n > 0; --n)
                    {
                        dst[0] = dst[1] = dst[2] = src[0];
                        dst[3] = src[1];
                        src += 2;
                        dst += 4;
                    }
                }
                else if (dz == 3)
                {
                    // a=max
                    for (; n > 0; --n)
                    {
                        dst[0] = src[0];
                        dst[1] = src[1];
                        dst[2] = src[2];
                        dst[3] = 0xff;
                        src += 3;
                        dst += 4;
                    }
                }
            }
        }

        // clean up
        if (buffer != image)
            delete[] buffer;
        delete file;
    }

    // clean up
    delete stream;

    return image;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
