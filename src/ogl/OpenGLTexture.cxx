/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "OpenGLTexture.h"
#include "OpenGLGState.h"
#include "MediaFile.h"
#include "ErrorHandler.h"
#include "bzfgl.h"
#include <string.h>
// glu used to resample textures for mipmaps;  should use something better.
#include <GL/glu.h>

//
// OpenGLTexture::Rep
//


OpenGLTexture::Rep*		OpenGLTexture::Rep::first = NULL;
const int				OpenGLTexture::Rep::minifyFilter[] = {
								GL_NEAREST,
								GL_NEAREST,
								GL_LINEAR,
								GL_NEAREST_MIPMAP_NEAREST,
								GL_LINEAR_MIPMAP_NEAREST,
								GL_NEAREST_MIPMAP_LINEAR,
								GL_LINEAR_MIPMAP_LINEAR
						};
const int				OpenGLTexture::Rep::magnifyFilter[] = {
								GL_NEAREST,
								GL_NEAREST,
								GL_LINEAR,
								GL_NEAREST,
								GL_LINEAR,
								GL_NEAREST,
								GL_LINEAR
						};

OpenGLTexture::Rep::Rep(const BzfString& _filename,
								int _width, int _height,
								const void* pixels,
								int _maxFilter,
								bool _repeat,
								int _internalFormat,
								bool adoptPixels) :
								refCount(1),
								width(_width),
								height(_height),
								list(0),
								alpha(false),
								repeat(_repeat),
								internalFormat(_internalFormat),
								filename(_filename),
								maxFilter(_maxFilter)

{
	// get internal format if not provided
	if (internalFormat == 0)
		internalFormat = getBestFormat(width, height, pixels);

	// add me to list
	next = first;
	first = this;

	// note if internal format uses alpha
	switch (internalFormat) {
		case GL_LUMINANCE_ALPHA:
		case GL_LUMINANCE4_ALPHA4:
		case GL_INTENSITY:
		case GL_INTENSITY4:
		case GL_RGBA:
			alpha = true;
			break;

		default:
			alpha = false;
			break;
	}

	// copy the original texture image
	if (adoptPixels) {
		image = const_cast<unsigned char*>(
				reinterpret_cast<const unsigned char*>(pixels));
	}
	else {
		image = new unsigned char[4 * width * height];
		::memcpy(image, pixels, 4 * width * height);
	}

	// watch for context recreation
	OpenGLGState::addContextInitializer(initContext, (void*)this);
}

OpenGLTexture::Rep::~Rep()
{
	// stop watching for context recreation
	OpenGLGState::removeContextInitializer(initContext, (void*)this);

	// done with OpenGL object
	destroyObject();

	// free image data
	delete[] image;

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

void					OpenGLTexture::Rep::setFilter(int filter)
{
	// limit filter.  try to keep nearest... filters as nearest and
	// linear... as linear.
	if (filter > maxFilter) {
		if (filter & 1 == 1)	// nearest...
			if (maxFilter & 1 == 1) filter = maxFilter;
			else filter = maxFilter > 0 ? maxFilter - 1 : 0;

		else					// linear...
			if (maxFilter & 1 == 1) filter = maxFilter - 1;
			else filter = maxFilter;
	}

	glBindTexture(GL_TEXTURE_2D, list);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minifyFilter[filter]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnifyFilter[filter]);
}

void					OpenGLTexture::Rep::createObject()
{
	// do nothing if we've already created the object
	if (list != 0)
		return;

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
	unsigned char* origData = new unsigned char[4 * copyWidth * copyHeight + 4];
	unsigned char* data = (unsigned char*)(((unsigned long)origData & ~3) + 4);
	unsigned char* origScaledData = new unsigned char[4 * scaledWidth * scaledHeight + 4];
	unsigned char* scaledData = (unsigned char*)(((unsigned long)origScaledData & ~3) + 4);
	::memcpy(data, image, 4 * tmpWidth * tmpHeight);

	// now make texture map display list (compute all mipmaps, if requested).
	// compute next mipmap from current mipmap to save time.
	const bool mipmap = (maxFilter > (int)Linear);
	GLint mipmapLevel = 0;

	// make texture map object/list
	glGenTextures(1, &list);
	glBindTexture(GL_TEXTURE_2D, list);

	// set parameters
	setFilter(maxFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
						repeat ? GL_REPEAT : GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
						repeat ? GL_REPEAT : GL_CLAMP);

	// set images
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

	// restore current filter
	setFilter(getFilter());

	// stop defining object
	glBindTexture(GL_TEXTURE_2D, 0);

	// free extra buffers
	delete[] origScaledData;
	delete[] origData;
}

OpenGLTexture::Rep*		OpenGLTexture::Rep::find(
								const BzfString& filename, Rep* prev)
{
	// choose first rep to search from
	Rep* scan;
	if (prev == NULL)
		scan = first;
	else
		scan = prev->next;

	// search
	for (; scan != NULL; scan = scan->next)
		if (scan->filename == filename)
			return scan;
	return NULL;
}

void					OpenGLTexture::Rep::destroyObject()
{
	// free OpenGL texture object
	if (list != 0) {
		glDeleteTextures(1, &list);
		list = 0;
	}
}

void					OpenGLTexture::Rep::initContext(
								bool destroy, void* _self)
{
	Rep* self = reinterpret_cast<Rep*>(_self);
	if (destroy) {
		self->destroyObject();
	}
	else {
		self->createObject();
	}
}


//
// OpenGLTexture
//

OpenGLTexture::Rep*		OpenGLTexture::lastRep = NULL;
OpenGLTexture::Filter	OpenGLTexture::filter = LinearMipmapLinear;

OpenGLTexture::OpenGLTexture()
{
	rep = NULL;
}

OpenGLTexture::OpenGLTexture(const BzfString& filename,
								int* _width, int* _height,
								Filter maxFilter,
								bool repeat,
								int internalFormat)
{
	// see if a suitable texture already exists
	Rep* scan = Rep::find(filename, NULL);
	while (scan != NULL) {
		// make sure filename match matches other parameters
		if (scan->maxFilter == (int)maxFilter && scan->repeat == repeat &&
		(internalFormat == 0 || internalFormat == scan->internalFormat))
			break;

		// search some more
		scan = Rep::find(filename, scan);
	}

	// use existing texture if a match was found
	if (scan != NULL) {
		rep = scan;
		ref();
	}

	// otherwise load the texture image
	else {
		int width, height;
		unsigned char* pixels = MediaFile::readImage(filename, &width, &height);
		if (pixels != NULL) {
			printError("loaded %s", filename.c_str());
			rep = new Rep(filename, width, height, pixels, (int)maxFilter,
								repeat, internalFormat, true);
		}
		else {
			// failed to load image
			rep = NULL;
			printError("cannot find texture %s", filename.c_str());
		}
	}

	// report size
	if (_width != NULL)
		if (rep != NULL)
			*_width = rep->width;
		else
			*_width = 0;
	if (_height != NULL)
		if (rep != NULL)
			*_height = rep->height;
		else
			*_height = 0;
}

OpenGLTexture::OpenGLTexture(int width, int height,
								const void* pixels,
								Filter maxFilter,
								bool repeat,
								int internalFormat,
								bool adoptPixels)
{
	rep = new Rep("", width, height, pixels, (int)maxFilter,
								repeat, internalFormat, adoptPixels);
}

OpenGLTexture::OpenGLTexture(const OpenGLTexture& t)
{
	rep = t.rep;
	ref();
}

OpenGLTexture::~OpenGLTexture()
{
	if (unref())
		delete rep;
}

OpenGLTexture&			OpenGLTexture::operator=(const OpenGLTexture& t)
{
	if (rep != t.rep) {
		if (unref())
			delete rep;
		rep = t.rep;
		ref();
	}
	return *this;
}

OpenGLTexture::Filter	OpenGLTexture::getFilter()
{
	return filter;
}

void					OpenGLTexture::setFilter(Filter _filter)
{
	filter = _filter;

	// change filter on all textures
	for (Rep* scan = Rep::first; scan; scan = scan->next)
		scan->setFilter(filter);

	// back to previously bound texture, or no texture if filter is Off
	if (Rep::first != lastRep && filter != Off)
		bind(lastRep);
	else if (lastRep)
		bind(NULL);
}

bool					OpenGLTexture::operator==(const OpenGLTexture& t) const
{
	return (rep == t.rep);
}

bool					OpenGLTexture::operator!=(const OpenGLTexture& t) const
{
	return (rep != t.rep);
}

bool					OpenGLTexture::operator<(const OpenGLTexture& t) const
{
	if (rep == t.rep)
		return false;
	if (!t.rep)
		return false;
	if (!rep)
		return true;
	return (rep->list < t.rep->list);
}

void					OpenGLTexture::execute() const
{
	bind(rep);
	lastRep = rep;
}

bool					OpenGLTexture::isRepeat() const
{
	return (rep != NULL && rep->repeat);
}

bool					OpenGLTexture::isRGB() const
{
	if (rep == NULL)
		return false;

	// this is somewhat hard to check if the client supplied the
	// internal format, due to the proliferation of those formats.
	// default to claiming RGB since that always works right (in
	// particular, RGB textures work in GL_DECAL mode while
	// alpha, luminance, and intensity textures do not).
	if (rep->internalFormat == GL_INTENSITY)
		return false;
	switch (rep->internalFormat) {
		case GL_LUMINANCE_ALPHA:
		case GL_LUMINANCE:
			return false;
	}
	return true;
}

BzfString				OpenGLTexture::getFilename() const
{
	if (rep == NULL)
		return BzfString();
	else
		return rep->filename;
}

void					OpenGLTexture::ref()
{
	if (rep)
		++(rep->refCount);
}

bool					OpenGLTexture::unref()
{
	return (rep && --(rep->refCount) == 0);
}

void					OpenGLTexture::bind(Rep* r)
{
	// make sure object has been created
	if (r != NULL)
		r->createObject();

	if (r && r->list)
		glBindTexture(GL_TEXTURE_2D, r->list);
	else
		glBindTexture(GL_TEXTURE_2D, 0);
}

int						OpenGLTexture::getRGBAFormat()
{
	return GL_RGBA;
}

int						OpenGLTexture::Rep::getBestFormat(
								int width, int height,
								const void* pixels)
{
	// see if all pixels are achromatic
	const unsigned char* scan = (const unsigned char*)pixels;
	const int size = width * height;
	int i;
	for (i = 0; i < size; scan += 4, i++)
		if (scan[0] != scan[1] || scan[0] != scan[2])
			break;
	const bool useLuminance = (i == size);

	// see if all pixels are opaque
	scan = (const unsigned char*)pixels;
	for (i = 0; i < size; scan += 4, i++)
		if (scan[3] != 0xff)
			break;
	const bool useAlpha = (i != size);

	// see if all pixels are r=g=b=a.  if so return intensity format.
/* some platforms don't support GL_INTENSITY correctly
	bool useIntensity = false;
	if (useLuminance) {
		scan = (const unsigned char*)pixels;
		for (i = 0; i < size; scan += 4, i++)
			if (scan[3] != scan[0])
				break;
		useIntensity = (i == size);
	}
	if (useIntensity)
		return GL_INTENSITY;
*/

	// pick internal format
	return (useLuminance ?
				(useAlpha ? GL_LUMINANCE_ALPHA : GL_LUMINANCE) :
				(useAlpha ? GL_RGBA : GL_RGB));
}
