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

#include "BeOSMedia.h"
#include "BzfMedia.h"
#include "ErrorHandler.h"
#include <MediaDefs.h>
#include <math.h>
#include <fcntl.h>
#include <endian.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define USE_FLOAT
//#define HALF_RATE_AUDIO

#ifdef HALF_RATE_AUDIO
static const int defaultAudioRate=11025;
#else
static const int defaultAudioRate=22050;
#endif

// debug fd
int debugWav = 0;

static const int	NumChunks = 4;

#define AUDIO_BLOCK_FRAME_COUNT 1024
#ifdef USE_FLOAT
#define AUDIO_BLOCK_SIZE (AUDIO_BLOCK_FRAME_COUNT * sizeof(float))
#else
#define AUDIO_BLOCK_SIZE (AUDIO_BLOCK_FRAME_COUNT * sizeof(short))
#endif
#define AUDIO_BLOCK_COUNT 8

#define AUDIO_BUFFER_SIZE (AUDIO_BLOCK_SIZE*AUDIO_BLOCK_COUNT)

BeOSMedia::BeOSMedia() :
		audioReady(false),
		audioBufferSize(AUDIO_BUFFER_SIZE),
		audioQueuePort(-1),
		audioQueueMaxCmds(10),
		outputBuffer(NULL),
		childThreadID(0),
		soundPlayer(NULL),
		audioHasQuit(false),
		checkLowWater(false),
		audioInputSem(-1),
		audioInputIndex(0),
		audioOutputSem(-1),
		audioOutputIndex(0)
{
  // do nothing
}

BeOSMedia::~BeOSMedia()
{
  // do nothing
}

bool					BeOSMedia::openAudio()
{
  // don't re-initialize
  if (audioReady)
    return false;

//debugWav = open("/boot/home/bzflagdbgwav.dat", O_WRONLY);

  audioQueuePort = create_port(audioQueueMaxCmds, "bzflag_audio_cmd_port");
  if (audioQueuePort < B_OK)
    return false;

  media_raw_audio_format format;

  audioInputSem = create_sem(AUDIO_BUFFER_SIZE, "bzflag_ringbuffer_input");
  if (audioInputSem < B_OK) {
    closeAudio();
    return false;
  }
  audioOutputSem = create_sem(0, "bzflag_ringbuffer_output");
  if (audioOutputSem < B_OK) {
    closeAudio();
    return -EIO;
  }
  audioInputIndex = 0;
  audioOutputIndex = 0;
  //queued = 0;

  // make an output buffer
  outputBuffer = (void *)malloc(audioBufferSize);
  if (outputBuffer == NULL) {
    closeAudio();
    return false;
  }

    
  format = media_raw_audio_format::wildcard;
#ifdef USE_FLOAT
  format.format = media_raw_audio_format::B_AUDIO_FLOAT;
#else
  format.format = media_raw_audio_format::B_AUDIO_SHORT;
#endif
  format.byte_order = B_HOST_IS_LENDIAN ? B_MEDIA_LITTLE_ENDIAN : B_MEDIA_BIG_ENDIAN;
  format.channel_count = 2;
  format.buffer_size = AUDIO_BLOCK_SIZE;
  format.frame_rate = defaultAudioRate;
  soundPlayer = new BSoundPlayer(&format, "bzflag output", audioplay_callback);
  if (soundPlayer->InitCheck() != B_OK) {
    closeAudio();
    return false;
  }
  soundPlayer->SetCookie((void *)this);
//  soundPlayer->SetVolume(1.0);
  soundPlayer->SetVolume(0.1);
  soundPlayer->Start();
  //soundPlayer->SetHasData(true); /* delay playback until the first frame */

  // Set default no thread
  childThreadID = 0;

  // ready to go
  audioReady = true;
  return true;
}

/* called back by BSoundPlayer */
void BeOSMedia::audioplay_callback(void *cookie, void *buffer, size_t bufferSize, const media_raw_audio_format &format)
{
  status_t err;
  BeOSMedia *s;
  size_t len, amount;
  unsigned char *buf = (unsigned char *)buffer;

//printf("audio_callback(, , %d, )\n", bufferSize);
  s = (BeOSMedia *)cookie;
  if (s->audioHasQuit)
    return;
  while (bufferSize > 0) {
    len = MIN(AUDIO_BLOCK_SIZE, bufferSize);
    err = acquire_sem_etc(s->audioOutputSem, len, B_CAN_INTERRUPT | B_RELATIVE_TIMEOUT, 5000LL);
//puts("audio callback unlocked");
    if (err < B_OK) {
      if (err == B_TIMED_OUT) {
//puts("audio callback late");
        /* tell we are late */
        if (s->checkLowWater)
          write_port(s->audioQueuePort, 'Late', NULL, 0);
        continue;
        //return;
      }
      s->audioHasQuit = 1;
      s->soundPlayer->SetHasData(false);
      return;
    }
    amount = MIN(len, (AUDIO_BUFFER_SIZE - s->audioOutputIndex));
    memcpy(buf, &((unsigned char *)s->outputBuffer)[s->audioOutputIndex], amount);
    s->audioOutputIndex += amount;
    if (s->audioOutputIndex >= AUDIO_BUFFER_SIZE) {
      s->audioOutputIndex %= AUDIO_BUFFER_SIZE;
      memcpy(buf + amount, &((unsigned char *)s->outputBuffer)[s->audioOutputIndex], len - amount);
      s->audioOutputIndex += len-amount;
      s->audioOutputIndex %= AUDIO_BUFFER_SIZE;
    }
    release_sem_etc(s->audioInputSem, len, 0);
    buf += len;
    bufferSize -= len;
  }
//  buf = (unsigned char *)buffer;
//  printf("cbbuff= %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
//  write(debugWav, buf, 2048);
}

void					BeOSMedia::closeAudio()
{
  if (audioQueuePort > B_OK)
    delete_port(audioQueuePort);
  audioQueuePort = -1;
  if (audioInputSem > B_OK)
    delete_sem(audioInputSem);
  audioInputSem = -1;
  if (audioOutputSem > B_OK)
    delete_sem(audioOutputSem);
  audioHasQuit = true;
	
  if (soundPlayer) {
    soundPlayer->Stop();
    delete soundPlayer;
  }
  soundPlayer = NULL;
	
  if (outputBuffer)
    free(outputBuffer);
  outputBuffer = NULL;
	
  audioOutputSem = -1;
  audioInputIndex = 0;
  audioOutputIndex = 0;
}

bool					BeOSMedia::startAudioThread(
								void (*proc)(void*), void* data)
{
  // if no audio thread then just call proc and return
  if (!hasAudioThread()) {
    proc(data);
    return true;
  }

  // has an audio thread so fork and call proc
  if (childThreadID)
    return true;
  childThreadID = spawn_thread((thread_func) proc, "bzflag_audio_thread", B_URGENT_DISPLAY_PRIORITY, data);
  if (childThreadID < B_OK)
    return false;
  resume_thread(childThreadID);
  return true;
}

void					BeOSMedia::stopAudioThread()
{
  if (childThreadID > B_OK) {
    status_t err;
    kill(childThreadID, SIGTERM);
    wait_for_thread(childThreadID, &err);
  }
  childThreadID = 0;
}

bool					BeOSMedia::hasAudioThread() const
{
#if defined(NO_AUDIO_THREAD)
  return false;
#else
  return true;
#endif
}

static void				die(int)
{
  exit_thread(B_OK);
}

void					BeOSMedia::audioThreadInit(void*)
{
  // parent will kill me when it wants me to quit.  catch the signal
  // and gracefully exit.  don't use PlatformFactory because that
  // doesn't distinguish between processes.
  signal(SIGTERM, die);
}

void					BeOSMedia::writeSoundCommand(const void* cmd, int len)
{
  if (port_count(audioQueuePort) + 2 < audioQueueMaxCmds) /* we don't want to block */
    write_port(audioQueuePort, 'SndC', cmd, len);
}

bool					BeOSMedia::readSoundCommand(void* cmd, int len)
{
//  assert((size_t)len < sizeof(cmdBuffer));

  int32 what = 0;
  while (what != 'SndC') {
    if (!port_count(audioQueuePort))
      return false;
    read_port(audioQueuePort, &what, cmd, len);
  }
  return true;
}

int						BeOSMedia::getAudioOutputRate() const
{
  return audioOutputRate;
}

int						BeOSMedia::getAudioBufferSize() const
{
  return AUDIO_BLOCK_FRAME_COUNT * AUDIO_BLOCK_COUNT;
}

int						BeOSMedia::getAudioBufferChunkSize() const
{
  return AUDIO_BLOCK_FRAME_COUNT;
}

bool					BeOSMedia::isAudioTooEmpty() const
{
  return true;
  int32 semcnt = 0;
//printf("BeOSMedia::isAudioTooEmpty()\n");
  get_sem_count(audioOutputSem, &semcnt); // XXX: make sure
  return (semcnt < -(AUDIO_BLOCK_FRAME_COUNT / 2));
}

void					BeOSMedia::writeAudioFrames(
								const float* samples, int numFrames)
{
  int len, ret;
  numFrames *= 2;
  int size = numFrames * sizeof(short);
  uint8 *buf = (uint8 *)samples;
	
//printf("BeOSMedia::writeAudioFrames(, %d)\n", numFrames);
//printf("inbuff= %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);

//  if (!soundPlayer->HasData()) /* delay playback until the first frame */
//    soundPlayer->SetHasData(true);

  while (size > 0) {
    int amount;
    len = MIN(size, AUDIO_BLOCK_SIZE);
    if (acquire_sem_etc(audioInputSem, len, B_CAN_INTERRUPT, 0LL) < B_OK)
      return;
    amount = MIN(len, (AUDIO_BUFFER_SIZE - audioInputIndex));
    memcpy(&((unsigned char *)outputBuffer)[audioInputIndex], buf, amount);
    audioInputIndex += amount;
    if (audioInputIndex >= AUDIO_BUFFER_SIZE) {
      audioInputIndex %= AUDIO_BUFFER_SIZE;

#ifndef USE_FLOAT
      float *s = (float *)(uint8 *)(buf + amount);
      short *d = (short *)&((unsigned char *)outputBuffer)[audioInputIndex];
      for (int i = (len - amount)/sizeof(short); i; i++) {
        if (*s < -32767) *d = -32767;
        else if (*s > 32767) *d = 32767;
        else *d = (short)(*s /** 32767*/);
        d++; s++;
      }
#else
      float *s = (float *)(uint8 *)(buf + amount);
      for (int i = (len - amount)/sizeof(short); i; i++, s++)
      	*s /= 32767;
      memcpy(&((unsigned char *)outputBuffer)[audioInputIndex], buf + amount, len - amount);
#endif
      audioInputIndex += len - amount;
    }
    release_sem_etc(audioOutputSem, len, 0);
    buf += len;
    size -= len;
  }
}

void					BeOSMedia::audioSleep(
								bool checkLowWater, double endTime)
{
//printf("BeOSMedia::audioSleep(%s, %f)\n", checkLowWater?"true":"false", (float)endTime);
  this->checkLowWater = checkLowWater;
  bigtime_t timeout = (bigtime_t)(1.0e6 * endTime);
  int32 flags = (endTime < 0)?0:B_RELATIVE_TIMEOUT;
  if (endTime < 0)
    timeout = B_INFINITE_TIMEOUT;
  port_buffer_size_etc(audioQueuePort, flags, timeout);
}

void					BeOSMedia::sleep(float timeInSeconds)
{
  snooze((bigtime_t)(1.0e6 * timeInSeconds));
}
// ex: shiftwidth=2 tabstop=8
