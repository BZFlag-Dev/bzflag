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

#ifndef BZF_MEDIA_FILE_H
#define BZF_MEDIA_FILE_H

#include "common.h"

/* system interface headers */
#include <string>

/* common interface headers */
#include "bzfio.h"


// if HALF_RATE_AUDIO defined then use half the normal audio sample
// rate (and downsample the audio files to match).  this reduces the
// demands on the system.
// #define HALF_RATE_AUDIO


/** This class is a base class for media files, which can be image files or
    audio files. */
class MediaFile {
public:
  /** Close the media file.  This does *not* destroy the stream. */
  virtual ~MediaFile();

  // read a sound file.  use delete[] to release the returned
  // audio.  returns NULL on failure.  sounds are stored
  // left/right.
  //	static float*		readSound(const std::string& filename,
  //							int* numFrames, int* rate);

protected:
  MediaFile(std::istream*);

  /** Get the stream. */
  std::istream* getStream() const { return stream; }

  /** Return true if the stream is in a readable state. */
  bool isOkay() const;

  /** Utility method to read raw data. */
  void readRaw(void* buffer, uint32_t bytes);

  /** Utility method to skip data. */
  void skip(uint32_t bytes);

  /** Utility method to read a 2 byte little-endian number into host byte
      order. */
  uint16_t read16LE();
  /** Utility method to read a 2 byte big-endian number into host byte
      order. */
  uint16_t read16BE();
  /** Utility method to read a 4 byte little-endian number into host byte
      order. */
  uint32_t read32LE();
  /** Utility method to read a 4 byte big-endian number into host byte
      order. */
  uint32_t read32BE();

  /** Utility method to byte swap a little-endian 2 byte number in place
      into host byte order. Returns the byte swapped data. */
  static uint16_t swap16LE(uint16_t*);

  /** Utility method to byte swap a big-endian 2 byte number in place into
      host byte order. Returns the byte swapped data. */
  static uint16_t swap16BE(uint16_t*);

  /** Utility method to byte swap a little-endian 4 byte number in place
      into host byte order. Returns the byte swapped data. */
  static uint32_t swap32LE(uint32_t*);

  /** Utility method to byte swap a big-endian 4 byte number in place into
      host byte order. Returns the byte swapped data. */
  static uint32_t swap32BE(uint32_t*);

private:
  std::istream*			stream;
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

