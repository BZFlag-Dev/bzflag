/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* SolarisPlatformMediaFactory:
 *	Factory for Sun Sparc/Solaris platform media stuff.
 */

#ifndef BZF_SOLARIS_PLATFORM_MEDIA_FACTORY_H
#define BZF_SOLARIS_PLATFORM_MEDIA_FACTORY_H

#include "UnixPlatformMediaFactory.h"
#include <sys/audioio.h>
#include <sys/types.h>

class SolarisPlatformMediaFactory : public UnixPlatformMediaFactory {
public:
	SolarisPlatformMediaFactory();
	~SolarisPlatformMediaFactory();

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
	SolarisPlatformMediaFactory(const SolarisPlatformMediaFactory&);
	SolarisPlatformMediaFactory&	operator=(const SolarisPlatformMediaFactory&);

private:
	int					maxFd;
	int					audio_fd;
	int					audioctl_fd;
	int					audio_ready;
	int					audioBufferSize;
	int					audioLowWaterMark;
	int					queueIn, queueOut;
	int					written, eof_written, eof_counter;
	pid_t				childProcID;
	audio_info_t		a_info;
	audio_device_t		a_dev;
	struct audio_info	info;
};

#endif // BZF_SOLARIS_PLATFORM_MEDIA_FACTORY_H
