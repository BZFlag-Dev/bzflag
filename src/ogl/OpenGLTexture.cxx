/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "OpenGLTexture.h"
#include "OpenGLGState.h"
#include <string.h>
// glu used to resample textures for mipmaps;  should use something better.
#if defined(_MACOSX_)
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#if defined(GL_VERSION_1_1)
#  define	BZF_TEXTURE_OBJECT
#elif defined(GL_EXT_texture_object)
#  define	BZF_TEXTURE_OBJECT
#  define	glBindTexture		glBindTextureEXT
#  define	glDeleteTextures	glDeleteTexturesEXT
#  define	glGenTextures		glGenTexturesEXT
#endif

#if defined(GL_VERSION_1_1)
#  define	BZF_INTENSITY_FORMAT	GL_INTENSITY
#elif defined(GL_EXT_texture)
#  define	BZF_INTENSITY_FORMAT	GL_INTENSITY_EXT
#else
#  define	BZF_INTENSITY_FORMAT	0 /* not used */
#endif

static int		hasTextureObject = -1;

//
// OpenGLTexture::Rep
//


OpenGLTexture::Rep*	OpenGLTexture::Rep::first = NULL;
const GLenum		OpenGLTexture::Rep::minifyFilter[] = {
				GL_NEAREST,
				GL_NEAREST,
				GL_LINEAR,
				GL_NEAREST_MIPMAP_NEAREST,
				GL_LINEAR_MIPMAP_NEAREST,
				GL_NEAREST_MIPMAP_LINEAR,
				GL_LINEAR_MIPMAP_LINEAR
			};
const GLenum		OpenGLTexture::Rep::magnifyFilter[] = {
				GL_NEAREST,
				GL_NEAREST,
				GL_LINEAR,
				GL_NEAREST,
				GL_LINEAR,
				GL_NEAREST,
				GL_LINEAR
			};

OpenGLTexture::Rep::Rep(int _width, int _height,
				const GLvoid* pixels,
				int _maxFilter,
				bool _repeat,
				int _internalFormat) :
				refCount(1), list(0),
				alpha(false),
				width(_width),
				height(_height),
				repeat(_repeat),
				internalFormat(_internalFormat),
				maxFilter(_maxFilter)

{
  // check for texture object extension
  if (hasTextureObject < 0) {
#if defined(GL_VERSION_1_1)
    hasTextureObject = 1;
#elif defined(GL_EXT_texture_object)
    hasTextureObject = (strstr((const char*)glGetString(GL_EXTENSIONS),
					"GL_EXT_texture_object") != NULL);
#else
    hasTextureObject = 0;
#endif // BZF_TEXTURE_OBJECT
  }

  // add me to list
  next = first;
  first = this;

  // make texture map object/list
#if defined(BZF_TEXTURE_OBJECT)
  if (hasTextureObject > 0)
    glGenTextures(1, &list);
  else
#endif // BZF_TEXTURE_OBJECT
  list = glGenLists(1);

  // get internal format if not provided
  if (internalFormat == 0)
    internalFormat = getBestFormat(width, height, pixels);

  // copy the original texture image
  image = new GLubyte[4 * width * height];
  ::memcpy(image, pixels, 4 * width * height);

  // create the texture maps
  doInitContext();

  // watch for context recreation
  OpenGLGState::registerContextInitializer(initContext, (void*)this);
}

OpenGLTexture::Rep::~Rep()
{
  OpenGLGState::unregisterContextInitializer(initContext, (void*)this);

  // free image data
  delete[] image;

  // free OpenGL display list or texture object
#if defined(BZF_TEXTURE_OBJECT)
  if (hasTextureObject > 0) {
    if (list) glDeleteTextures(1, &list);
  }
  else
#endif // BZF_TEXTURE_OBJECT
  if (list) glDeleteLists(list, 1);

  // remove me from list
  if (this == first) {
    first = next;
  }
  else {
    for (Rep* scan = first; scan; scan = scan->next)
      if (scan->next == this) {
	scan->next = next;
	break;
      }
  }
}

void			OpenGLTexture::Rep::setFilter(int filter)
{
  // limit filter.  try to keep nearest... filters as nearest and
  // linear... as linear.
  if (filter > maxFilter) {
    if ((filter & 1) == 1)	// nearest...
      if ((maxFilter & 1) == 1) filter = maxFilter;
      else filter = maxFilter > 0 ? maxFilter - 1 : 0;

    else			// linear...
      if ((maxFilter & 1) == 1) filter = maxFilter - 1;
      else filter = maxFilter;
  }

#if defined(BZF_TEXTURE_OBJECT)
  if (hasTextureObject > 0)
    glBindTexture(GL_TEXTURE_2D, list);
#endif // BZF_TEXTURE_OBJECT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minifyFilter[filter]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnifyFilter[filter]);
}

void			OpenGLTexture::Rep::doInitContext()
{
  // set size
  int tmpWidth = width;
  int tmpHeight = height;

  // get minimum valid size for texture (boost to 2^m x 2^n)
  GLint scaledWidth = 1, scaledHeight = 1;
  GLint maxTextureSize;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
  if (maxTextureSize > 512) maxTextureSize = 512;
  while (scaledWidth < tmpWidth) scaledWidth <<= 1;
  while (scaledHeight < tmpHeight) scaledHeight <<= 1;
  if (scaledWidth > maxTextureSize) scaledWidth = maxTextureSize;
  if (scaledHeight > maxTextureSize) scaledHeight = maxTextureSize;
  const int copyWidth = (scaledWidth > tmpWidth) ? scaledWidth : tmpWidth;
  const int copyHeight = (scaledHeight > tmpHeight) ? scaledHeight : tmpHeight;

  // make buffers for copied and scaled data
  GLubyte* origData = new GLubyte[4 * copyWidth * copyHeight + 4];
  GLubyte* data = (GLubyte*)(((unsigned long)origData & ~3) + 4);
  GLubyte* origScaledData = new GLubyte[4 * scaledWidth * scaledHeight + 4];
  GLubyte* scaledData = (GLubyte*)(((unsigned long)origScaledData & ~3) + 4);
  ::memcpy(data, image, 4 * tmpWidth * tmpHeight);

  // note if internal format uses alpha
  switch (internalFormat) {
    case BZF_INTENSITY_FORMAT:
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

  // now make texture map display list (compute all mipmaps, if requested).
  // compute next mipmap from current mipmap to save time.
  const bool mipmap = ((int)maxFilter > (int)Linear);
  GLint mipmapLevel = 0;

#if defined(BZF_TEXTURE_OBJECT)
  if (hasTextureObject > 0)
    glBindTexture(GL_TEXTURE_2D, list);
  else
#endif // BZF_TEXTURE_OBJECT
  glNewList(list, GL_COMPILE);

  setFilter(maxFilter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
			repeat ? GL_REPEAT : GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
			repeat ? GL_REPEAT : GL_CLAMP);
  do {
    bool doScale = (scaledWidth != tmpWidth || scaledHeight != tmpHeight);

    // scale image to next mipmap level
    if (doScale)
      gluScaleImage(
		GL_RGBA, tmpWidth, tmpHeight, GL_UNSIGNED_BYTE, data,
		scaledWidth ? scaledWidth : 1,
		scaledHeight ? scaledHeight : 1,
		GL_UNSIGNED_BYTE, scaledData);

    // make texture
    glTexImage2D(GL_TEXTURE_2D, mipmapLevel, internalFormat,
		scaledWidth ? scaledWidth : 1,
		scaledHeight ? scaledHeight : 1,
		0, GL_RGBA, GL_UNSIGNED_BYTE,
		doScale ? scaledData : data);

    // prepare for next iteration
    mipmapLevel++;
    tmpWidth = scaledWidth;
    tmpHeight = scaledHeight;
    scaledWidth >>= 1;
    scaledHeight >>= 1;
    if (doScale)
      ::memcpy(data, scaledData, 4 * tmpWidth * tmpHeight);
  } while (mipmap && (scaledWidth > 0 || scaledHeight > 0));

  setFilter(getFilter());

#if defined(BZF_TEXTURE_OBJECT)
  if (hasTextureObject > 0)
    glBindTexture(GL_TEXTURE_2D, 0);
  else
#endif // BZF_TEXTURE_OBJECT
  glEndList();

  // free extra buffers
  delete[] origScaledData;
  delete[] origData;
}

void			OpenGLTexture::Rep::initContext(void* self)
{
  ((Rep*)self)->doInitContext();
}

//
// OpenGLTexture
//

OpenGLTexture::Rep*	OpenGLTexture::lastRep = NULL;
OpenGLTexture::Filter	OpenGLTexture::filter = LinearMipmapLinear;

OpenGLTexture::OpenGLTexture()
{
  rep = NULL;
}

OpenGLTexture::OpenGLTexture(int width, int height,
				const GLvoid* pixels,
				Filter maxFilter,
				bool repeat,
				int internalFormat)
{
  rep = new Rep(width, height, pixels, (int)maxFilter, repeat, internalFormat);
}

OpenGLTexture::OpenGLTexture(const OpenGLTexture& t)
{
  rep = t.rep;
  ref();
}

OpenGLTexture::~OpenGLTexture()
{
  if (unref()) delete rep;
}

OpenGLTexture&		OpenGLTexture::operator=(const OpenGLTexture& t)
{
  if (rep != t.rep) {
    if (unref()) delete rep;
    rep = t.rep;
    ref();
  }
  return *this;
}

OpenGLTexture::Filter	OpenGLTexture::getFilter()
{
  return filter;
}

void			OpenGLTexture::setFilter(Filter _filter)
{
  filter = _filter;

#if defined(BZF_TEXTURE_OBJECT)

  // can only change filters when using texture objects
  if (hasTextureObject <= 0) return;

  // change filter on all textures
  for (Rep* scan = Rep::first; scan; scan = scan->next)
    scan->setFilter(filter);

  // back to previously bound texture, or no texture if filter is Off
  if (Rep::first != lastRep && filter != Off) bind(lastRep);
  else if (lastRep) bind(NULL);

#else // BZF_TEXTURE_OBJECT

  // bind no texture if filter is Off
  if (filter == Off && lastRep) bind(NULL);

#endif // BZF_TEXTURE_OBJECT
}

bool			OpenGLTexture::operator==(const OpenGLTexture& t) const
{
  return (rep == t.rep);
}

bool			OpenGLTexture::operator!=(const OpenGLTexture& t) const
{
  return (rep != t.rep);
}

bool			OpenGLTexture::operator<(const OpenGLTexture& t) const
{
  if (rep == t.rep) return false;
  if (!t.rep) return false;
  if (!rep) return true;
  return (rep->list < t.rep->list);
}

GLuint			OpenGLTexture::getList() const
{
  return (!rep ? 0 : rep->list);
}

void			OpenGLTexture::execute() const
{
  bind(rep);
  lastRep = rep;
}

void			OpenGLTexture::ref()
{
  if (rep) ++rep->refCount;
}

bool			OpenGLTexture::unref()
{
  return (rep && --rep->refCount == 0);
}

void			OpenGLTexture::bind(Rep* r)
{
#if defined(BZF_TEXTURE_OBJECT)
  if (hasTextureObject > 0) {
    if (r && r->list) glBindTexture(GL_TEXTURE_2D, r->list);
    else glBindTexture(GL_TEXTURE_2D, 0);
  }
  else
#endif // BZF_TEXTURE_OBJECT
  if (r && r->list) glCallList(r->list);
  else glTexImage2D(GL_TEXTURE_2D, 0, 3, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
}

int			OpenGLTexture::Rep::getBestFormat(
				int width, int height,
				const GLvoid* pixels)
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
#if defined(GL_VERSION_1_1)
  static const bool hasTextureExt = true;
#elif defined(GL_INTENSITY_EXT)
  static const bool hasTextureExt = (strstr((const char*)
		glGetString(GL_EXTENSIONS), "GL_EXT_texture") != NULL);
#else
  static const bool hasTextureExt = false;
#endif // defined(GL_VERSION_1_1)

  // see if all pixels are r=g=b=a.  if so return intensity format.
  // SGI IMPACT and 3Dfx systems don't support GL_INTENSITY.
  const char* const glRenderer = (const char*)glGetString(GL_RENDERER);
  static bool noIntensity =
	(strncmp(glRenderer, "IMPACT", 6) == 0) ||
	(strncmp(glRenderer, "3Dfx", 4) == 0);
  if (!noIntensity) {
    bool useIntensity = false;
    if (hasTextureExt && useLuminance) {
      scan = (const GLubyte*)pixels;
      for (i = 0; i < size; scan += 4, i++)
	if (scan[3] != scan[0])
	  break;
      useIntensity = (i == size);
    }
    if (useIntensity) return BZF_INTENSITY_FORMAT;
  }

  // pick internal format
  return (useLuminance ?
		(useAlpha ? GL_LUMINANCE_ALPHA : GL_LUMINANCE) :
		(useAlpha ? GL_RGBA : GL_RGB));
}
// ex: shiftwidth=2 tabstop=8
