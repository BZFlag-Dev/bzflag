/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* BzfMedia:
 *	Abstract, platform independent base for media I/O.
 */

#ifndef BZF_MEDIA_H
#define	BZF_MEDIA_H

#include "common.h"
#include "BzfString.h"
#include <stdio.h>

// if HALF_RATE_AUDIO defined then use half the normal audio sample
// rate (and downsample the audio files to match).  this reduces the
// demands on the system.
// #define HALF_RATE_AUDIO

// if NO_AUDIO_THREAD defined then play audio in the same thread as
// the main playing loop.  note that not all platforms support this;
// those platforms will use a separate thread regardless.
//
// some platforms don't context switch well enough for the real
// time demands of audio.
// #define NO_AUDIO_THREAD

class BzfMedia {
  public:
			BzfMedia();
    virtual		~BzfMedia();

    // get and set default directory to look for media files in
    BzfString		getMediaDirectory() const;
    void		setMediaDirectory(const BzfString&);

    // images are stored RGBARGBA..., left to right, bottom to top.
    // depth indicates how many channels were in the stored image.
    // use delete[] to release the returned memory.
    unsigned char*	readImage(const BzfString& filename,
				int& width, int& height, int& depth) const;

    // sounds are stored as left, right, left, right ..., values are
    // in the range -1 to 1.  numFrames returns the number of left,right
    // pairs.  rate is in frames per second.  use delete[] to release
    // the returned memory.
    float*		readSound(const BzfString& filename,
				int& numFrames, int& rate) const;

    // sleep for given number of seconds
    virtual double	stopwatch(boolean start);

    // sleep for given number of seconds
    virtual void	sleep(float timeInSeconds) = 0;

    // initialize the audio subsystem.  return true iff successful.
    virtual boolean	openAudio() = 0;

    // close the audio subsystem
    virtual void	closeAudio() = 0;

    // returns true iff audio keeps playing output (ring) buffer
    // after last written sample.
    virtual boolean	isAudioBrainDead() const = 0;

    // start a thread for audio processing and call proc in that thread.
    // data is passed to the proc function.  return true iff the thread
    // was started successfully.
    virtual boolean	startAudioThread(void (*proc)(void*), void* data) = 0;

    // stop the audio thread
    virtual void	stopAudioThread() = 0;

    // returns true if audio is running in a separate thread
    virtual boolean	hasAudioThread() const = 0;

    // append a command to the sound effect command queue
    virtual void	writeSoundCommand(const void*, int length) = 0;

    // read the next command from the sound effect command queue.
    // return immediately with false if no command is ready.
    // otherwise read the command and return true if successful.
    virtual boolean	readSoundCommand(void*, int length) = 0;

    // returns the output rate (in frames per second);
    virtual int		getAudioOutputRate() const = 0;

    // returns the number of frames in the whole audio buffer
    virtual int		getAudioBufferSize() const = 0;

    // returns the number of frames in each chunk of the audio buffer
    virtual int		getAudioBufferChunkSize() const = 0;

    // return true iff the audio buffer is getting too low
    virtual boolean	isAudioTooEmpty() const = 0;

    // append sound samples to end of audio output buffer.  this
    // method should return immediately, if possible.
    virtual void	writeAudioFrames(const float* samples,
				int numFrames) = 0;

    // wait for the sound buffer to empty to the low water mark or
    // until a sound effect command is pending or until maxTime
    // seconds have passed.  if maxTime < 0, then do not timeout.
    // if !checkLowWater then don't check the low water mark.
    virtual void	audioSleep(boolean checkLowWater,
				double maxTime = -1.0) = 0;

  protected:
    // return default extensions for image and sound files
    virtual BzfString	getImageExtension() const;
    virtual BzfString	getSoundExtension() const;

    // return NULL on failure
    virtual unsigned char* doReadImage(const char* filename,
				int& width, int& height, int& depth) const;

    // return NULL on failure
    virtual float*	doReadSound(const char* filename,
				int& numFrames, int& rate) const;

    // concatenate directory to filename
    virtual BzfString	makePath(const BzfString& dir,
				const BzfString& filename) const;

    // replace (or add) extension to pathname
    BzfString		replaceExtension(const BzfString& pathname,
				const BzfString& extension) const;

    // return the position of the extension separator, or zero if
    // it cannot be found.
    virtual int		findExtension(const BzfString& pathname) const;

  private:
    static int16_t	getShort(const void*);
    static uint16_t	getUShort(const void*);
    static int32_t	getLong(const void*);
    static boolean	doReadVerbatim(FILE*, int, int, int,
				unsigned char*);
    static boolean	doReadRLE(FILE*, int, int, int,
				unsigned char*);

  private:
    BzfString		mediaDir;
};

#endif // BZF_MEDIA_H
