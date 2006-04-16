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

/* WinMedia:
 *	Media I/O on MS Windows.
 */

#ifndef BZF_WINMEDIA_H
#define	BZF_WINMEDIA_H

#include "BzfMedia.h"
#ifdef HAVE_DSOUND_H
#include <dsound.h>
#endif

class WinWindow;

class WinMedia : public BzfMedia {
  public:
			WinMedia(WinWindow*);
			~WinMedia();

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
    static DWORD WINAPI	audioThreadInit(void*);

  private:
    HWND		window;
    bool		audioReady;
    bool		audioPlaying;
#ifdef HAVE_DSOUND_H
    IDirectSound*	audioInterface;
    IDirectSoundBuffer*	audioPrimaryPort;
    IDirectSoundBuffer*	audioPort;
#endif
    int			audioNumChannels;
    int			audioOutputRate;
    int			audioBufferSize;
    int			audioBufferChunkSize;
    int			audioLowWaterMark;
    int			audioBytesPerSample;
    int			audioBytesPerFrame;
    int			audioWritePtr;
    short*		outputBuffer;
    unsigned char*	audioCommandBuffer;
    int			audioCommandBufferLen;
    int			audioCommandBufferHead;
    int			audioCommandBufferTail;
    HANDLE		audioCommandEvent;
    HANDLE		audioCommandMutex;
    HANDLE		audioThread;
    static void		(*threadProc)(void*);
    static void*	threadData;
};

#endif // BZF_WINMEDIA_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

