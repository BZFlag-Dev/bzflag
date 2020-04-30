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

#include "common.h"

// system headers
#include <string>
#include <string.h>

// common headers
#include "bzfio.h"
#include "StateDatabase.h"
#include "bzfgl.h"
#include "OpenGLTexture.h"
#include "OpenGLGState.h"
#include "OpenGLUtils.h"

#ifndef _WIN32
typedef int64_t s64;
#else
typedef __int64 s64;
#endif


//
// OpenGLTexture::Rep
//

const GLenum        OpenGLTexture::minifyFilter[] =
{
    GL_NEAREST,
    GL_NEAREST,
    GL_LINEAR,
    GL_NEAREST_MIPMAP_NEAREST,
    GL_LINEAR_MIPMAP_NEAREST,
    GL_NEAREST_MIPMAP_LINEAR,
    GL_LINEAR_MIPMAP_LINEAR
};
const GLenum        OpenGLTexture::magnifyFilter[] =
{
    GL_NEAREST,
    GL_NEAREST,
    GL_LINEAR,
    GL_NEAREST,
    GL_LINEAR,
    GL_NEAREST,
    GL_LINEAR
};
const char*     OpenGLTexture::configFilterNames[] =
{
    "no",
    "nearest",
    "linear",
    "nearestmipmapnearest",
    "linearmipmapnearest",
    "nearestmipmaplinear",
    "linearmipmaplinear"
};


//
// OpenGLTexture
//

const int OpenGLTexture::filterCount = Max + 1;
OpenGLTexture::Filter OpenGLTexture::maxFilter = Default;


OpenGLTexture::OpenGLTexture(int _width, int _height, const GLvoid* pixels,
                             Filter _filter, bool _repeat) :
    width(_width), height(_height)
{
    repeat = _repeat;
    filter = _filter;
    list = INVALID_GL_TEXTURE_ID;

    // copy/scale the original texture image
    setupImage((const GLubyte*)pixels);

    // get internal format
    getBestFormat();

    // build and bind the GL texture
    initContext();

    // watch for context recreation
    OpenGLGState::registerContextInitializer(static_freeContext,
            static_initContext, (void*)this);
}


OpenGLTexture::~OpenGLTexture()
{
    OpenGLGState::unregisterContextInitializer(static_freeContext,
            static_initContext, (void*)this);
    delete[] imageMemory;

    freeContext();
    return;
}


void OpenGLTexture::static_freeContext(void *that)
{
    ((OpenGLTexture*) that)->freeContext();
}


void OpenGLTexture::static_initContext(void *that)
{
    ((OpenGLTexture*) that)->initContext();
}


void OpenGLTexture::freeContext()
{
    // glDeleteTextures should set binding to 0 by itself when the texture
    //  is in use, but some stacks (Linux/glx/matrox) are broken, so play it safe
    glBindTexture(GL_TEXTURE_2D, 0);

    if (list != INVALID_GL_TEXTURE_ID)
    {
        glDeleteTextures(1, &list);
        list = INVALID_GL_TEXTURE_ID;
    }
    return;
}


void OpenGLTexture::initContext()
{
    // make texture map object/list
    glGenTextures(1, &list);

    // now make texture map display list (compute all mipmaps, if requested).
    // compute next mipmap from current mipmap to save time.
    setFilter(filter);
    glBindTexture(GL_TEXTURE_2D, list);
    if (GLEW_VERSION_1_4)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
                     scaledWidth, scaledHeight,
                     0, internalFormat, GL_UNSIGNED_BYTE, image);
    }
    else
    {
        gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat,
                          scaledWidth, scaledHeight,
                          internalFormat, GL_UNSIGNED_BYTE, image);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    return;
}


void OpenGLTexture::setupImage(const GLubyte* pixels)
{
    if (GLEW_ARB_texture_non_power_of_two)
    {
        scaledWidth = width;
        scaledHeight = height;
        // Remove check of max size. I don't want to use gluScale if GL can handle non POT
    }
    else
    {
        // align to a 2^N value
        scaledWidth = 1;
        scaledHeight = 1;
        while (scaledWidth < width)
            scaledWidth <<= 1;
        while (scaledHeight < height)
            scaledHeight <<= 1;

        // get maximum valid size for texture (boost to 2^m x 2^n)
        GLint maxTextureSize;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

        // hard limit, some drivers have problems with sizes greater
        // then this (espeically if you are using glTexSubImage2D)
        const GLint dbMaxTexSize = BZDB.evalInt("maxTextureSize");
        GLint bzMaxTexSize = 1;
        // align the max size to a power of two  (wasteful)
        while (bzMaxTexSize < dbMaxTexSize)
            bzMaxTexSize <<= 1;

        if ((maxTextureSize < 0) || (maxTextureSize > bzMaxTexSize))
            maxTextureSize = bzMaxTexSize;

        // clamp to the maximum size
        if (scaledWidth > maxTextureSize)
            scaledWidth = maxTextureSize;
        if (scaledHeight > maxTextureSize)
            scaledHeight = maxTextureSize;
    }

    // copy the data into a 4-byte aligned buffer
    GLubyte* unaligned = new GLubyte[4 * width * height + 4];
    GLubyte* aligned = (GLubyte*)(((unsigned long)unaligned & ~3) + 4);
    ::memcpy(aligned, pixels, 4 * width * height);

    // scale the image if required
    if ((scaledWidth != width) || (scaledHeight != height))
    {
        GLubyte* unalignedScaled = new GLubyte[4 * scaledWidth * scaledHeight + 4];
        GLubyte* alignedScaled = (GLubyte*)(((unsigned long)unalignedScaled & ~3) + 4);

        // FIXME: 0 is success, return false otherwise...
        gluScaleImage (GL_RGBA, width, height, GL_UNSIGNED_BYTE, aligned,
                       scaledWidth, scaledHeight, GL_UNSIGNED_BYTE, alignedScaled);

        delete[] unaligned;
        unaligned = unalignedScaled;
        aligned = alignedScaled;
        logDebugMessage(1,"Scaling texture from %ix%i to %ix%i\n",
                        width, height, scaledWidth, scaledHeight);
    }

    // set the image
    image = aligned;
    imageMemory = unaligned;
}


OpenGLTexture::Filter OpenGLTexture::getFilter()
{
    return filter;
}


void OpenGLTexture::setFilter(Filter _filter)
{
    filter = _filter;

    int filterIndex = (int) filter;
    // limit filter.  try to keep nearest... filters as nearest and
    // linear... as linear.
    if (filterIndex > maxFilter)
    {
        if ((filterIndex & 1) == 1)   // nearest...
        {
            if ((maxFilter & 1) == 1)
                filterIndex = maxFilter;
            else
                filterIndex = maxFilter > 0 ? maxFilter - 1 : 0;
        }
        else   // linear...
        {
            if ((maxFilter & 1) == 1)
                filterIndex = maxFilter - 1;
            else
                filterIndex = maxFilter;
        }
    }
    GLint binding;
    glGetIntegerv (GL_TEXTURE_BINDING_2D, &binding);
    glBindTexture(GL_TEXTURE_2D, list);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minifyFilter[filterIndex]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnifyFilter[filterIndex]);
    if (OpenGLGState::hasAnisotropicFiltering)
    {
        GLint aniso = BZDB.evalInt("aniso");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
    }
    glBindTexture(GL_TEXTURE_2D, binding);
}


OpenGLTexture::Filter OpenGLTexture::getMaxFilter()
{
    return maxFilter;
}

void OpenGLTexture::setMaxFilter(Filter _filter)
{
    maxFilter = _filter;
}


void OpenGLTexture::execute()
{
    bind();
}


const char* OpenGLTexture::getFilterName(OpenGLTexture::Filter filter)
{
    if ((filter < 0) || (filter > Max))
        return configFilterNames[Max];
    else
        return configFilterNames[filter];
}


int OpenGLTexture::getFilterCount()
{
    return filterCount;
}


const char** OpenGLTexture::getFilterNames()
{
    return configFilterNames;
}


float OpenGLTexture::getAspectRatio() const
{
    return ((float) height) / ((float) width);
}


void OpenGLTexture::bind()
{
    if (list != INVALID_GL_TEXTURE_ID)
        glBindTexture(GL_TEXTURE_2D, list);
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0); // heh, it's the same call
    }
}


void OpenGLTexture::getBestFormat()
{
    GLubyte* scan = image;
    const int size = scaledWidth * scaledHeight;
    int i;
    bool useLuminance = true;
    alpha = false;
    for (i = 0; i < size; scan += 4, i++)
    {
        // see if all pixels are achromatic
        if (scan[0] != scan[1] || scan[0] != scan[2])
            useLuminance = false;
        // see if all pixels are opaque
        if (scan[3] != 0xff)
            alpha = true;
        if (!useLuminance && alpha)
            break;
    }

    scan = image;
    GLubyte* scanOut = image;
    for (i = 0; i < size; i++)
    {
        *scanOut++ = *scan++;
        if (useLuminance)
        {
            scan++;
            scan++;
        }
        else
        {
            *scanOut++ = *scan++;
            *scanOut++ = *scan++;
        }
        if (alpha)
            *scanOut++ = *scan++;
        else
            scan++;
    }

    // pick internal format
    internalFormat = useLuminance ?
                     (alpha ? GL_LUMINANCE_ALPHA : GL_LUMINANCE) :
                     (alpha ? GL_RGBA : GL_RGB);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
