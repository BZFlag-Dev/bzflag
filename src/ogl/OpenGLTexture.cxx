/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"
#include <string>
#include "bzfio.h"
#include "bzfgl.h"
#include "OpenGLTexture.h"
#include "OpenGLGState.h"

//
// OpenGLTexture::Rep
//

const GLenum		OpenGLTexture::minifyFilter[] = {
				GL_NEAREST,
				GL_NEAREST,
				GL_LINEAR,
				GL_NEAREST_MIPMAP_NEAREST,
				GL_LINEAR_MIPMAP_NEAREST,
				GL_NEAREST_MIPMAP_LINEAR,
				GL_LINEAR_MIPMAP_LINEAR
			};
const GLenum		OpenGLTexture::magnifyFilter[] = {
				GL_NEAREST,
				GL_NEAREST,
				GL_LINEAR,
				GL_NEAREST,
				GL_LINEAR,
				GL_NEAREST,
				GL_LINEAR
			};
const char*		OpenGLTexture::configFilterValues[] = {
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

OpenGLTexture::Filter	OpenGLTexture::filter = LinearMipmapLinear;


OpenGLTexture::OpenGLTexture(int _width, int _height,
				const GLvoid* pixels,
				Filter _maxFilter,
				bool _repeat,
				int _internalFormat)
				: alpha(false),
				width(_width),
				height(_height),
				repeat(_repeat),
				internalFormat(_internalFormat),
				list(INVALID_GL_TEXTURE_ID),
				maxFilter(_maxFilter)
{

  // get internal format if not provided
  if (internalFormat == 0)
    internalFormat = getBestFormat(width, height, pixels);

  // copy the original texture image
  image = new GLubyte[4 * width * height];
  ::memcpy(image, pixels, 4 * width * height);

  setFilter(configFilterValues[static_cast<int>(_maxFilter)]);
  initContext();
  // watch for context recreation
  OpenGLGState::registerContextInitializer(static_freeContext,
					   static_initContext, (void*)this);
}

OpenGLTexture::~OpenGLTexture()
{
  OpenGLGState::unregisterContextInitializer(static_freeContext,
					     static_initContext, (void*)this);
  // free image data
  delete[] image;
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

  // making sure we don't change these values accidentally in this function
  const int tmpWidth = width;
  const int tmpHeight = height;


  // align to a 2^N value
  GLint scaledWidth = 1;
  GLint scaledHeight = 1;
  while (scaledWidth < tmpWidth) {
    scaledWidth <<= 1;
  }
  while (scaledHeight < tmpHeight) {
    scaledHeight <<= 1;
  }
  
  // get minimum valid size for texture (boost to 2^m x 2^n)
  GLint maxTextureSize;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

  // hard limit, some drivers have problems with sizes greater
  // then this (espeically if you are using glTexSubImage2D)
  const GLint bzMaxTexSize = 512;
  if (maxTextureSize > bzMaxTexSize) {
    maxTextureSize = bzMaxTexSize;
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
  GLubyte* unaligned = new GLubyte[4 * tmpWidth * tmpHeight + 4];
  GLubyte* aligned = (GLubyte*)(((unsigned long)unaligned & ~3) + 4);
  ::memcpy(aligned, image, 4 * tmpWidth * tmpHeight);

  // scale the image if required
  if ((scaledWidth != tmpWidth) || (scaledHeight != tmpHeight)) {
    GLubyte* unalignedScaled = new GLubyte[4 * scaledWidth * scaledHeight + 4];
    GLubyte* alignedScaled = (GLubyte*)(((unsigned long)unalignedScaled & ~3) + 4);

    gluScaleImage (GL_RGBA, tmpWidth, tmpHeight, GL_UNSIGNED_BYTE, aligned,
		   scaledWidth, scaledHeight, GL_UNSIGNED_BYTE, alignedScaled);
		   
    delete[] unaligned;
    unaligned = unalignedScaled;
    aligned = alignedScaled;
    DEBUG1("Scaling texture from %ix%i to %ix%i\n",
           tmpWidth, tmpHeight, scaledWidth, scaledHeight);
  }

  // now make texture map display list (compute all mipmaps, if requested).
  // compute next mipmap from current mipmap to save time.
  setFilter(getFilter());
  glBindTexture(GL_TEXTURE_2D, list);
  gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat,
		    scaledWidth, scaledHeight,
		    GL_RGBA, GL_UNSIGNED_BYTE, aligned);
  glBindTexture(GL_TEXTURE_2D, 0);

  // free the unaligned pointer
  delete[] unaligned;

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
  
  return;
}


OpenGLTexture::Filter OpenGLTexture::getFilter()
{
  return filter;
}


std::string OpenGLTexture::getFilterName()
{
  return configFilterValues[static_cast<int>(filter)];
}


void OpenGLTexture::setFilter(std::string name)
{
  for (unsigned int i = 0; i < (sizeof(configFilterValues) /
				sizeof(configFilterValues[0])); i++)
  if (name == configFilterValues[i])
    setFilter(static_cast<Filter>(i));
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
  glBindTexture(GL_TEXTURE_2D, list);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minifyFilter[filterIndex]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnifyFilter[filterIndex]);
}


void OpenGLTexture::execute()
{
  bind();
}


float OpenGLTexture::getAspectRatio() const
{
  return ((float) height) / ((float) width);
}


void OpenGLTexture::bind()
{
  if (list != INVALID_GL_TEXTURE_ID) {
    glBindTexture(GL_TEXTURE_2D, list);
  } else {
    glBindTexture(GL_TEXTURE_2D, 0); // heh, it's the same call
  }
}


int OpenGLTexture::getBestFormat(int width, int height, const GLvoid* pixels)
{
  // see if all pixels are achromatic
  const GLubyte* scan = (const GLubyte*)pixels;
  const int size = width * height;
  int i;
  for (i = 0; i < size; scan += 4, i++)
    if (scan[0] != scan[1] || scan[0] != scan[2])
      break;
  const bool useLuminance = (i == size);

  // see if all pixels are opaque
  scan = (const GLubyte*)pixels;
  for (i = 0; i < size; scan += 4, i++)
    if (scan[3] != 0xff)
      break;
  const bool useAlpha = (i != size);

  // intensity format defined in 1.1 and an extension in 1.0
  static const bool hasTextureExt = true;

  // see if all pixels are r=g=b=a.  if so return intensity format.
  // SGI IMPACT and 3Dfx systems don't support GL_INTENSITY.
  const char* const glRenderer = (const char*)glGetString(GL_RENDERER);
  static bool noIntensity =
    ((glRenderer == NULL) ||
     (strncmp(glRenderer, "IMPACT", 6) == 0) ||
     (strncmp(glRenderer, "3Dfx", 4) == 0));
  if (!noIntensity) {
    bool useIntensity = false;
    if (hasTextureExt && useLuminance) {
      scan = (const GLubyte*)pixels;
      for (i = 0; i < size; scan += 4, i++)
	if (scan[3] != scan[0])
	  break;
      useIntensity = (i == size);
    }
    if (useIntensity)
      return GL_INTENSITY;
  }

  // pick internal format
  return (useLuminance ?
		(useAlpha ? GL_LUMINANCE_ALPHA : GL_LUMINANCE) :
		(useAlpha ? GL_RGBA : GL_RGB));
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

