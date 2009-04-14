/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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


//
// OpenGLTexture::Rep
//

const unsigned int OpenGLTexture::minifyFilter[] = {
  GL_NEAREST,
  GL_NEAREST,
  GL_LINEAR,
  GL_NEAREST_MIPMAP_NEAREST,
  GL_LINEAR_MIPMAP_NEAREST,
  GL_NEAREST_MIPMAP_LINEAR,
  GL_LINEAR_MIPMAP_LINEAR
};

const unsigned int OpenGLTexture::magnifyFilter[] = {
  GL_NEAREST,
  GL_NEAREST,
  GL_LINEAR,
  GL_NEAREST,
  GL_LINEAR,
  GL_NEAREST,
  GL_LINEAR
};

const char* OpenGLTexture::configFilterNames[] = {
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


OpenGLTexture::OpenGLTexture(int _width, int _height, const void* pixels,
			     Filter _filter, bool _repeat, int _internalFormat)
: width(_width)
, height(_height)
{
  alpha = false;
  repeat = _repeat;
  internalFormat = _internalFormat;
  filter = _filter;
  list = INVALID_GL_TEXTURE_ID;

  // get internal format if not provided
  if (internalFormat == 0)
    internalFormat = getBestFormat(width, height, pixels);

  // copy/scale the original texture image
  setupImage(pixels);

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

  if (list != INVALID_GL_TEXTURE_ID) {
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
  gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat,
		    scaledWidth, scaledHeight,
		    GL_RGBA, GL_UNSIGNED_BYTE, image);
  glBindTexture(GL_TEXTURE_2D, 0);

  return;
}

void OpenGLTexture::replateImageData(const void* pixels)
{
  freeContext();
  memcpy(image, pixels, internalFormat * scaledWidth * scaledHeight);
  initContext();
}


bool OpenGLTexture::setupImage(const void* pixels)
{
  // align to a 2^N value
  scaledWidth = 1;
  scaledHeight = 1;
  while (scaledWidth < width) {
    scaledWidth <<= 1;
  }
  while (scaledHeight < height) {
    scaledHeight <<= 1;
  }

  // get maximum valid size for texture (boost to 2^m x 2^n)
  GLint maxTextureSize = 1024;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

  if (BZDB.isSet("forceMaxTextureSize")) // gk knows it's max size, but if they REALY want to force it, do it
  {
    // hard limit, some drivers have problems with sizes greater
    // then this (espeically if you are using glTexSubImage2D)
    const GLint dbMaxTexSize = BZDB.evalInt("forceMaxTextureSize");
    GLint bzMaxTexSize = 1;
    if (dbMaxTexSize > 0) {
      // align the max size to a power of two  (wasteful)
      while (bzMaxTexSize < dbMaxTexSize) {
	bzMaxTexSize <<= 1;
      }
    } else {
      bzMaxTexSize = scaledHeight > scaledWidth ? scaledHeight : scaledWidth;
    }

    if ((maxTextureSize < 0) || (maxTextureSize > bzMaxTexSize)) {
      maxTextureSize = bzMaxTexSize;
    }
  }

  // clamp to the maximum size
  if (scaledWidth > maxTextureSize) {
    scaledWidth = maxTextureSize;
  }
  if (scaledHeight > maxTextureSize) {
    scaledHeight = maxTextureSize;
  }

  // NOTE: why these are 4-byte aligned is beyond me...

  // copy the data into a 4-byte aligned buffer
  GLubyte* unaligned = new GLubyte[4 * width * height + 4];
  GLubyte* aligned = (GLubyte*)(((unsigned long)unaligned & ~3) + 4);
  ::memcpy(aligned, pixels, 4 * width * height);

  // scale the image if required
  if ((scaledWidth != width) || (scaledHeight != height)) {
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

  // note if internal format uses alpha
  switch (internalFormat) {
    case GL_LUMINANCE_ALPHA:
#if defined(GL_LUMINANCE4_ALPHA4)
    case GL_LUMINANCE4_ALPHA4:
#elif defined(GL_LUMINANCE4_ALPHA4_EXT)
    case GL_LUMINANCE4_ALPHA4_EXT:
#endif
    case GL_RGBA:
#if defined(GL_INTENSITY4)
    case GL_INTENSITY4:
#elif defined(GL_INTENSITY4_EXT)
    case GL_INTENSITY4_EXT:
#endif
      alpha = true;
      break;
    default:
      alpha = false;
      break;
  }

  return true;
}


void OpenGLTexture::setFilter(Filter _filter)
{
  filter = _filter;

  int filterIndex = (int) filter;
  // limit filter.  try to keep nearest... filters as nearest and
  // linear... as linear.
  if (filterIndex > maxFilter) {
    if ((filterIndex & 1) == 1)	{ // nearest...
      if ((maxFilter & 1) == 1) {
	filterIndex = maxFilter;
      } else {
	filterIndex = maxFilter > 0 ? maxFilter - 1 : 0;
      }
    }
    else { // linear...
      if ((maxFilter & 1) == 1) {
	filterIndex = maxFilter - 1;
      } else {
	filterIndex = maxFilter;
      }
    }
  }
  realFilter = (Filter)filterIndex;

  GLint binding;
  glGetIntegerv (GL_TEXTURE_BINDING_2D, &binding);
  glBindTexture(GL_TEXTURE_2D, list);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minifyFilter[filterIndex]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnifyFilter[filterIndex]);
  if (GLEW_EXT_texture_filter_anisotropic) {
    GLint aniso = BZDB.evalInt("aniso");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
  }
  glBindTexture(GL_TEXTURE_2D, binding);
}


OpenGLTexture::Filter OpenGLTexture::getFilter()
{
  return filter;
}


unsigned int OpenGLTexture::getMinFilter()
{
  return minifyFilter[realFilter];
}


unsigned int OpenGLTexture::getMagFilter()
{
  return magnifyFilter[realFilter];
}


OpenGLTexture::Filter OpenGLTexture::getMaxFilter()
{
  return maxFilter;
}


void OpenGLTexture::setMaxFilter(Filter _filter)
{
  maxFilter = _filter;
}


bool OpenGLTexture::execute()
{
  return bind();
}


const char* OpenGLTexture::getFilterName(OpenGLTexture::Filter filter)
{
  if ((filter < 0) || (filter > Max)) {
    return configFilterNames[Max];
  } else {
    return configFilterNames[filter];
  }
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


bool OpenGLTexture::bind()
{
  if (list != INVALID_GL_TEXTURE_ID) {
    glBindTexture(GL_TEXTURE_2D, list);
    return true;
  } else {
    glBindTexture(GL_TEXTURE_2D, 0);
    return false;
  }
}


int OpenGLTexture::getBestFormat(int _width, int _height, const void* pixels)
{
  // see if all pixels are achromatic
  const GLubyte* scan = (const GLubyte*)pixels;
  const int size = _width * _height;
  int i;
  for (i = 0; i < size; scan += 4, i++) {
    if (scan[0] != scan[1] || scan[0] != scan[2]) {
      break;
    }
  }
  const bool useLuminance = (i == size);

  // see if all pixels are opaque
  scan = (const GLubyte*)pixels;
  for (i = 0; i < size; scan += 4, i++) {
    if (scan[3] != 0xff) {
      break;
    }
  }
  const bool useAlpha = (i != size);

  // see if all pixels are r=g=b=a.  if so return intensity format.
  // SGI IMPACT systems don't support GL_INTENSITY.
  const char* const glRenderer = (const char*)glGetString(GL_RENDERER);
  static bool noIntensity = ((glRenderer == NULL) ||
                             (strncmp(glRenderer, "IMPACT", 6) == 0));
  if (!noIntensity) {
    bool useIntensity = false;
    if (useLuminance) {
      scan = (const GLubyte*)pixels;
      for (i = 0; i < size; scan += 4, i++) {
	if (scan[3] != scan[0]) {
	  break;
        }
      }
      useIntensity = (i == size);
    }
    if (useIntensity) {
      return GL_INTENSITY;
    }
  }

  // pick internal format
  return (useLuminance ?
		(useAlpha ? GL_LUMINANCE_ALPHA : GL_LUMINANCE) :
		(useAlpha ? GL_RGBA : GL_RGB));
}


bool OpenGLTexture::getColorAverages(fvec4& rgba, bool factorAlpha) const
{
  if ((image == NULL) || (scaledWidth <= 0) || (scaledHeight <= 0)) {
    return false;
  }

  factorAlpha = (factorAlpha && alpha);
  const int channelCount = alpha ? 4 : 3;

  // tally the values
  int64_t rgbaTally[4] = { 0, 0, 0, 0 };
  for (int x = 0; x < scaledWidth; x++) {
    for (int y = 0; y < scaledHeight; y++) {
      for (int c = 0; c < channelCount; c++) {
	const int pixelBase = 4 * (x + (y * scaledWidth));
	if (factorAlpha) {
	  const GLubyte alphaVal = image[pixelBase + 3];
	  if (c == 3) {
	    rgbaTally[3] += alphaVal;
	  } else {
	    rgbaTally[c] += image[pixelBase + c] * alphaVal;
	  }
	} else {
	  rgbaTally[c] += image[pixelBase + c];
	}
      }
    }
  }

  // calculate the alpha average
  float maxTally = 255.0f * (scaledWidth * scaledHeight);
  if (channelCount == 3) {
    rgba.a = 1.0f;
  } else {
    rgba.a = (float)rgbaTally[3] / maxTally;
  }

  // adjust the maxTally for alpha weighting
  if (factorAlpha) {
    maxTally = maxTally * 255.0f;
  }

  // calcualte the color averages
  for (int c = 0; c < 3; c++) {
    rgba[c] = (float)rgbaTally[c] / maxTally;
  }

  return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
