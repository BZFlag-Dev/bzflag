/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __BZFMEDIA_H__
#define	__BZFMEDIA_H__

#include "common.h"

#include <string>
#include <stdio.h>

static const std::string	DEFAULT_MEDIA_DIR = "data";

/** BzfMedia is a helper class that will read in audio and image
 * data files.  It's an abstract, platform independant base for
 * media I/O.
 *
 * if HALF_RATE_AUDIO defined then use half the normal audio sample
 * rate (and downsample the audio files to match).  this reduces the
 * demands on the system.
 * #define HALF_RATE_AUDIO
 *
 * if NO_AUDIO_THREAD defined then play audio in the same thread as
 * the main playing loop.  note that not all platforms support this;
 * those platforms will use a separate thread regardless.
 *
 * some platforms don't context switch well enough for the real
 * time demands of audio.  but be aware that running audio in the
 * main thread is fraught with peril.
 * #define NO_AUDIO_THREAD
 */
class BzfMedia {
  public:
			BzfMedia();
    virtual		~BzfMedia();

    // get and set default directory to look for media files in
    std::string		getMediaDirectory() const;
    void		setMediaDirectory(const std::string&);

    // sounds are stored as left, right, left, right ..., values are
    // in the range -1 to 1.  numFrames returns the number of left,right
    // pairs.  rate is in frames per second.  use delete[] to release
    // the returned memory.
    float*		readSound(const std::string& filename,
				int& numFrames, int& rate) const;

    // sleep for given number of seconds
    virtual double	stopwatch(bool start);

    // initialize the audio subsystem.  return true iff successful.
    virtual bool	openAudio() = 0;

    // close the audio subsystem
    virtual void	closeAudio() = 0;

    // start a thread for audio processing and call proc in that thread.
    // data is passed to the proc function.  return true iff the thread
    // was started successfully.
    virtual bool	startAudioThread(void (*proc)(void*), void* data) = 0;

    // stop the audio thread
    virtual void	stopAudioThread() = 0;

    // returns true if audio is running in a separate thread
    virtual bool	hasAudioThread() const = 0;

    // register a callback for audio processing. The passed procedure will be
    // called whenever audio needs to be filled
    virtual void	startAudioCallback(bool (*)(void)) {};

    // returns true if audio is running via callback
    virtual bool	hasAudioCallback() const {return false;};

    // append a command to the sound effect command queue
    virtual void	writeSoundCommand(const void*, int length) = 0;

    // read the next command from the sound effect command queue.
    // return immediately with false if no command is ready.
    // otherwise read the command and return true if successful.
    virtual bool	readSoundCommand(void*, int length) = 0;

    // returns the output rate (in frames per second);
    virtual int		getAudioOutputRate() const = 0;

    // returns the number of frames in the whole audio buffer
    virtual int		getAudioBufferSize() const = 0;

    // returns the number of frames in each chunk of the audio buffer
    virtual int		getAudioBufferChunkSize() const = 0;

    // return true iff the audio buffer is getting too low
    virtual bool	isAudioTooEmpty() const = 0;

    // append sound samples to end of audio output buffer.  this
    // method should return immediately, if possible.
    virtual void	writeAudioFrames(const float* samples,
				int numFrames) = 0;

    // wait for the sound buffer to empty to the low water mark or
    // until a sound effect command is pending or until maxTime
    // seconds have passed.  if maxTime < 0, then do not timeout.
    // if !checkLowWater then don't check the low water mark.
    virtual void	audioSleep(bool checkLowWater,
				double maxTime = -1.0) = 0;

    virtual void	setDriver(std::string driverName);
    virtual void	setDevice(std::string deviceName);
    virtual void	audioDriver(std::string& driverName);

  protected:
    // return default extensions for image and sound files
    virtual std::string	getImageExtension() const;
    virtual std::string	getSoundExtension() const;

    // return NULL on failure
    virtual float*	doReadSound(const std::string& filename,
				int& numFrames, int& rate) const;

    // concatenate directory to filename
    virtual std::string	makePath(const std::string& dir,
				const std::string& filename) const;

    // replace (or add) extension to pathname
    std::string		replaceExtension(const std::string& pathname,
				const std::string& extension) const;

    // return the position of the extension separator, or zero if
    // it cannot be found.
    virtual int		findExtension(const std::string& pathname) const;

  private:
    static int16_t	getShort(const void*);
    static uint16_t	getUShort(const void*);
    static int32_t	getLong(const void*);
    static bool		doReadVerbatim(FILE*, int, int, int,
				unsigned char*);
    static bool		doReadRLE(FILE*, int, int, int,
				unsigned char*);

  private:
    std::string		mediaDir;
};

#endif // __BZFMEDIA_H__

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

