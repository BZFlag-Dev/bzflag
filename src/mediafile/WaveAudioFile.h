/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_WAVE_AUDIO_FILE_H
#define BZF_WAVE_AUDIO_FILE_H

#include "AudioFile.h"

class WaveAudioFile : public AudioFile {
public:
  WaveAudioFile(std::istream*);
  virtual ~WaveAudioFile();

  static std::string	getExtension();

  // AudioFile overrides
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

