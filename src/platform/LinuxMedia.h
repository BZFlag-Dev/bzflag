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

    double		stopwatch(boolean);
    void		sleep(float);
    boolean		openAudio();
    void		closeAudio();
    boolean		isAudioBrainDead() const;
    boolean		startAudioThread(void (*)(void*), void*);
    void		stopAudioThread();
    boolean		hasAudioThread() const;
    void		writeSoundCommand(const void*, int);
    boolean		readSoundCommand(void*, int);
    int			getAudioOutputRate() const;
    int			getAudioBufferSize() const;
    int			getAudioBufferChunkSize() const;
    boolean		isAudioTooEmpty() const;
    void		writeAudioFrames(const float* samples, int numFrames);
    void		audioSleep(boolean checkLowWater, double maxTime);

  private:
    boolean		checkForAudioHardware();
    boolean		openAudioHardware();
    static void		audioThreadInit(void*);

    void		writeAudioFrames8Bit(
				const float* samples, int numFrames);
    void		writeAudioFrames16Bit(
				const float* samples, int numFrames);

  private:
    boolean		audioReady;
    int		        audioOutputRate;
    int			audioBufferSize;
    int			audioLowWaterMark;
    int			maxFd;
    int			audioPortFd;
    int			queueIn, queueOut;
    short*		outputBuffer;
    pid_t		childProcID;
    double		stopwatchTime;
    boolean		audio8Bit;
};

#endif // BZF_LINUXMEDIA_H
