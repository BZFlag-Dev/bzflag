/* bzflag
 * Copyright (c) 1993 - 2000 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* WinMedia:
 *	Media I/O on MS Windows.
 */

#ifndef BZF_WINMEDIA_H
#define	BZF_WINMEDIA_H

#include "BzfMedia.h"
#include <windows.h>
#include <dsound.h>

class WinWindow;

class WinMedia : public BzfMedia {
  public:
			WinMedia(WinWindow*);
			~WinMedia();

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
    static DWORD WINAPI	audioThreadInit(void*);

  private:
    HWND		window;
    boolean		audioReady;
    boolean		audioPlaying;
    IDirectSound*	audioInterface;
    IDirectSoundBuffer*	audioPrimaryPort;
    IDirectSoundBuffer*	audioPort;
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
    HANDLE		dummyEvent;
    static void		(*threadProc)(void*);
    static void*	threadData;
};

#endif // BZF_WINMEDIA_H
