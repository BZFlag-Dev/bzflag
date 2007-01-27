/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "LinuxMedia.h"
#include <math.h>
#include <fcntl.h>
#ifdef BSD
#include <machine/endian.h>
#else
#include <endian.h>
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "bzsignal.h"
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <TimeKeeper.h>
#include <errno.h>
#include <string.h>

#ifdef HALF_RATE_AUDIO
static const int defaultAudioRate=11025;
#else
static const int defaultAudioRate=22050;
#endif

//
// LinuxMedia
//

LinuxMedia::LinuxMedia() : BzfMedia(), audioReady(false),
				audioBufferSize(defaultAudioRate),
				audioPortFd(-1),
				queueIn(-1), queueOut(-1),
				outputBuffer(NULL),
				childProcID(0),
				audio8Bit(false),
				getospaceBroken(false)
{
  // do nothing
}

LinuxMedia::~LinuxMedia()
{
  // do nothing
}

double			LinuxMedia::getTime()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return (double)tv.tv_sec + 1.0e-6 * (double)tv.tv_usec;
}

double			LinuxMedia::stopwatch(bool start)
{
  if (start) {
    stopwatchTime = getTime();
    return 0.0;
  }
  return getTime() - stopwatchTime;
}

bool			LinuxMedia::openAudio()
{
  // don't re-initialize
  if (audioReady) return false;

  // check for and open audio hardware
  if (!checkForAudioHardware() || !openAudioHardware()) return false;

  // open communication channel (FIFO pipe).  close on exec.
  int fd[2];
  if (pipe(fd)<0) {
    closeAudio();
    return false;
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
  audioReady = true;
  return true;
}

bool			LinuxMedia::checkForAudioHardware()
{
  bool flag=false;
  if (!access("/dev/dsp", W_OK)) flag=true;
  if (!access("/dev/sound/dsp", W_OK)) flag=true;
  return flag;
}

bool			LinuxMedia::openIoctl(
				int cmd, void* value, bool req)
{
  if (audioPortFd == -1)
      return false;

  if (ioctl(audioPortFd, cmd, value) < 0) {
    fprintf(stderr, "audio ioctl failed (cmd %x, err %d)... ", cmd, errno);
    if (req) {
      close(audioPortFd);
      audioPortFd = -1;
      fprintf(stderr, "giving up on audio\n");
    }
    else {
      fprintf(stderr, "ignored\n");
    }
    return false;
  }
  return true;
}

static const int	NumChunks = 4;

bool			LinuxMedia::openAudioHardware()
{
  int format, n;

  // what's the audio format?
#if BYTE_ORDER == BIG_ENDIAN
  format = AFMT_S16_BE;
#else
  format = AFMT_S16_LE;
#endif

  // what the frequency?
  audioOutputRate = defaultAudioRate;

  // how big a fragment to use?  we want to hold at around 1/10th of
  // a second.
  int fragmentSize = (int)(0.08f * (float)audioOutputRate);
  n = 0;
  while ((1 << n) < fragmentSize)
    ++n;

  // samples are two bytes each and we're in stereo so quadruple the size
  fragmentSize = n + 2;

  // now how many fragments and what's the low water mark (in fragments)?
  int fragmentInfo = (NumChunks << 16) | fragmentSize;
  audioLowWaterMark = 2;

  // open device (but don't wait for it)
  audioPortFd = open("/dev/dsp", O_WRONLY | O_NDELAY, 0);
  if (audioPortFd == -1) {
    audioPortFd = open("/dev/sound/dsp", O_WRONLY | O_NDELAY, 0);
    if (audioPortFd == -1) {
      fprintf(stderr, "Failed to open audio device /dev/dsp or /dev/sound/dsp (%d)\n", errno);
      return false;
	}
  }

  // back to blocking I/O
  fcntl(audioPortFd, F_SETFL, fcntl(audioPortFd, F_GETFL, 0) & ~O_NDELAY);

  /* close audio on exec so launched server doesn't hold sound device */
  fcntl(audioPortFd, F_SETFD, fcntl(audioPortFd, F_GETFD) | FD_CLOEXEC);

  // initialize device
  openIoctl(SNDCTL_DSP_RESET, 0);
  n = fragmentInfo;
  noSetFragment = false;
  if (!openIoctl(SNDCTL_DSP_SETFRAGMENT, &n, false)) {
    // this is not good.  we can't set the size of the fragment
    // buffers.  we'd like something short to minimize latencies
    // and the default is probably too long.  we've got two
    // options here:  accept the latency or try to force the
    // driver to play partial fragments.  we'll try the later
    // unless BZF_AUDIO_NOPOST is in the environment
    if (!getenv("BZF_AUDIO_NOPOST"))
      noSetFragment = true;
  }
  n = format;
  openIoctl(SNDCTL_DSP_SETFMT, &n, false);
  if (n != format) {
    audio8Bit = true;
    n = AFMT_U8;
    openIoctl(SNDCTL_DSP_SETFMT, &n);
  }
  n = 1;
  openIoctl(SNDCTL_DSP_STEREO, &n);
  n = defaultAudioRate;
  openIoctl(SNDCTL_DSP_SPEED, &n);

  // set audioBufferSize, which is the number of samples (not bytes)
  // in each fragment.  there are two bytes per sample so divide the
  // fragment size by two unless we're in audio8Bit mode.  also, if
  // we couldn't set the fragment size then force the buffer size to
  // the size we would've asked for.  we'll force the buffer to be
  // flushed after we write that much data to keep latency low.
  if (noSetFragment ||
	!openIoctl(SNDCTL_DSP_GETBLKSIZE, &audioBufferSize, false) ||
	audioBufferSize > (1 << fragmentSize)) {
    audioBufferSize = 1 << fragmentSize;
    noSetFragment = true;
  }
  if (!audio8Bit)
    audioBufferSize >>= 1;

  // SNDCTL_DSP_GETOSPACE not supported on all platforms.  check if
  // it fails here and, if so, do a workaround by using the wall
  // clock.  *shudder*
  if (audioPortFd != -1) {
    audio_buf_info info;
    if (!openIoctl(SNDCTL_DSP_GETOSPACE, &info, false)) {
      getospaceBroken = true;
      chunksPending = 0;
      chunksPerSecond = (double)getAudioOutputRate() /
				(double)getAudioBufferChunkSize();
    }
  }

  return (audioPortFd != -1);
}

void			LinuxMedia::closeAudio()
{
  delete [] outputBuffer;
  if (audioPortFd>=0) close(audioPortFd);
  if (queueIn!=-1) close(queueIn);
  if (queueOut!=-1) close(queueOut);
  audioReady=false;
  audioPortFd=-1;
  queueIn=-1;
  queueOut=-1;
  outputBuffer=0;
}

bool			LinuxMedia::startAudioThread(
				void (*proc)(void*), void* data)
{
  // if no audio thread then just call proc and return
  if (!hasAudioThread()) {
    proc(data);
    return true;
  }

  // has an audio thread so fork and call proc
  if (childProcID) return true;
  if ((childProcID=fork()) > 0) {
    close(queueOut);
    close(audioPortFd);
    return true;
  }
  else if (childProcID < 0) {
    return false;
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

bool			LinuxMedia::hasAudioThread() const
{
#if defined(NO_AUDIO_THREAD)
  return false;
#else
  return true;
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

bool			LinuxMedia::readSoundCommand(void* cmd, int len)
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

bool			LinuxMedia::isAudioTooEmpty() const
{
  if (getospaceBroken) {
    if (chunksPending > 0) {
      // get time elapsed since chunkTime
      const double dt = getTime() - chunkTime;

      // how many chunks could've played in the elapsed time?
      const int numChunks = (int)(dt * chunksPerSecond);

      // remove pending chunks
      LinuxMedia* self = (LinuxMedia*)this;
      self->chunksPending -= numChunks;
      if (chunksPending < 0)
	self->chunksPending = 0;
      else
	self->chunkTime += (double)numChunks / chunksPerSecond;
    }
    return chunksPending < audioLowWaterMark;
  }
  else {
    audio_buf_info info;
    if (ioctl(audioPortFd, SNDCTL_DSP_GETOSPACE, &info) < 0)
      return false;
    return info.fragments > info.fragstotal - audioLowWaterMark;
  }
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

    // fill out the chunk (we never write a partial chunk)
    if (limit < audioBufferSize) {
      for (int j = limit; j < audioBufferSize; ++j)
	smOutputBuffer[j] = 127;
      limit = audioBufferSize;
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
  int limit = 0;

  while (numSamples > 0) {
    if (numSamples>audioBufferSize) limit=audioBufferSize;
    else limit=numSamples;
    for (int j = 0; j < limit; j++) {
      if (samples[j] < -32767.0) outputBuffer[j] = -32767;
      else if (samples[j] > 32767.0) outputBuffer[j] = 32767;
      else outputBuffer[j] = short(samples[j]);
    }

    // fill out the chunk (we never write a partial chunk)
    if (limit < audioBufferSize) {
      for (int j = limit; j < audioBufferSize; ++j)
	outputBuffer[j] = 0;
      limit = audioBufferSize;
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

  // if we couldn't set the fragment size then force the driver
  // to play the short buffer.
  if (noSetFragment) {
    int dummy = 0;
    ioctl(audioPortFd, SNDCTL_DSP_POST, &dummy);
  }

  if (getospaceBroken) {
    if (chunksPending == 0)
      chunkTime = getTime();
    chunksPending += (numFrames + getAudioBufferChunkSize() - 1) /
						getAudioBufferChunkSize();
  }
}

void			LinuxMedia::audioSleep(
				bool checkLowWater, double endTime)
{
  fd_set commandSelectSet;
  struct timeval tv;

  // To do both these operations at once, we need to poll.
  if (checkLowWater) {
    // start looping
    TimeKeeper start = TimeKeeper::getCurrent();
    do {
      // break if buffer has drained enough
      if (isAudioTooEmpty()) break;
      FD_ZERO(&commandSelectSet);
      FD_SET((unsigned int)queueOut, &commandSelectSet);
      tv.tv_sec=0;
      tv.tv_usec=50000;
      if (select(maxFd, &commandSelectSet, 0, 0, &tv)) break;

    } while (endTime<0.0 || (TimeKeeper::getCurrent()-start)<endTime);
  } else {
    FD_ZERO(&commandSelectSet);
    FD_SET((unsigned int)queueOut, &commandSelectSet);
    tv.tv_sec=int(endTime);
    tv.tv_usec=int(1.0e6*(endTime-floor(endTime)));

    select(maxFd, &commandSelectSet, 0, 0, (endTime>=0.0)?&tv : 0);
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
