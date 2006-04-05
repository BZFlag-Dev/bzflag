/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
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
// Disable SDL_image, support for now
//
#undef HAVE_SDL_IMAGE


//
// MediaFile
//

/******************************************************************************/
#if defined(HAVE_SDL) && defined(HAVE_SDL_IMAGE)
/******************************************************************************/


#include <SDL/SDL_image.h>
#include "StateDatabase.h"
#include "FileManager.h"

#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#  include <unistd.h>
#endif

static bool fileExists (const std::string& name)
{
  struct stat buf;
#ifndef _WIN32
  return (stat(name.c_str(), &buf) == 0);
#else
  // Windows sucks yet again, if there is a trailing  "\"
  // at the end of the filename, _stat will return -1.
  std::string dirname = name;
  while (dirname.find_last_of('\\') == (dirname.size() - 1)) {
    dirname.resize(dirname.size() - 1);
  }
  return (_stat(dirname.c_str(), (struct _stat *) &buf) == 0);
#endif
}  


static bool checkExt(const std::string& base, const char* ext,
                     std::string& result)
{
#ifndef _WIN32
  const std::string sep = "/";
#else
  const std::string sep = "\\";
#endif

  result = base;
  result += ext;
  const std::string filename = result;
  
  const bool relative = !FILEMGR.isAbsolute(filename);

  if (relative) {
    // try directory stored in DB
    if (BZDB.isSet("directory")) {
      result = BZDB.get("directory") + sep + filename;
      if (fileExists(result)) return true;
    }

    // try data directory
    result = "data" + sep + filename;
    if (fileExists(result)) return true;
  }
  
  // try current directory (or absolute path)
  result = filename;
  if (fileExists(result)) return true;

#if defined(INSTALL_DATA_DIR)
  // try install directory
  if (relative) {
    result = INSTALL_DATA_DIR + sep + filename;
    if (fileExists(result)) return true;
  }
#endif

  return false;
}


/******************************************************************************/

// From the header...
// - Use delete[] to release the returned image.
// - Returns NULL on failure. 
// - Images are stored RGBA, left to right, bottom to top.

unsigned char* MediaFile::readImage(std::string filename, int* w, int* h)
{
  // get the absolute filename for cache textures
  if (CACHEMGR.isCacheFileType(filename)) {
    filename = CACHEMGR.getLocalName(filename);
  }

#ifdef WIN32
  // cheat and make sure the file is a windows file path
  ConvertPath(filename);
#endif //WIN32

  // try looking in different directories and appending
  // supported extensions to find the source image file
  std::string name;      
  if (!checkExt(filename, "",     name) && 
      !checkExt(filename, ".png", name) &&
      !checkExt(filename, ".gif", name) &&
      !checkExt(filename, ".bmp", name) &&
      !checkExt(filename, ".pcx", name) &&
      !checkExt(filename, ".tga", name) &&
      !checkExt(filename, ".xpm", name) &&
      !checkExt(filename, ".pbm", name) &&
      !checkExt(filename, ".pgm", name) &&
      !checkExt(filename, ".ppm", name) &&
      !checkExt(filename, ".jpg", name) &&
      !checkExt(filename, ".jpeg", name) &&
      !checkExt(filename, ".tif", name) &&
      !checkExt(filename, ".tiff", name)) {
    DEBUG3("SDL_image: could not find %s\n", filename.c_str());
    return NULL;
  }

  SDL_Surface* surface;
  surface = IMG_Load(name.c_str());
  if (surface == NULL) {
    DEBUG3("SDL_image: could not load %s\n", name.c_str());
    return NULL;
  }

  // save for debugging
  const int origWidth = surface->w;
  const int origHeight = surface->h;
  const int origBpp = surface->format->BitsPerPixel;

  // convert the format
  SDL_PixelFormat fmt;
  fmt.palette = NULL;
  fmt.BitsPerPixel = 32;
  fmt.BytesPerPixel = 4;
  fmt.Rloss = fmt.Gloss = fmt.Bloss = fmt.Aloss = 0;
  fmt.Rshift = fmt.Gshift = fmt.Bshift = fmt.Ashift = 0;
  fmt.colorkey = 0;
  fmt.alpha = 0;
  // handle endianess
  fmt.Rmask = fmt.Gmask = fmt.Bmask = fmt.Amask = 0;
  ((unsigned char*)&fmt.Rmask)[0] = 0xff;
  ((unsigned char*)&fmt.Gmask)[1] = 0xff;
  ((unsigned char*)&fmt.Bmask)[2] = 0xff;
  ((unsigned char*)&fmt.Amask)[3] = 0xff;
  
  SDL_Surface* rgba = SDL_ConvertSurface(surface, &fmt, SDL_SWSURFACE);
  SDL_FreeSurface(surface);

  // bail if the conversion failed
  if (rgba == NULL) {
    DEBUG3("SDL_Image: rgba conversion failed: %s: %dx%d %dbpp\n",
           name.c_str(), origWidth, origHeight, origBpp);
    return NULL;
  }

  // copy the parameters
  *w = rgba->w;
  *h = rgba->h;

  // copy the memory, with a vertical flip
  const int rowlen = (rgba->w * 4);
  const int imageSize = (rowlen * rgba->h);
  unsigned char* image = new unsigned char[imageSize];
  const unsigned char* source = (unsigned char*) rgba->pixels;
  for (int i = 0; i < rgba->h; i++) {
    memcpy(image + (rowlen * i),
           source + (rowlen * (rgba->h - 1 - i)),
           rowlen);
  }

  DEBUG3("SDL_Image: loaded %s: %dx%d %dbpp\n",
         name.c_str(), origWidth, origHeight, origBpp);

  SDL_FreeSurface(rgba);

  return image;
}


/******************************************************************************/
#else // ! (HAVE_SDL && HAVE_SDL_IMAGE)
/******************************************************************************/


MediaFile::MediaFile(std::istream* _stream) : stream(_stream)
{
  // do nothing
}

MediaFile::~MediaFile()
{
  // do nothing
}

bool		MediaFile::isOkay() const
{
  return (stream != NULL && stream->good());
}

void		MediaFile::readRaw(void* vbuffer, uint32_t bytes)
{
  char* buffer = reinterpret_cast<char*>(vbuffer);
  stream->read(buffer, bytes);
}

void		MediaFile::skip(uint32_t bytes)
{
  stream->ignore(bytes);
}

uint16_t	MediaFile::read16LE()
{
  uint16_t b;
  stream->read(reinterpret_cast<char*>(&b), sizeof(b));
  return swap16LE(&b);
}

uint16_t	MediaFile::read16BE()
{
  uint16_t b;
  stream->read(reinterpret_cast<char*>(&b), sizeof(b));
  return swap16BE(&b);
}

uint32_t	MediaFile::read32LE()
{
  uint32_t b;
  stream->read(reinterpret_cast<char*>(&b), sizeof(b));
  return swap32LE(&b);
}

uint32_t	MediaFile::read32BE()
{
  uint32_t b;
  stream->read(reinterpret_cast<char*>(&b), sizeof(b));
  return swap32BE(&b);
}

uint16_t	MediaFile::swap16LE(uint16_t* d)
{
  unsigned char* b = reinterpret_cast<unsigned char*>(d);
  *d = static_cast<uint16_t>(b[0]) + (static_cast<uint16_t>(b[1]) << 8);
  return *d;
}

uint16_t	MediaFile::swap16BE(uint16_t* d)
{
  unsigned char* b = reinterpret_cast<unsigned char*>(d);
  *d = static_cast<uint16_t>(b[1]) + (static_cast<uint16_t>(b[0]) << 8);
  return *d;
}

uint32_t	MediaFile::swap32LE(uint32_t* d)
{
  unsigned char* b = reinterpret_cast<unsigned char*>(d);
  *d = static_cast<uint32_t>(b[0]) + (static_cast<uint32_t>(b[1]) << 8) +
       (static_cast<uint32_t>(b[2]) << 16) +
       (static_cast<uint32_t>(b[3]) << 24);
  return *d;
}

uint32_t	MediaFile::swap32BE(uint32_t* d)
{
  unsigned char* b = reinterpret_cast<unsigned char*>(d);
  *d = static_cast<uint32_t>(b[3]) + (static_cast<uint32_t>(b[2]) << 8) +
       (static_cast<uint32_t>(b[1]) << 16) +
       (static_cast<uint32_t>(b[0]) << 24);
  return *d;
}

//
// utility methods to read various media files in any supported format
//

#include "FileManager.h"
#include "SGIImageFile.h"
#include "PNGImageFile.h"
#include "WaveAudioFile.h"

#define OPENMEDIA(_T)					\
do {							\
  stream = FILEMGR.createDataInStream(filename, true);	\
  if (stream == NULL)					\
    stream = FILEMGR.createDataInStream(filename +	\
	     _T::getExtension(), true);			\
  if (stream != NULL) {					\
    file = new _T(stream);				\
    if (!file->isOpen()) {				\
      file = NULL;					\
      delete stream;					\
      stream = NULL;					\
    }							\
  }							\
} while (0)

unsigned char*		MediaFile::readImage(
				std::string filename,
				int* width, int* height)
{
  // get the absolute filename for cache textures
  if (CACHEMGR.isCacheFileType(filename)) {
    filename = CACHEMGR.getLocalName(filename);
  }
  
#ifdef WIN32
  // cheat and make sure the file is a windows file path
  ConvertPath(filename);
#endif //WIN32

  // try opening file as an image
  std::istream* stream;
  ImageFile* file = NULL;
  if (file == NULL)
    OPENMEDIA(PNGImageFile);
  if (file == NULL)
    OPENMEDIA(SGIImageFile);

  // read the image
  unsigned char* image = NULL;
  if (file != NULL) {
    // get the image size
    int dx = *width  = file->getWidth();
    int dy = *height = file->getHeight();
    int dz =	   file->getNumChannels();

    // make buffer for final image
    image = new unsigned char[dx * dy * 4];

    // make buffer to read image.  if the image file has 4 channels
    // then read directly into the final image buffer.
    unsigned char* buffer = (dz == 4) ? image : new unsigned char[dx * dy * dz];

    // read the image
    if (image != NULL && buffer != NULL) {
      if (!file->read(buffer)) {
	// failed to read image.  clean up.
	if (buffer != image)
	  delete[] buffer;
	delete[] image;
	image  = NULL;
	buffer = NULL;
      } else {
	// expand image into 4 channels
	int n = dx * dy;
	const unsigned char* src = buffer;
	unsigned char* dst = image;
	if (dz == 1) {
	  // r=g=b=i, a=max
	  for (; n > 0; --n) {
	    dst[0] = dst[1] = dst[2] = src[0];
	    dst[3] = 0xff;
	    src += 1;
	    dst += 4;
	  }
	} else if (dz == 2) {
	  // r=g=b=i
	  for (; n > 0; --n) {
	    dst[0] = dst[1] = dst[2] = src[0];
	    dst[3] = src[1];
	    src += 2;
	    dst += 4;
	  }
	} else if (dz == 3) {
	  // a=max
	  for (; n > 0; --n) {
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


/******************************************************************************/
#endif // (HAVE_SDL && HAVE_SDL_IMAGE)
/******************************************************************************/


/*
float*		MediaFile::readSound(
			const std::string& filename,
			int* _numFrames, int* rate)
{
  // try opening as an audio file
  std::istream* stream;
  AudioFile* file = NULL;
  if (file == NULL)
    OPENMEDIA(WaveAudioFile);

  // read the audio
  float* audio = NULL;
  if (file != NULL) {
    // get the audio info
    *rate	   = file->getFramesPerSecond();
    int numChannels = file->getNumChannels();
    int numFrames   = file->getNumFrames();
    int sampleWidth = file->getSampleWidth();

    // make a buffer to read into
    unsigned char* raw = new unsigned char[numFrames * numChannels * sampleWidth];

    // read raw samples
    if (file->read(raw, numFrames)) {
      // prepare conversion parameters
      int step   = 1;
#if defined(HALF_RATE_AUDIO)
      step      *= 2;
      numFrames /= 2;
      *rate     /= 2;
#endif

      // make final audio buffer
      audio = new float[2 * numFrames];

      // convert
      if (numChannels == 1) {
	if (sampleWidth == 1) {
	  signed char* src = reinterpret_cast<signed char*>(raw);
	  for (int i = 0; i < numFrames; ++i) {
	    audio[2 * i] = audio[2 * i + 1] = 257.0f * static_cast<float>(*src);
	    src += step;
	  }
	} else {
	  int16_t* src = reinterpret_cast<int16_t*>(raw);
	  for (int i = 0; i < numFrames; ++i) {
	    audio[2 * i] = audio[2 * i + 1] = static_cast<float>(*src);
	    src += step;
	  }
	}
      } else {
	step *= 2;
	if (sampleWidth == 1) {
	  signed char* src = reinterpret_cast<signed char*>(raw);
	  for (int i = 0; i < numFrames; ++i) {
	    audio[2 * i]     = 257.0f * static_cast<float>(src[0]);
	    audio[2 * i + 1] = 257.0f * static_cast<float>(src[1]);
	    src += step;
	  }
	} else {
	  int16_t* src = reinterpret_cast<int16_t*>(raw);
	  for (int i = 0; i < numFrames; ++i) {
	    audio[2 * i]     = static_cast<float>(src[0]);
	    audio[2 * i + 1] = static_cast<float>(src[1]);
	    src += step;
	  }
	}
      }
    }

    // clean up
    delete[] raw;
    delete file;
    *_numFrames = numFrames;
  }

  // clean up
  delete stream;

  return audio;
}
*/


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

