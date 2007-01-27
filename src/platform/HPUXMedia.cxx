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

//This is a copy of Solaris, and I don't care about audio right now, so I'm just commenting as needed

#include "HPUXMedia.h"
//#include "TimeKeeper.h"

#define DEBUG_HPUX			0	//(1 = debug, 0 = don't!)

#define AUDIO_BUFFER_SIZE		5000

#if defined(HALF_RATE_AUDIO)
static const int defaultAudioRate	= 11025;
#else
static const int defaultAudioRate	= 22050;
#endif

static const int defaultChannels	= 2;
//static const int defaultEncoding	= AUDIO_ENCODING_LINEAR;
static const int defaultPrecision	= 16;

short tmp_buf[512];

//
// HPUXMedia
//

HPUXMedia::HPUXMedia() /*: BzfMedia(), audio_fd(-1),
				queueIn(-1), queueOut(-1),
				childProcID(0),
				written(0), eof_written(0),
				eof_counter(0),audio_ready(0)*/

{
  // do nothing
}

HPUXMedia::~HPUXMedia()
{
  // do nothing
}
/*
double			HPUXMedia::stopwatch(bool start)
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
*/
static const int	NumChunks = 4;

bool			HPUXMedia::openAudio()
{
   return false;
/*  int fd[2];

  if(audio_ready)
    return false;

  audio_fd = open("/dev/audio", O_WRONLY  | O_NDELAY);

  if(audio_fd < 0)
    return false;

  if(DEBUG_HPUX)
    fprintf(stderr, "Audio device '/dev/audio' opened\n");

  audioctl_fd = open("/dev/audioctl", O_RDWR);

  if(audioctl_fd < 0)
    return false;

  if(DEBUG_HPUX)
    fprintf(stderr, "Opened audio control device '/dev/audioctl'\n");

// removing these avoids a kernel crash on HPUX 8 - bzFrank
#ifdef FLUSHDRAIN
  // Empty buffers
  ioctl(audio_fd, AUDIO_DRAIN, 0);
  ioctl(audio_fd, I_FLUSH, FLUSHRW);
  ioctl(audioctl_fd, I_FLUSH, FLUSHRW);
#endif

  if(ioctl(audio_fd, AUDIO_GETDEV, &a_dev) < 0)
  {
    if(DEBUG_HPUX)
      fprintf(stderr, "Cannot get audio information.\n");
    close(audio_fd);
    close(audioctl_fd);
    return false;
  }

  if(DEBUG_HPUX)
    fprintf(stderr, "Sound device is a %s %s version %s\n", a_dev.config, a_dev.name, a_dev.version);

  // Get audio parameters
  if(ioctl(audioctl_fd, AUDIO_GETINFO, &a_info) < 0)
  {
    if(DEBUG_HPUX)
      fprintf(stderr, "Cannot get audio information.\n");
    close(audio_fd);
    close(audioctl_fd);
    return false;
  }

  AUDIO_INITINFO(&a_info);

  a_info.play.sample_rate = defaultAudioRate;
  a_info.play.channels    = defaultChannels;
  a_info.play.precision   = defaultPrecision;
  a_info.play.encoding    = defaultEncoding;

  a_info.play.buffer_size = NumChunks * AUDIO_BUFFER_SIZE;
  audioBufferSize	  = AUDIO_BUFFER_SIZE;
  audioLowWaterMark	  = 2;

  if(ioctl(audio_fd, AUDIO_SETINFO, &a_info) == -1)
  {
    if(DEBUG_HPUX)
      fprintf(stderr, "Warning: Cannot set audio parameters.\n");

    return false;
  }

  if(DEBUG_HPUX)
    fprintf(stderr, "Audio initialised. Setting up queues...\n");

  if (pipe(fd)<0) {
    closeAudio();
    return false;
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
  audio_ready = true;

  if(DEBUG_HPUX)
    fprintf(stderr, "Audio ready.\n");

  return true;
  */
}


void			HPUXMedia::closeAudio()
{/*
  close(audio_fd);
  close(audioctl_fd);
  */
}

bool			HPUXMedia::startAudioThread(
				void (*proc)(void*), void* data)
{
  // if no audio thread then just call proc and return
  if (!hasAudioThread()) {
    proc(data);
    return true;
  }

  /*
  // has an audio thread so fork and call proc
  if (childProcID) return true;
  if ((childProcID=fork()) > 0) {
    close(queueOut);
    close(audio_fd);
    return true;
  }
  else if (childProcID < 0) {
    return false;
  }
  close(queueIn);
  proc(data);
  exit(0);
  */
}

void			HPUXMedia::stopAudioThread()
{
  //if (childProcID != 0) kill(childProcID, SIGTERM);
  //childProcID=0;
}

bool			HPUXMedia::hasAudioThread() const
{
  // XXX -- adjust this if the system always uses or never uses a thread
//#if defined(NO_AUDIO_THREAD)
  return false;
//#else
  //return true;
//#endif
}

void			HPUXMedia::writeSoundCommand(const void* cmd, int len)
{/*
  if (!audio_ready) return;
  write(queueIn, cmd, len);
*/}

bool			HPUXMedia::readSoundCommand(void* cmd, int len)
{
   return false;
//  return (read(queueOut, cmd, len)==len);
}

int			HPUXMedia::getAudioOutputRate() const
{
  return defaultAudioRate;
}

int			HPUXMedia::getAudioBufferSize() const
{
  return 0; //NumChunks * (audioBufferSize >> 1);
}

int			HPUXMedia::getAudioBufferChunkSize() const
{
  return 0; //audioBufferSize>>1;
}

bool			HPUXMedia::isAudioTooEmpty() const
{
   return false;
  //ioctl(audioctl_fd, AUDIO_GETINFO, &a_info);
  //return ((int)a_info.play.eof >= eof_written - audioLowWaterMark);
}

void			HPUXMedia::writeAudioFrames(
				const float* samples, int numFrames)
{
   /*
  int numSamples = 2 * numFrames;
  int limit;

  while (numSamples > 0) {
    if (numSamples>512) limit=512;
    else limit=numSamples;
    for (int j = 0; j < limit; j++) {
      if (samples[j] < -32767.0) tmp_buf[j] = -32767;
      else if (samples[j] > 32767.0) tmp_buf[j] = 32767;
      else tmp_buf[j] = short(samples[j]);
    }

    // fill out the chunk (we never write a partial chunk)
    if (limit < 512) {
      for (int j = limit; j < 512; ++j)
	tmp_buf[j] = 0;
      limit = 512;
    }

    write(audio_fd, tmp_buf, 2*limit);
    write(audio_fd, NULL, 0);
    samples += limit;
    numSamples -= limit;
    eof_written++;
  }
*/}

void			HPUXMedia::audioSleep(
				bool checkLowWater, double endTime)
{
/*  fd_set commandSelectSet;
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
*/}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
