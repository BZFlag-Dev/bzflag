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

#include "SGIPlatformMediaFactory.h"
#include "SGIDisplay.h"
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <invent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
//#include <bstring.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <signal.h>
#include <limits.h>
#include <sys/schedctl.h>
#include <stddef.h>

PlatformMediaFactory*	PlatformMediaFactory::getInstance()
{
	if (instance == NULL)
		instance = new SGIPlatformMediaFactory;
	return instance;
}

static const int	NumChunks = 3;

void					(*SGIPlatformMediaFactory::threadProc)(void*);
void*					SGIPlatformMediaFactory::threadData;

SGIPlatformMediaFactory::SGIPlatformMediaFactory() :
								audioReady(false),
								audioPort(NULL),
								queueIn(-1), queueOut(-1),
								outputBuffer(NULL),
								childProcID(0)
{
	// do nothing
}

SGIPlatformMediaFactory::~SGIPlatformMediaFactory()
{
	// do nothing
}

BzfDisplay*				SGIPlatformMediaFactory::createDisplay(
								const char* name, const char*)
{
	XDisplay* display = new XDisplay(name, new SGIDisplayMode);
	if (!display || !display->isValid()) {
		delete display;
		return NULL;
	}
	return display;
}

bool					SGIPlatformMediaFactory::openAudio()
{
	// don't re-initialize
	if (audioReady)
		return false;

	// check for and open audio hardware
	if (!checkForAudioHardware() || !openAudioHardware())
		return false;

	// open communication channel (FIFO pipe)
	int fd[2];
	if (pipe(fd) < 0) {
		closeAudio();
		return false;
	}
	queueIn = fd[1];
	queueOut = fd[0];
	fcntl(queueOut, F_SETFL, fcntl(queueOut, F_GETFL, 0) | O_NDELAY);

	// compute maxFd for use in select() call
	maxFd = queueOut;
	if (maxFd < audioPortFd) maxFd = audioPortFd;
	maxFd++;

	// make an output buffer
	outputBuffer = new short[audioBufferSize];

	// ready to go
	audioReady = true;
	return true;
}

bool					SGIPlatformMediaFactory::checkForAudioHardware()
{
	inventory_t* scan;
	setinvent();
	if (!(scan = getinvent())) return 0;
	while (scan) {
		if (scan->inv_class == INV_AUDIO) break;
		scan = getinvent();
	}
	endinvent();
	return scan != NULL;
}

bool					SGIPlatformMediaFactory::openAudioHardware()
{
	ALconfig		config;

	// get current configuration
	originalAudioParams[0] = AL_OUTPUT_RATE;
	originalAudioParams[2] = AL_LEFT_SPEAKER_GAIN;
	originalAudioParams[4] = AL_RIGHT_SPEAKER_GAIN;
	originalAudioParams[6] = AL_INPUT_RATE;
	ALgetparams(AL_DEFAULT_DEVICE, originalAudioParams, 8);
	if (originalAudioParams[1] == 0)
		originalAudioParams[1] = originalAudioParams[7];

	// compute my desired configuration
	// NOTE: should be able to set this to originalAudioParams[1]
#if defined(HALF_RATE_AUDIO)
	audioOutputRate = AL_RATE_11025;
#else
	audioOutputRate = AL_RATE_22050;
#endif
	audioBufferSize = (long)(0.1f * (float)audioOutputRate) & ~1;
	audioLowWaterMark = audioBufferSize;

	// make port configuration
	config = ALnewconfig();
	ALsetwidth(config, AL_SAMPLE_16);
	ALsetchannels(config, AL_STEREO);
	ALsetqueuesize(config, NumChunks * audioBufferSize);

	// open output port
	audioPort = ALopenport("bzflag", "w", config);
	if (audioPort) {
		audioPortFd = ALgetfd(audioPort);
		ALsetfillpoint(audioPort, audioLowWaterMark);
	}

	// free configuration
	ALfreeconfig(config);

	// if no audio ports available then don't change configuration
	if (audioPort == 0) return false;

	// set my configuration
	audioParams[0] = AL_OUTPUT_RATE;
	audioParams[1] = audioOutputRate;
	audioParams[2] = AL_LEFT_SPEAKER_GAIN;
	audioParams[3] = originalAudioParams[3];
	audioParams[4] = AL_RIGHT_SPEAKER_GAIN;
	audioParams[5] = originalAudioParams[5];
	ALsetparams(AL_DEFAULT_DEVICE, audioParams, 6);

	return true;
}

void					SGIPlatformMediaFactory::closeAudio()
{
	// release memory
	delete[] outputBuffer;

	// close down audio ports
	if (audioPort) {
		ALcloseport(audioPort);

		// reset original configuration
		ALsetparams(AL_DEFAULT_DEVICE, originalAudioParams, 6);
	}

	// close audio command queue
	if (queueIn != -1) close(queueIn);
	if (queueOut != -1) close(queueOut);

	audioReady = false;
	audioPort = NULL;
	queueIn = -1;
	queueOut = -1;
	outputBuffer = NULL;
}

bool					SGIPlatformMediaFactory::startAudioThread(
								void (*proc)(void*), void* data)
{
	if (childProcID > 0 || !proc)
		return false;
	threadProc = proc;
	threadData = data;
	childProcID = sproc(&audioThreadInit, PR_SADDR);
	return (childProcID != -1);
}

void					SGIPlatformMediaFactory::stopAudioThread()
{
	// kill child proc and wait for it
	kill(childProcID, SIGTERM);
	wait(NULL);
	childProcID = 0;
}

bool					SGIPlatformMediaFactory::hasAudioThread() const
{
	return true;
}

static void				die(int)
{
	exit(0);
}

void					SGIPlatformMediaFactory::audioThreadInit(void*)
{
	// make sure I die when the parent does
	prctl(PR_TERMCHILD, 0);

	// parent will kill me when it wants me to quit.  catch the signal
	// and gracefully exit.  don't use PlatformFactory because that
	// doesn't distinguish between processes.
	signal(SIGTERM, SIG_PF(die));

#if defined(DEADLINE)
	// increase priority
	struct sched_deadline deadline;
	deadline.dl_period.tv_sec = 0;
	deadline.dl_period.tv_nsec = 25000000;
	deadline.dl_alloc.tv_sec = 0;
	deadline.dl_alloc.tv_nsec = 1000000;
	schedctl(DEADLINE, 0, (int*)&deadline);
#endif

	(*threadProc)(threadData);
}

void					SGIPlatformMediaFactory::writeSoundCommand(const void* cmd, int len)
{
	if (!audioReady) return;
	write(queueIn, cmd, len);
}

bool					SGIPlatformMediaFactory::readSoundCommand(void* cmd, int len)
{
	return (read(queueOut, cmd, len) == len);
}

int						SGIPlatformMediaFactory::getAudioOutputRate() const
{
	return audioParams[1];
}

int						SGIPlatformMediaFactory::getAudioBufferSize() const
{
	return NumChunks * (audioBufferSize >> 1);
}

int						SGIPlatformMediaFactory::getAudioBufferChunkSize() const
{
	return audioBufferSize >> 1;
}

bool					SGIPlatformMediaFactory::isAudioTooEmpty() const
{
	return ALgetfillable(audioPort) >= audioLowWaterMark;
}

void					SGIPlatformMediaFactory::writeAudioFrames(
								const float* samples, int numFrames)
{
	int numSamples = 2 * numFrames;
	while (numSamples > audioBufferSize) {
		for (int j = 0; j < audioBufferSize; j++)
			if (samples[j] < -32767.0f) outputBuffer[j] = -32767;
			else if (samples[j] > 32767.0f) outputBuffer[j] = 32767;
			else outputBuffer[j] = short(samples[j]);
		ALwritesamps(audioPort, outputBuffer, audioBufferSize);
		samples += audioBufferSize;
		numSamples -= audioBufferSize;
	}

	if (numSamples > 0) {
		for (int j = 0; j < numSamples; j++)
			if (samples[j] < -32767.0f) outputBuffer[j] = -32767;
			else if (samples[j] > 32767.0f) outputBuffer[j] = 32767;
			else outputBuffer[j] = short(samples[j]);
		ALwritesamps(audioPort, outputBuffer, numSamples);
	}
}

void					SGIPlatformMediaFactory::audioSleep(
								bool checkLowWater, double endTime)
{
	// prepare fd bit vectors
	fd_set audioSelectSet;
	fd_set commandSelectSet;
	FD_ZERO(&commandSelectSet);
	FD_SET(queueOut, &commandSelectSet);
	if (checkLowWater) {
		FD_ZERO(&audioSelectSet);
		FD_SET(audioPortFd, &audioSelectSet);
	}

	// prepare timeout
	struct timeval tv;
	if (endTime >= 0.0) {
		tv.tv_sec = (long)endTime;
		tv.tv_usec = (long)(1.0e6 * (endTime - floor(endTime)));
	}

	// wait
	select(maxFd, &commandSelectSet, checkLowWater ? &audioSelectSet : NULL,
						NULL, (struct timeval*)(endTime >= 0.0 ? &tv : NULL));
}
