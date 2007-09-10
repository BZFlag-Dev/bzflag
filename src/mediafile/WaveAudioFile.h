/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_WAVE_AUDIO_FILE_H
#define BZF_WAVE_AUDIO_FILE_H

#include "AudioFile.h"

/** This class represents a WAVE file. It implements the read() function from
    AudioFile. */
class WaveAudioFile : public AudioFile {
public:
  WaveAudioFile(std::istream*);
  virtual ~WaveAudioFile();

  /** This function returns the default extension of WAVE audio files. */
  static std::string	getExtension();

  /** This function reads data from a WAVE file. */
  virtual bool		read(void* buffer, int numFrames);

protected:
  bool			readHeader(char* tag, uint32_t* length);
  bool			findChunk(const char* tag, uint32_t* length);
};

#endif


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

