/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_OGG_AUDIO_FILE_H
#define BZF_OGG_AUDIO_FILE_H

#include "AudioFile.h"
#include <vorbis/vorbisfile.h>
#include <vorbis/codec.h>

struct OAFInputBundle {
  std::istream*		input;
  std::streamoff	length;
};

/** This class represents an Ogg Vorbis audio file. It implements the read()
    function from AudioFile. */
class OggAudioFile : public AudioFile {
public:
  OggAudioFile(std::istream*);
  virtual ~OggAudioFile();

  /** This class returns the default extension of Ogg Vorbis files. */
  static std::string	getExtension();

  /** This function reads data from an Ogg Vorbis file. */
  virtual bool		read(void* buffer, int numFrames);

protected:
  OggVorbis_File	file;
  vorbis_info*		info;
  int			stream;
};

size_t	OAFRead(void* ptr, size_t size, size_t nmemb, void* datasource);
int	OAFSeek(void* datasource, ogg_int64_t offset, int whence);
int	OAFClose(void* datasource);
long	OAFTell(void* datasource);

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

