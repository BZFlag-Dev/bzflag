/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "LinuxMedia.h"
#include <math.h>
#include <fcntl.h>
#include <endian.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <TimeKeeper.h>

#ifdef HALF_RATE_AUDIO
static const int defaultAudioRate=11025;
#else
static const int defaultAudioRate=22050;
#endif

//
// LinuxMedia
//

LinuxMedia::LinuxMedia() : BzfMedia(), audioReady(False),
				audioPortFd(-1),
				queueIn(-1), queueOut(-1),
				outputBuffer(NULL),
				childProcID(0)
{
  // do nothing
}

LinuxMedia::~LinuxMedia()
{
  // do nothing
}

double			LinuxMedia::stopwatch(boolean start)
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  if (start) {
    stopwatchTime = (double)tv.tv_sec + 1.0e-6 * (double)tv.tv_usec;
    return 0.0;
  }
  return (double)tv.tv_sec + 1.0e-6 * (double)tv.tv_usec - stopwatchTime;
}

void			LinuxMedia::sleep(float timeInSeconds)
{
  struct timeval tv;
  tv.tv_sec = (long)timeInSeconds;
  tv.tv_usec = (long)(1.0e6 * (timeInSeconds - tv.tv_sec));
  select(0, NULL, NULL, NULL, &tv);
}

boolean			LinuxMedia::openAudio()
{
  // don't re-initialize
  if (audioReady) return False;

  // check for and open audio hardware
  if (!checkForAudioHardware() || !openAudioHardware()) return False;

  // open communication channel (FIFO pipe).  close on exec.
  int fd[2];
  if (pipe(fd)<0) {
    closeAudio();
    return False;
  }
  queueIn = fd[1];
  queueOut = fd[0];
  fcntl(queueIn,  F_SETFL, fcntl(queueIn, F_GETFL, 0)  | O_NDELAY);
  fcntl(queueOut, F_SETFL, fcntl(queueOut, F_GETFL, 0) | O_NDELAY);
  fcntl(queueIn,  F_SETFD, fcntl(queueIn, F_GETFD, 0)  | FD_CLOEXEC);
  fcntl(queueOut, F_SETFD, fcntl(queueOut, F_GETFD, 0) | FD_CLOEXEC);

  // compute maxFd for use in select() call
  maxFd = queueOut;
  if (maxFd<audioPortFd) maxFd = audioPortFd;
  maxFd++;

  // make an output buffer
  outputBuffer = new short[audioBufferSize];

  // Set default no thread
  childProcID=0;

  // ready to go
  audioReady = True;
  return True;
}

boolean			LinuxMedia::checkForAudioHardware()
{
  if (!access("/dev/dsp", W_OK)) return True;
  return False;
}

static const int	NumChunks = 3;

boolean			LinuxMedia::openAudioHardware()
{
  int format, origFormat, stereo, audioChunkSize;

  if ((audioPortFd=open("/dev/dsp", O_WRONLY, 0))==-1) {
    fprintf(stderr, "Couldn't open /dev/dsp\n");
    return False;
  }
  if (ioctl(audioPortFd, SNDCTL_DSP_RESET, 0)==-1) {
    fprintf(stderr, "Couldn't reset audio\n");
    return False;
  }

  /* close audio on exec so launched server doesn't hold sound device */
  fcntl(audioPortFd, F_SETFD, fcntl(audioPortFd, F_GETFD) | FD_CLOEXEC);

  audio8Bit=False;
#if BYTE_ORDER == BIG_ENDIAN
  origFormat=AFMT_S16_BE;
#else
  origFormat=AFMT_S16_LE;
#endif
  format=origFormat;
  if ((ioctl(audioPortFd, SNDCTL_DSP_SETFMT, &format)==-1) || 
      format!=origFormat) {
    format=AFMT_U8;
    audio8Bit=True;
    if ((ioctl(audioPortFd, SNDCTL_DSP_SETFMT, &format)==-1) || 
	format!=AFMT_U8) {
      close(audioPortFd);
      audioPortFd=-1;
      fprintf(stderr, "Couldn't setup audio format\n");
      return False;
    }
  }
  stereo = 1;
  if ((ioctl(audioPortFd, SNDCTL_DSP_STEREO, &stereo)==-1) ||
      stereo!=1) {
    close(audioPortFd);
    audioPortFd=-1;
    fprintf(stderr, "Couldn't set stereo mode\n");
    return False;
  }
  audioOutputRate=defaultAudioRate;
  if (ioctl(audioPortFd, SNDCTL_DSP_SPEED, &audioOutputRate)==-1) {
    close(audioPortFd);
    audioPortFd=-1;
    fprintf(stderr, "Couldn't set rate to %d\n", defaultAudioRate);
    return False;
  }
/*
  if (audioOutputRate!=defaultAudioRate) {
    fprintf(stderr, "Using output rate of %d\n", audioOutputRate);
  }
*/

  // get size of fragment
  audio_buf_info info;
  if (ioctl(audioPortFd, SNDCTL_DSP_GETOSPACE, &info) < 0) {
    close(audioPortFd);
    audioPortFd=-1;
    fprintf(stderr, "Couldn't get audio buffer parameters\n");
    return False;
  }

  // set other sound buffering parameters
  audioBufferSize = info.fragsize;
  if (!audio8Bit) audioBufferSize >>= 1;
  audioLowWaterMark = info.fragstotal - 2;

  fprintf(stderr, "audio initialized\n");
}

void			LinuxMedia::closeAudio()
{
  delete [] outputBuffer;
  if (audioPortFd>=0) close(audioPortFd);
  if (queueIn!=-1) close(queueIn);
  if (queueOut!=-1) close(queueOut);
  audioReady=False;
  audioPortFd=-1;
  queueIn=-1;
  queueOut=-1;
  outputBuffer=0;
}

boolean			LinuxMedia::isAudioBrainDead() const
{
  return True;
}

boolean			LinuxMedia::startAudioThread(
				void (*proc)(void*), void* data)
{
  // if no audio thread then just call proc and return
  if (!hasAudioThread()) {
    proc(data);
    return True;
  }

  // has an audio thread so fork and call proc
  if (childProcID) return True;
  if ((childProcID=fork()) > 0) {
    close(queueOut);
    close(audioPortFd);
    return True;
  }
  else if (childProcID < 0) {
    return False;
  }
  close(queueIn);
  proc(data);
  exit(0);
}

void			LinuxMedia::stopAudioThread()
{
  if (childProcID != 0) kill(childProcID, SIGTERM);
  childProcID=0;
}

boolean			LinuxMedia::hasAudioThread() const
{
#if defined(NO_AUDIO_THREAD)
  return False;
#else
  return True;
#endif
}

void			LinuxMedia::audioThreadInit(void*)
{
}

void			LinuxMedia::writeSoundCommand(const void* cmd, int len)
{
  if (!audioReady) return;
  write(queueIn, cmd, len);
}

boolean			LinuxMedia::readSoundCommand(void* cmd, int len)
{
  return (read(queueOut, cmd, len)==len);
}

int			LinuxMedia::getAudioOutputRate() const
{
  return audioOutputRate;
}

int			LinuxMedia::getAudioBufferSize() const
{
  return NumChunks*(audioBufferSize>>1);
}

int			LinuxMedia::getAudioBufferChunkSize() const
{
  return audioBufferSize>>1;
}

boolean			LinuxMedia::isAudioTooEmpty() const
{
  audio_buf_info info;

  if (ioctl(audioPortFd, SNDCTL_DSP_GETOSPACE, &info)==-1) {
    fprintf(stderr, "Couldn't read sound buffer space\n");
    return False;
  }
  return info.fragments >= audioLowWaterMark;
}

void			LinuxMedia::writeAudioFrames8Bit(
				const float* samples, int numFrames)
{
  int numSamples = 2 * numFrames;
  int limit;
  char *smOutputBuffer;

  smOutputBuffer=(char*)outputBuffer;
  while (numSamples > 0) {
    if (numSamples>audioBufferSize) limit=audioBufferSize;
    else limit=numSamples;
    for (int j = 0; j < limit; j++) {
      if (samples[j] <= -32767.0) smOutputBuffer[j] = 0;
      else if (samples[j] >= 32767.0) smOutputBuffer[j] = 255;
      else smOutputBuffer[j] = char((samples[j]+32767)/257);
    }

    write(audioPortFd, smOutputBuffer, limit);
    samples += limit;
    numSamples -= limit;
  }
}

void			LinuxMedia::writeAudioFrames16Bit(
				const float* samples, int numFrames)
{
  int numSamples = 2 * numFrames;
  int limit;

  while (numSamples > 0) {
    if (numSamples>audioBufferSize) limit=audioBufferSize;
    else limit=numSamples;
    for (int j = 0; j < limit; j++) {
      if (samples[j] < -32767.0) outputBuffer[j] = -32767;
      else if (samples[j] > 32767.0) outputBuffer[j] = 32767;
      else outputBuffer[j] = short(samples[j]);
    }

    write(audioPortFd, outputBuffer, 2*limit);
    samples += limit;
    numSamples -= limit;
  }
}

void			LinuxMedia::writeAudioFrames(
				const float* samples, int numFrames)
{
  if (audio8Bit) writeAudioFrames8Bit(samples, numFrames);
  else writeAudioFrames16Bit(samples, numFrames);
}

void			LinuxMedia::audioSleep(
				boolean checkLowWater, double endTime)
{
  fd_set commandSelectSet;
  struct timeval tv;

  FD_ZERO(&commandSelectSet);
  FD_SET(queueOut, &commandSelectSet);

  isAudioTooEmpty();
  // To do both these operations at once, we need to poll.
  if (checkLowWater) {
    // start looping
    TimeKeeper start = TimeKeeper::getCurrent();
    do {
      // break if buffer has drained enough
      if (isAudioTooEmpty()) break;
      FD_SET(queueOut, &commandSelectSet);
      tv.tv_sec=0;
      tv.tv_usec=0;
      if (select(maxFd, &commandSelectSet, 0, 0, &tv)) break;

    } while (endTime<0.0 || (TimeKeeper::getCurrent()-start)<endTime);
  } else {
    tv.tv_sec=int(endTime);
    tv.tv_usec=int(1.0e6*(endTime-floor(endTime)));

    select(maxFd, &commandSelectSet, 0, 0, (endTime>=0.0)?&tv : 0);
  }
}
