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

#include "SolarisMedia.h"

#define DEBUG_SOLARIS			0	//(1 = debug, 0 = don't!)

#define AUDIO_BUFFER_SIZE		5000

#if defined(HALF_RATE_AUDIO)
static const int defaultAudioRate	= 11025;
#else
static const int defaultAudioRate	= 22050;
#endif

static const int defaultChannels	= 2;
static const int defaultEncoding 	= AUDIO_ENCODING_LINEAR;
static const int defaultPrecision	= 16;

short tmp_buf[512];

//
// SolarisMedia
//

SolarisMedia::SolarisMedia() : BzfMedia(), audio_fd(-1),
				queueIn(-1), queueOut(-1),
				childProcID(0),
				written(0), eof_written(0),
				eof_counter(0),audio_ready(0)

{
  // do nothing
}

SolarisMedia::~SolarisMedia()
{
  // do nothing
}

double			SolarisMedia::stopwatch(boolean start)
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	if (start) 
	{
		stopwatchTime = (double)tv.tv_sec + 1.0e-6 * (double)tv.tv_usec;
		return 0.0;
	}
	return (double)tv.tv_sec + 1.0e-6 * (double)tv.tv_usec - stopwatchTime;
}

void			SolarisMedia::sleep(float timeInSeconds)
{
	struct timeval tv;
	tv.tv_sec = (long)timeInSeconds;
	tv.tv_usec = (long)(1.0e6 * (timeInSeconds - tv.tv_sec));
	select(0, NULL, NULL, NULL, &tv);
}

boolean			SolarisMedia::openAudio()
{
  int fd[2];

  if(audio_ready)
    return False;

  audio_fd = open("/dev/audio", O_WRONLY  | O_NDELAY);

  if(audio_fd < 0)
    return False;

  if(DEBUG_SOLARIS)
    fprintf(stderr, "Audio device '/dev/audio' opened\n");

  audioctl_fd = open("/dev/audioctl", O_RDWR);

  if(audioctl_fd < 0)
    return False;

  if(DEBUG_SOLARIS)
    fprintf(stderr, "Opened audio control device '/dev/audioctl'\n");

  // Empty buffers
  ioctl(audio_fd, AUDIO_DRAIN, 0);
  ioctl(audio_fd, I_FLUSH, FLUSHRW);
  ioctl(audioctl_fd, I_FLUSH, FLUSHRW);

  if(ioctl(audio_fd, AUDIO_GETDEV, &a_dev) < 0)
  {
    if(DEBUG_SOLARIS)
      fprintf(stderr, "Cannot get audio information.\n");
    close(audio_fd);
    close(audioctl_fd);
    return False;
  }

  if(DEBUG_SOLARIS)
    fprintf(stderr, "Sound device is a %s %s version %s\n", a_dev.config, a_dev.name, a_dev.version);

  // Get audio parameters
  if(ioctl(audioctl_fd, AUDIO_GETINFO, &a_info) < 0)
  {
    if(DEBUG_SOLARIS)
      fprintf(stderr, "Cannot get audio information.\n");
    close(audio_fd);
    close(audioctl_fd);
    return False;
  }

  AUDIO_INITINFO(&a_info);

  a_info.play.sample_rate = defaultAudioRate;
  a_info.play.channels    = defaultChannels;
  a_info.play.precision   = defaultPrecision;
  a_info.play.encoding    = defaultEncoding;

  a_info.play.buffer_size = AUDIO_BUFFER_SIZE;
  audioBufferSize   	  = AUDIO_BUFFER_SIZE;
  audioLowWaterMark	  = AUDIO_BUFFER_SIZE;

  if(ioctl(audio_fd, AUDIO_SETINFO, &a_info) == -1)
  {
    if(DEBUG_SOLARIS)
      fprintf(stderr, "Warning: Cannot set audio parameters.\n");

    return False;
  }

  if(DEBUG_SOLARIS)
    fprintf(stderr, "Audio initialised. Setting up queues...\n");

  if (pipe(fd)<0) {
    closeAudio();
    return False;
  }

  queueIn = fd[1];
  queueOut = fd[0];

  fcntl(queueOut, F_SETFL, fcntl(queueOut, F_GETFL, 0) | O_NDELAY);

  // compute maxFd for use in select() call
  maxFd = queueOut;
  if (maxFd<audio_fd) maxFd = audio_fd;
  maxFd++;

  // Set default no thread
  childProcID=0;

  // ready to go
  audio_ready = True;
 
  if(DEBUG_SOLARIS)
    fprintf(stderr, "Audio ready.\n");

  return True;
}


void			SolarisMedia::closeAudio()
{
  close(audio_fd);
  close(audioctl_fd);
}

boolean			SolarisMedia::startAudioThread(
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
    close(audio_fd);
    return True;
  }
  else if (childProcID < 0) {
    return False;
  }
  close(queueIn);
  proc(data);
  exit(0);
}

void			SolarisMedia::stopAudioThread()
{
  if (childProcID != 0) kill(childProcID, SIGTERM);
  childProcID=0;
}

boolean			SolarisMedia::hasAudioThread() const
{
  // XXX -- adjust this if the system always uses or never uses a thread
#if defined(NO_AUDIO_THREAD)
  return False;
#else
  return True;
#endif
}

void			SolarisMedia::writeSoundCommand(const void* cmd, int len)
{
  if (!audio_ready) return;
  write(queueIn, cmd, len);
}

boolean			SolarisMedia::readSoundCommand(void* cmd, int len)
{
  return (read(queueOut, cmd, len)==len);
}

int			SolarisMedia::getAudioOutputRate() const
{
  return defaultAudioRate;
}

int			SolarisMedia::getAudioBufferSize() const
{
  return audioBufferSize;
}

int			SolarisMedia::getAudioBufferChunkSize() const
{
  return audioBufferSize>>1;
}

boolean			SolarisMedia::isAudioTooEmpty() const
{
  ioctl(audioctl_fd, AUDIO_GETINFO, &a_info);

  return (AUDIO_BUFFER_SIZE - (written-(a_info.play.eof*512))) >= audioLowWaterMark;
}

void			SolarisMedia::writeAudioFrames(
				const float* samples, int numFrames)
{
  int i;
  char c;

  i = 0;
  while(i < (audioBufferSize / 2)) 
  {
    if(samples[i] < -32767.0) tmp_buf[eof_counter] = -32767;
    else
      if(samples[i] > 32767.0) tmp_buf[eof_counter] = 32767;
      else
        tmp_buf[eof_counter] = (short) samples[i];

    eof_counter++;
    i = i++;
    if(eof_counter >= 512)
    {
      write(audio_fd, tmp_buf, (eof_counter)*2);
      written += (eof_counter);
      write(audio_fd, &c, 0);
      eof_counter = 0;
      eof_written++;
    }
  }
}

void			SolarisMedia::audioSleep(
				boolean checkLowWater, double endTime)
{
  // prepare fd bit vectors
  fd_set audioSelectSet;
  fd_set commandSelectSet;
  FD_ZERO(&commandSelectSet);
  FD_SET(queueOut, &commandSelectSet);
  if (checkLowWater) {
    FD_ZERO(&audioSelectSet);
    FD_SET(audio_fd, &audioSelectSet);
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
