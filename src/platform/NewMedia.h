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

/* NewMedia:
 *	Media I/O template
 */

#ifndef BZF_NEWMEDIA_H
#define	BZF_NEWMEDIA_H

#include "BzfMedia.h"

class NewMedia : public BzfMedia {
  public:
			NewMedia();
			~NewMedia();

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
};

#endif // BZF_NEWMEDIA_H
// ex: shiftwidth=2 tabstop=8
