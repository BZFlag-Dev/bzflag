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
    static void		audioThreadInit(void*);

  private:
    bool		audioReady;
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

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
