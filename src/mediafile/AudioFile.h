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

#ifndef BZF_AUDIO_FILE_H
#define BZF_AUDIO_FILE_H

#include "MediaFile.h"

class AudioFile : public MediaFile {
public:
	// close the audio file.  this does *not* destroy the stream.
	virtual ~AudioFile();

	// note -- all concrete AudioFile types should have a method to
	// return the default file extension for files in the format:
	// static std::string getExtension();

	// read more sample data into buffer.  samples are in left,right
	// order for stereo files.  samples are in host byte order.
	// client must not attempt to read past last sample.  returns
	// false on error.  (a frame is one sample for each channel.)
	// buffer must be numFrames * getNumChannels() * getSampleWidth()
	// bytes at least.
	virtual bool		read(void* buffer, int numFrames) = 0;

	// returns true if the stream was successfully opened as an audio file
	bool				isOpen() const;

	// get information about the audio file.  a frame is one sample
	// for each channel.  sample width is in bytes
	int					getFramesPerSecond() const;
	int					getNumChannels() const;
	int					getNumFrames() const;
	int					getSampleWidth() const;

protected:
	AudioFile(std::istream*);

	// save info about the stream.  called by the derived c'tor.
	// don't call this if the stream is not an audio file.
	void				init(int framesPerSecond, int numChannels,
							int numFrames, int sampWidth);

private:
	bool				open;
	int					framesPerSecond;
	int					numChannels;
	int					numFrames;
	int					sampWidth;
};

#endif

