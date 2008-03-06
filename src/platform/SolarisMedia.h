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

/* SolarisMedia:
 *	Media I/O on Solaris
 */

#ifndef BZF_SOLARISMEDIA_H
#define	BZF_SOLARISMEDIA_H

#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include "bzsignal.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/audioio.h>
#include <sys/stropts.h>

#include "BzfMedia.h"

class SolarisMedia : public BzfMedia {
  public:
			SolarisMedia();
			~SolarisMedia();

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

    int			maxFd;
    int			audio_fd;
    int			audioctl_fd;
    int			audio_ready;
    int			audioBufferSize;
    int			audioLowWaterMark;
    int			queueIn, queueOut;
    int			written, eof_written, eof_counter;
    pid_t		childProcID;
    double		stopwatchTime;
    audio_info_t	a_info;
    audio_device_t	a_dev;
    struct audio_info	info;

};

#endif // BZF_SOLARISMEDIA_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

