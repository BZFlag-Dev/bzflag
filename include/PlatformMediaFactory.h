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

/* PlatformMediaFactory:
 *	Abstract builder for platform dependent audio/video stuff.
 */

// if NO_AUDIO_THREAD defined then play audio in the same thread as
// the main playing loop.  note that not all platforms support this;
// those platforms will use a separate thread regardless.
//
// some platforms don't context switch well enough for the real
// time demands of audio.  but be aware that running audio in the
// main thread is fraught with peril.
// #define NO_AUDIO_THREAD

#ifndef BZF_PLATFORM_MEDIA_FACTORY_H
#define BZF_PLATFORM_MEDIA_FACTORY_H

#define MPLATFORM (PlatformMediaFactory::getInstance())

#if defined(HALF_RATE_AUDIO)
static const int defaultAudioRate = 11025;
#else
static const int defaultAudioRate = 22050;
#endif

#include "common.h"

class BzfDisplay;
class BzfVisual;
class BzfWindow;

class PlatformMediaFactory {
public:
	PlatformMediaFactory();
	virtual ~PlatformMediaFactory();

	// create stuff for display
	virtual BzfDisplay*	createDisplay(const char* name,
							const char* videoFormat) = 0;
	virtual BzfVisual*	createVisual(const BzfDisplay*) = 0;
	virtual BzfWindow*	createWindow(const BzfDisplay*, BzfVisual*) = 0;

	// initialize the audio subsystem.  return true iff successful.
	virtual bool		openAudio() = 0;

	// close the audio subsystem
	virtual void		closeAudio() = 0;

	// start a thread for audio processing and call proc in that thread.
	// data is passed to the proc function.  return true iff the thread
	// was started successfully.
	virtual bool		startAudioThread(void (*proc)(void*), void* data) = 0;

	// stop the audio thread
	virtual void		stopAudioThread() = 0;

	// returns true if audio is running in a separate thread
	virtual bool		hasAudioThread() const = 0;

	// append a command to the sound effect command queue
	virtual void		writeSoundCommand(const void*, int length) = 0;

	// read the next command from the sound effect command queue.
	// return immediately with false if no command is ready.
	// otherwise read the command and return true if successful.
	virtual bool		readSoundCommand(void*, int length) = 0;

	// returns the output rate (in frames per second);
	virtual int			getAudioOutputRate() const = 0;

	// returns the number of frames in the whole audio buffer
	virtual int			getAudioBufferSize() const = 0;

	// returns the number of frames in each chunk of the audio buffer
	virtual int			getAudioBufferChunkSize() const = 0;

	// return true iff the audio buffer is getting too low
	virtual bool		isAudioTooEmpty() const = 0;

	// append sound samples to end of audio output buffer.  this
	// method should return immediately, if possible.
	virtual void		writeAudioFrames(const float* samples,
							int numFrames) = 0;

	// wait for the sound buffer to empty to the low water mark or
	// until a sound effect command is pending or until maxTime
	// seconds have passed.  if maxTime < 0, then do not timeout.
	// if !checkLowWater then don't check the low water mark.
	virtual void		audioSleep(bool checkLowWater,
							double maxTime = -1.0) = 0;

	static PlatformMediaFactory*	getInstance();

private:
	PlatformMediaFactory(const PlatformMediaFactory&);
	PlatformMediaFactory& operator=(const PlatformMediaFactory&);

private:
	static PlatformMediaFactory*	instance;
};

#endif // BZF_PLATFORM_MEDIA_FACTORY_H
// ex: shiftwidth=4 tabstop=4
