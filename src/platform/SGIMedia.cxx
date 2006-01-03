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

#include "SGIMedia.h"
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <invent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#ifdef HAVE_BSTRING_H
#  include <bstring.h>
#endif
#include <sys/prctl.h>
#include <sys/wait.h>
#include "bzsignal.h"
#include <limits.h>
#include <sys/schedctl.h>

#include <stddef.h>
#include <sys/mman.h>
#include <sys/syssgi.h>

//
// SGIMedia
//

static const int	NumChunks = 3;

void			(*SGIMedia::threadProc)(void*);
void*			SGIMedia::threadData;

SGIMedia::SGIMedia() : BzfMedia(), audioReady(false),
				audioPort(NULL),
				queueIn(-1), queueOut(-1),
				outputBuffer(NULL),
				childProcID(0),
				iotimer_addr(NULL)
{
  // prepare high resolution timer
  unsigned int cycleval;
  const ptrdiff_t addr = syssgi(SGI_QUERY_CYCLECNTR, &cycleval);
  if (addr != -1) {
    const int poffmask = getpagesize() - 1;
    const __psunsigned_t phys_addr = (__psunsigned_t)addr;
    const __psunsigned_t raddr = phys_addr & ~poffmask;
    int fd = open("/dev/mmem", O_RDONLY);
    iotimer_addr = (volatile unsigned int *)mmap(0, poffmask, PROT_READ,
				MAP_PRIVATE, fd, (off_t)raddr);
    iotimer_addr = (unsigned int *)((__psunsigned_t)iotimer_addr +
						(phys_addr & poffmask));
#ifdef SGI_CYCLECNTR_SIZE
    if ((int)syssgi(SGI_CYCLECNTR_SIZE) > 32) iotimer_addr++;
#endif
    secondsPerTick = 1.0e-12 * (double)cycleval;
  }
}

SGIMedia::~SGIMedia()
{
  // do nothing
}

double			SGIMedia::stopwatch(bool start)
{
  if (!iotimer_addr) return 0.0;
  if (start) {
    stopwatchTime = *iotimer_addr;
    return 0.0;
  }
  else {
    return (double)(*iotimer_addr - stopwatchTime) * secondsPerTick;
  }
}

bool			SGIMedia::openAudio()
{
  // don't re-initialize
  if (audioReady) return false;

  // check for and open audio hardware
  if (!checkForAudioHardware() || !openAudioHardware()) return false;

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

bool			SGIMedia::checkForAudioHardware()
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

bool			SGIMedia::openAudioHardware()
{
  ALconfig	config;

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

void			SGIMedia::closeAudio()
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

bool			SGIMedia::startAudioThread(
				void (*proc)(void*), void* data)
{
  if (childProcID > 0 || !proc) return false;
  threadProc = proc;
  threadData = data;
  childProcID = sproc(&audioThreadInit, PR_SADDR);
  return (childProcID != -1);
}

void			SGIMedia::stopAudioThread()
{
  // kill child proc and wait for it
  kill(childProcID, SIGTERM);
  wait(NULL);
  childProcID = 0;
}

bool			SGIMedia::hasAudioThread() const
{
  return true;
}

static void		die(int)
{
  exit(0);
}

void			SGIMedia::audioThreadInit(void*)
{
  // make sure I die when the parent does
  prctl(PR_TERMCHILD, 0);

  // parent will kill me when it wants me to quit.  catch the signal
  // and gracefully exit.
  bzSignal(SIGTERM, SIG_PF(die));

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

void			SGIMedia::writeSoundCommand(const void* cmd, int len)
{
  if (!audioReady) return;
  write(queueIn, cmd, len);
}

bool			SGIMedia::readSoundCommand(void* cmd, int len)
{
  return (read(queueOut, cmd, len) == len);
}

int			SGIMedia::getAudioOutputRate() const
{
  return audioParams[1];
}

int			SGIMedia::getAudioBufferSize() const
{
  return NumChunks * (audioBufferSize >> 1);
}

int			SGIMedia::getAudioBufferChunkSize() const
{
  return audioBufferSize >> 1;
}

bool			SGIMedia::isAudioTooEmpty() const
{
  return ALgetfillable(audioPort) >= audioLowWaterMark;
}

void			SGIMedia::writeAudioFrames(
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

void			SGIMedia::audioSleep(
				bool checkLowWater, double endTime)
{
  // prepare fd bit vectors
  fd_set audioSelectSet;
  fd_set commandSelectSet;
  FD_ZERO(&commandSelectSet);
  FD_SET((unsigned int)queueOut, &commandSelectSet);
  if (checkLowWater) {
    FD_ZERO(&audioSelectSet);
    FD_SET((unsigned int)audioPortFd, &audioSelectSet);
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

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

