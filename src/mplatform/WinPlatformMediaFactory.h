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

/* WinPlatformMediaFactory:
 *	Factory for Windows platform media stuff.
 */

#ifndef BZF_WIN_PLATFORM_MEDIA_FACTORY_H
#define BZF_WIN_PLATFORM_MEDIA_FACTORY_H

#include "PlatformMediaFactory.h"
#include <windows.h>
#include <dsound.h>

class WinWindow;

class WinPlatformMediaFactory : public PlatformMediaFactory {
public:
						WinPlatformMediaFactory();
						~WinPlatformMediaFactory();

	BzfDisplay*			createDisplay(const char* name,
							const char* videoFormat);
	BzfVisual*			createVisual(const BzfDisplay*);
	BzfWindow*			createWindow(const BzfDisplay*, BzfVisual*);
	bool				openAudio();
	void				closeAudio();
	bool				startAudioThread(void (*)(void*), void*);
	void				stopAudioThread();
	bool				hasAudioThread() const;
	void				writeSoundCommand(const void*, int);
	bool				readSoundCommand(void*, int);
	int					getAudioOutputRate() const;
	int					getAudioBufferSize() const;
	int					getAudioBufferChunkSize() const;
	bool				isAudioTooEmpty() const;
	void				writeAudioFrames(const float* samples, int numFrames);
	void				audioSleep(bool checkLowWater, double maxTime);

private:
	WinPlatformMediaFactory(const WinPlatformMediaFactory&);
	WinPlatformMediaFactory&	operator=(const WinPlatformMediaFactory&);

	static DWORD WINAPI	audioThreadInit(void*);

private:
	HWND				window;
	bool				audioReady;
	bool				audioPlaying;
	IDirectSound*		audioInterface;
	IDirectSoundBuffer*	audioPrimaryPort;
	IDirectSoundBuffer*	audioPort;
	int					audioNumChannels;
	int					audioOutputRate;
	int					audioBufferSize;
	int					audioBufferChunkSize;
	int					audioLowWaterMark;
	int					audioBytesPerSample;
	int					audioBytesPerFrame;
	int					audioWritePtr;
	short*				outputBuffer;
	unsigned char*		audioCommandBuffer;
	int					audioCommandBufferLen;
	int					audioCommandBufferHead;
	int					audioCommandBufferTail;
	HANDLE				audioCommandEvent;
	HANDLE				audioCommandMutex;
	HANDLE				audioThread;
	static void			(*threadProc)(void*);
	static void*		threadData;
};

#endif // BZF_WIN_PLATFORM_MEDIA_FACTORY_H
// ex: shiftwidth=4 tabstop=4
