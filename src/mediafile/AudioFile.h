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

#ifndef BZF_AUDIO_FILE_H
	#define BZF_AUDIO_FILE_H

	#include "MediaFile.h"


/** This ABC represents an audio file. It has subclasses for different audio
formats. */
class AudioFile: public MediaFile
{
public:
	/** Close the audio file. This does *not* destroy the stream. */
	virtual ~AudioFile();

	// note -- all concrete AudioFile types should have a method to
	// return the default file extension for files in the format:
	// static std::string getExtension();

	/** Read more sample data into buffer. Samples are in left, right
	order for stereo files. Samples are in host byte order.
	Client must not attempt to read past last sample. Returns
	false on error. A frame is one sample for each channel.
	@c buffer must be @c numFrames * @c getNumChannels() *
	@c getSampleWidth() bytes at least. */
	virtual bool read( void *buffer, int numFrames ) = 0;

	/** Returns true if the stream was successfully opened as an audio file. */
	bool isOpen()const;

	/** Get the frame rate of the audio file. A frame is one sample
	for each channel. */
	int getFramesPerSecond()const;

	/** Get the number of channels in the audio file. */
	int getNumChannels()const;

	/** Get the number of frames in the audio file. A frame is one sample
	for each channel. */
	int getNumFrames()const;

	/** Get the sample width of the audio file, in bytes. */
	int getSampleWidth()const;

protected:
	AudioFile( std::istream* );

	// save info about the stream.  called by the derived c'tor.
	// don't call this if the stream is not an audio file.
	void init( int framesPerSecond, int numChannels, int numFrames, int sampWidth );

private:
	bool open;
	int framesPerSecond;
	int numChannels;
	int numFrames;
	int sampWidth;
};

#endif 


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
