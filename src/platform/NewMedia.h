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
};

#endif // BZF_NEWMEDIA_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

