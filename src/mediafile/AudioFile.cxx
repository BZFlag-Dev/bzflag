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

#include "AudioFile.h"

//
// AudioFile
//

AudioFile::AudioFile(std::istream* _stream) : MediaFile(_stream), open(false)
{
  // do nothing
}

AudioFile::~AudioFile()
{
  // do nothing
}

void		AudioFile::init(int _framesPerSecond, int _numChannels,
				int _numFrames, int _sampWidth)
{
  open	    = true;
  framesPerSecond = _framesPerSecond;
  numChannels     = _numChannels;
  numFrames       = _numFrames;
  sampWidth       = _sampWidth;
}

bool		AudioFile::isOpen() const
{
  return open;
}

int		AudioFile::getFramesPerSecond() const
{
  return framesPerSecond;
}

int		AudioFile::getNumChannels() const
{
  return numChannels;
}

int		AudioFile::getNumFrames() const
{
  return numFrames;
}

int		AudioFile::getSampleWidth() const
{
  return sampWidth;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

