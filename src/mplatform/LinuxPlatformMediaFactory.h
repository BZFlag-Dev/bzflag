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

/* LinuxPlatformMediaFactory:
 *	Factory for Linux platform media stuff.
 */

#ifndef BZF_LINUX_MEDIA_PLATFORM_FACTORY_H
#define BZF_LINUX_MEDIA_PLATFORM_FACTORY_H

#include "UnixPlatformMediaFactory.h"

class LinuxPlatformMediaFactory : public UnixPlatformMediaFactory {
public:
	LinuxPlatformMediaFactory();
	~LinuxPlatformMediaFactory();

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
	LinuxPlatformMediaFactory(const LinuxPlatformMediaFactory&);
	LinuxPlatformMediaFactory& operator=(const LinuxPlatformMediaFactory&);

	bool				checkForAudioHardware();
	bool				openAudioHardware();
	bool				openIoctl(int cmd, void* value, bool req = true);
	static void			audioThreadInit(void*);

	void				writeAudioFrames8Bit(
							const float* samples, int numFrames);
	void				writeAudioFrames16Bit(
							const float* samples, int numFrames);

private:
	bool				audioReady;
	int					audioOutputRate;
	int					audioBufferSize;
	int					audioLowWaterMark;
	int					maxFd;
	int					audioPortFd;
	int					queueIn, queueOut;
	short*				outputBuffer;
	pid_t				childProcID;
	bool				audio8Bit;

	bool				noSetFragment;
	bool				getospaceBroken;
	int					chunksPending;
	double				chunkTime;
	double				chunksPerSecond;

	int					cmdBufferSize;
	char				cmdBuffer[100];
};

#endif // BZF_LINUX_PLATFORM_MEDIA_FACTORY_H
