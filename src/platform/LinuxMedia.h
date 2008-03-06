/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* LinuxMedia:
 *	Media I/O on Linux
 */

#ifndef BZF_LINUXMEDIA_H
#define	BZF_LINUXMEDIA_H

#include "BzfMedia.h"
#include <stdio.h>
#include <unistd.h>

class LinuxMedia : public BzfMedia {
  public:
			LinuxMedia();
			~LinuxMedia();

    double		stopwatch(bool);
    bool		openAudio();
    void		closeAudio();
    bool		startAudioThread(void (*)(void*), void*);
    void		stopAudioThread();
    bool		hasAudioThread() const;
    void		writeSoundCommand(const void*, int);
    bool		readSoundCommand(void*, int);
    int			getAudioOutputRate() const;
    int			getAudioBufferSize() const;
    int			getAudioBufferChunkSize() const;
    bool		isAudioTooEmpty() const;
    void		writeAudioFrames(const float* samples, int numFrames);
    void		audioSleep(bool checkLowWater, double maxTime);

  private:
    bool		checkForAudioHardware();
    bool		openAudioHardware();
    bool		openIoctl(int cmd, void* value, bool req = true);
    static void		audioThreadInit(void*);

    void		writeAudioFrames8Bit(
				const float* samples, int numFrames);
    void		writeAudioFrames16Bit(
				const float* samples, int numFrames);

    static double	getTime();

  private:
    bool		audioReady;
    int			audioOutputRate;
    int			audioBufferSize;
    int			audioLowWaterMark;
    int			maxFd;
    int			audioPortFd;
    int			queueIn, queueOut;
    short*		outputBuffer;
    pid_t		childProcID;
    double		stopwatchTime;
    bool		audio8Bit;

    bool		noSetFragment;
    bool		getospaceBroken;
    int			chunksPending;
    double		chunkTime;
    double		chunksPerSecond;
};

#endif // BZF_LINUXMEDIA_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

