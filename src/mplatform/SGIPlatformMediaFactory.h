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

/* SGIPlatformMediaFactory:
 *	Factory for SGI Irix platform media stuff.
 */

#ifndef BZF_SGI_PLATFORM_MEDIA_FACTORY_H
#define BZF_SGI_PLATFORM_MEDIA_FACTORY_H

#include "UnixPlatformMediaFactory.h"
#include <dmedia/audio.h>

class SGIPlatformMediaFactory : public UnixPlatformMediaFactory {
public:
	SGIPlatformMediaFactory();
	~SGIPlatformMediaFactory();

	BzfDisplay*			createDisplay(const char* name, const char*);
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
	SGIPlatformMediaFactory(const SGIPlatformMediaFactory&);
	SGIPlatformMediaFactory&	operator=(const SGIPlatformMediaFactory&);

	bool				checkForAudioHardware();
	bool				openAudioHardware();
	static void			audioThreadInit(void*);

private:
	bool				audioReady;
	long				originalAudioParams[8];
	long				audioParams[6];
	long				audioOutputRate;
	long				audioBufferSize;
	long				audioLowWaterMark;
	int					maxFd;
	ALport				audioPort;
	int					audioPortFd;
	int					queueIn, queueOut;
	short*				outputBuffer;
	int					childProcID;
	double				secondsPerTick;
	unsigned int		stopwatchTime;
	volatile unsigned int* iotimer_addr;
	static void			(*threadProc)(void*);
	static void*		threadData;
};

#endif // BZF_SGI_PLATFORM_MEDIA_FACTORY_H
