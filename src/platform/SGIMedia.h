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

/* SGIMedia:
 *	Media I/O on SGI Irix.
 */

#ifndef BZF_SGIMEDIA_H
#define	BZF_SGIMEDIA_H

#include "BzfMedia.h"
#include <dmedia/audio.h>

class SGIMedia : public BzfMedia {
  public:
			SGIMedia();
			~SGIMedia();

    double		stopwatch(boolean);
    void		sleep(float);
    boolean		openAudio();
    void		closeAudio();
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

  private:
    boolean		audioReady;
    long		originalAudioParams[8];
    long		audioParams[6];
    long		audioOutputRate;
    long		audioBufferSize;
    long		audioLowWaterMark;
    int			maxFd;
    ALport		audioPort;
    int			audioPortFd;
    int			queueIn, queueOut;
    short*		outputBuffer;
    int			childProcID;
    double		secondsPerTick;
    unsigned int	stopwatchTime;
    volatile unsigned int* iotimer_addr;
    static void		(*threadProc)(void*);
    static void*	threadData;
};

#endif // BZF_SGIMEDIA_H
// ex: shiftwidth=2 tabstop=8
