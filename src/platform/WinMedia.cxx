/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// Jeff Myers 10/13/97 changed direct sound cooperative level to
//	exclusive for compatibility with NT.

#include "WinMedia.h"
#include "WinWindow.h"
#include "TimeKeeper.h"
#include "Pack.h"
#include <stdio.h>

#ifdef HAVE_DSOUND_H

static const int	defaultOutputRate = 22050;
static const int	NumChunks = 4;
void			(*WinMedia::threadProc)(void*);
void*			WinMedia::threadData;

WinMedia::WinMedia(WinWindow* _window) :
				window(_window->getHandle()),
				audioReady(false),
				audioInterface(NULL),
				audioPrimaryPort(NULL),
				audioPort(NULL),
				outputBuffer(NULL),
				audioCommandBuffer(NULL),
				audioCommandEvent(NULL),
				audioCommandMutex(NULL),
				audioThread(NULL)
{
}

WinMedia::~WinMedia()
{
}

bool			WinMedia::openAudio()
{
  // don't re-initialize
  if (audioReady) return false;

  // create DirectSound interface pointer
  if (DirectSoundCreate(NULL, &audioInterface, NULL) != DS_OK)
    return false;

  // set cooperative level
  if (audioInterface->SetCooperativeLevel(window, DSSCL_EXCLUSIVE) != DS_OK) {
    closeAudio();
    return false;
  }

  // create audio command queue
  audioCommandBufferHead = 0;
  audioCommandBufferTail = 0;
  audioCommandBufferLen = 4096;
  audioCommandBuffer = new unsigned char[audioCommandBufferLen];
  audioCommandEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  audioCommandMutex = CreateMutex(NULL, FALSE, NULL);

  // compute desired configuration
  audioNumChannels = 2;
  audioOutputRate = getAudioOutputRate();
  audioBufferChunkSize = (long)(0.1 * audioOutputRate) & ~1;
  audioLowWaterMark = (NumChunks - 1) * audioBufferChunkSize;
  audioBufferSize = NumChunks * audioBufferChunkSize;
  audioBytesPerSample = 2;
  audioBytesPerFrame = audioNumChannels * audioBytesPerSample;

  // create a `primary sound buffer';  we only need this to force it
  // to play at all times (presumably decreasing latency on secondary
  // buffers).
  DSBUFFERDESC audioBufferParams;
  audioBufferParams.dwSize = sizeof(audioBufferParams);
  audioBufferParams.dwBufferBytes = 0;
  audioBufferParams.dwReserved = 0;
  audioBufferParams.lpwfxFormat = NULL;
  audioBufferParams.dwFlags = DSBCAPS_PRIMARYBUFFER |
				DSBCAPS_CTRLFREQUENCY |
				DSBCAPS_GETCURRENTPOSITION2;
  HRESULT status = audioInterface->CreateSoundBuffer(&audioBufferParams,
						&audioPrimaryPort, NULL);
  if (FAILED(status)) {
    audioBufferParams.dwFlags = DSBCAPS_PRIMARYBUFFER |
				DSBCAPS_GETCURRENTPOSITION2;
    status = audioInterface->CreateSoundBuffer(&audioBufferParams,
						&audioPrimaryPort, NULL);
  }
  if (FAILED(status)) {
    audioBufferParams.dwFlags = DSBCAPS_PRIMARYBUFFER;
    status = audioInterface->CreateSoundBuffer(&audioBufferParams,
						&audioPrimaryPort, NULL);
  }
  if (FAILED(status)) {
    closeAudio();
    return false;
  }

  // haven't started playing yet
  audioPlaying = false;

  // have written zero samples so far
  audioWritePtr = 0;

  // create a `secondary sound buffer';  we need a buffer large enough
  // for 3 audioBufferSize chunks.  we'll fill it up then wait for it
  // to drain below the audioLowWaterMark, then fill it up again.
  WAVEFORMATEX waveFormat;
  waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveFormat.nChannels = audioNumChannels;
  waveFormat.nSamplesPerSec = audioOutputRate;
  waveFormat.nBlockAlign = audioBytesPerFrame;
  waveFormat.nAvgBytesPerSec = audioBytesPerFrame * waveFormat.nSamplesPerSec;
  waveFormat.wBitsPerSample = 8 * audioBytesPerSample;
  waveFormat.cbSize = 0;
  audioBufferParams.dwSize = sizeof(audioBufferParams);
  audioBufferParams.dwBufferBytes = audioBufferSize * audioBytesPerSample;
  audioBufferParams.dwReserved = 0;
  audioBufferParams.lpwfxFormat = &waveFormat;
  audioBufferParams.dwFlags = DSBCAPS_CTRLFREQUENCY |
				DSBCAPS_GETCURRENTPOSITION2;
  status = audioInterface->CreateSoundBuffer(&audioBufferParams,
							&audioPort, NULL);
  if (FAILED(status)) {
    audioBufferParams.dwFlags = DSBCAPS_CTRLFREQUENCY;
    status = audioInterface->CreateSoundBuffer(&audioBufferParams,
						&audioPort, NULL);
  }
  if (FAILED(status)) {
    audioBufferParams.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
    status = audioInterface->CreateSoundBuffer(&audioBufferParams,
						&audioPort, NULL);
  }
  if (FAILED(status)) {
    audioBufferParams.dwFlags = 0;
    status = audioInterface->CreateSoundBuffer(&audioBufferParams,
						&audioPort, NULL);
  }
  if (FAILED(status)) {
    closeAudio();
    return false;
  }
//  audioPrimaryPort->Play(0, 0, DSBPLAY_LOOPING);

  // make an output buffer
  outputBuffer = new short[audioBufferChunkSize];

  audioReady = true;
  return true;
}

void			WinMedia::closeAudio()
{
  if (!audioReady) return;

  // release memory
  delete[] outputBuffer;

  // shut down audio
  if (audioPort) {
    audioPort->Stop();
    audioPort->Release();
  }
  if (audioPrimaryPort) {
    audioPrimaryPort->Stop();
    audioPrimaryPort->Release();
  }
  if (audioInterface) audioInterface->Release();

  // close audio command queue
  if (audioCommandEvent) CloseHandle(audioCommandEvent);
  if (audioCommandMutex) CloseHandle(audioCommandMutex);
  delete[] audioCommandBuffer;

  audioReady = false;
  audioInterface = NULL;
  audioPrimaryPort = NULL;
  audioPort = NULL;
  outputBuffer = NULL;
}

bool			WinMedia::startAudioThread(
				void (*proc)(void*), void* data)
{
  if (audioThread || !proc) return false;
  threadProc = proc;
  threadData = data;

  // create thread
  DWORD dummy;
  audioThread = CreateThread(NULL, 0, audioThreadInit, NULL, 0, &dummy);
  return (audioThread != NULL);
}

void			WinMedia::stopAudioThread()
{
  if (!audioThread) return;

  // wait for thread to terminate (don't wait forever, though)
  if (WaitForSingleObject(audioThread, 5000) != WAIT_OBJECT_0)
    TerminateThread(audioThread, 0);

  // free thread
  CloseHandle(audioThread);
  audioThread = NULL;
}

bool			WinMedia::hasAudioThread() const
{
  return true;
}

DWORD WINAPI		WinMedia::audioThreadInit(void*)
{
  // boost the audio thread's priority
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

  // call user routine
  (*threadProc)(threadData);
  return 0;
}

void			WinMedia::writeSoundCommand(const void* msg, int len)
{
  if (!audioReady) return;

  // take ownership of the buffer.  if we can't then give up.
  if (WaitForSingleObject(audioCommandMutex, INFINITE) != WAIT_OBJECT_0)
    return;

  // ignore if buffer is overflowing.  should expand buffer but if
  // overflowing then the consumer is probably dead anyway.
  long queueFilled = audioCommandBufferHead - audioCommandBufferTail;
  if (queueFilled < 0) queueFilled += audioCommandBufferLen;
  if (queueFilled + len < audioCommandBufferLen) {
    // add command to queue
    const long endLen = audioCommandBufferLen - audioCommandBufferHead;
    if (endLen < len) {
      memcpy(audioCommandBuffer + audioCommandBufferHead, msg, endLen);
      memcpy(audioCommandBuffer, (char*)msg + endLen, len - endLen);
      audioCommandBufferHead = len - endLen;
    }
    else {
      memcpy(audioCommandBuffer + audioCommandBufferHead, msg, len);
      audioCommandBufferHead += len;
    }

    // signal non-empty queue
    SetEvent(audioCommandEvent);
  }

  // release hold of buffer
  ReleaseMutex(audioCommandMutex);
}

bool			WinMedia::readSoundCommand(void* msg, int len)
{
  // no event unless signaled non-empty
  if (WaitForSingleObject(audioCommandEvent, 0) != WAIT_OBJECT_0)
    return false;

  // take ownership of the buffer.  if we can't then give up after
  // resetting the event flag.
  if (WaitForSingleObject(audioCommandMutex, INFINITE) != WAIT_OBJECT_0) {
    ResetEvent(audioCommandEvent);
    return false;
  }

  // read message
  const long endLen = audioCommandBufferLen - audioCommandBufferTail;
  if (endLen < len) {
    memcpy(msg, audioCommandBuffer + audioCommandBufferTail, endLen);
    memcpy((char*)msg + endLen, audioCommandBuffer, len - endLen);
    audioCommandBufferTail = len - endLen;
  }
  else {
    memcpy(msg, audioCommandBuffer + audioCommandBufferTail, len);
    audioCommandBufferTail += len;
  }

  // clear event if no more commands pending
  if (audioCommandBufferTail == audioCommandBufferHead)
    ResetEvent(audioCommandEvent);

  // release hold of buffer
  ReleaseMutex(audioCommandMutex);

  return true;
}

int			WinMedia::getAudioOutputRate() const
{
#if defined(HALF_RATE_AUDIO)
  return defaultOutputRate / 2;
#else
  return defaultOutputRate;
#endif
}

int			WinMedia::getAudioBufferSize() const
{
  return audioBufferSize / audioNumChannels;
}

int			WinMedia::getAudioBufferChunkSize() const
{
  return audioBufferChunkSize / audioNumChannels;
}

bool			WinMedia::isAudioTooEmpty() const
{
  // the write offset returned by GetCurrentPosition() is probably
  // useless.  the documentation certainly is.
  DWORD playOffset, writeOffset;
  if (audioPort->GetCurrentPosition(&playOffset, &writeOffset) == DS_OK) {
    const int playSamples = playOffset / audioBytesPerSample;
    int samplesLeft = audioWritePtr - playSamples;
    if (samplesLeft < 0) samplesLeft += audioBufferSize;
    return samplesLeft <= audioLowWaterMark;
  }
  return false;
}

void			WinMedia::writeAudioFrames(
				const float* samples, int numFrames)
{
  // ignore empty buffers
  if (numFrames == 0) return;

  // first convert the samples to 16 bits signed integers.
  // NOTE -- truncate sample buffer if too long.  this is not good,
  // but we won't send too many samples so we don't care.
  int numSamples = audioNumChannels * numFrames;
  if (numSamples > audioBufferChunkSize) numSamples = audioBufferChunkSize;
  for (int j = 0; j < numSamples; j++) {
    if (samples[j] < -32767.0f) outputBuffer[j] = -32767;
    else if (samples[j] > 32767.0f) outputBuffer[j] = 32767;
    else outputBuffer[j] = (short)samples[j];
  }

  // lock enough of the buffer for numFrames, starting at the write
  // pointer.  it's worth noting here that the DirectSound API really
  // sucks -- it doesn't abstract away the circularity of the sample
  // buffer so we get possibly two pointers to write to, the system
  // may capriciously take our sound buffer away so that we have to
  // restore it ourselves, the writeOffset returned by
  // GetCurrentPosition() is complete fiction so we have to maintain
  // it ourselves, and the lock-modify-unlock scheme is totally brain
  // dead.  *all* this can be fixed with a single function:
  // WriteSamples();  it would write just past where we wrote last
  // time, automatically restore the buffer, handle the `circularity'
  // of the sample buffer, and then DirectSound wouldn't be prone to
  // attempts to write at locations that are being played.  And,
  // surprise!, WriteSamples() is easier to understand, use, and
  // implement.
  void* ptr1, *ptr2;
  DWORD size1, size2;
  HRESULT result = audioPort->Lock(audioWritePtr * audioBytesPerSample,
				numSamples * audioBytesPerSample,
				&ptr1, &size1, &ptr2, &size2, 0);
  if (result == DSERR_BUFFERLOST) {
    // system took away our buffer.  try to get it back.
    result = audioPort->Restore();

    // if we got it back then try the lock again
    if (result == DS_OK)
      result = audioPort->Lock(audioWritePtr * audioBytesPerSample,
				numSamples * audioBytesPerSample,
				&ptr1, &size1, &ptr2, &size2, 0);
  }

  // if we got the lock the write the samples
  if (result == DS_OK) {
    memcpy(ptr1, outputBuffer, size1);
    if (ptr2)
      memcpy(ptr2, outputBuffer + size1 / audioBytesPerSample, size2);

    // now unlock the buffer
    audioPort->Unlock(ptr1, size1, ptr2, size2);

    if (!audioPlaying) {
      audioPlaying = true;
      audioPort->Play(0, 0, DSBPLAY_LOOPING);
    }

    // record how many samples written
    audioWritePtr += numSamples;
    while (audioWritePtr >= audioBufferSize)
      audioWritePtr -= audioBufferSize;
  }
}

void			WinMedia::audioSleep(
				bool checkLowWater, double maxTime)
{
  // wait for a message on the command queue.  do this by waiting
  // for the command queue event.
  //
  // not suprisingly, DirectSound is missing a critical bit of
  // functionality -- the ability to sleep until the sound buffer
  // has drained below some threshold.  we need to simulate that
  // behavior here if checkLowWater is true by sleeping briefly
  // then checking if the buffer has drained.
  //
  // never wait longer than maxTime seconds.

  if (checkLowWater) {
    // start looping
    TimeKeeper start = TimeKeeper::getCurrent();
    do {
      // break if buffer has drained enough
      if (isAudioTooEmpty())
	break;

      // wait.  break if command was sent.
      if (WaitForSingleObject(audioCommandEvent, 1) == WAIT_OBJECT_0)
	break;

    } while (maxTime < 0.0 || (TimeKeeper::getCurrent() - start) < maxTime);
  }

  else {
    DWORD timeout = (maxTime >= 0.0) ? (DWORD)(maxTime * 1000.0) : INFINITE;
    WaitForSingleObject(audioCommandEvent, timeout);
  }
}
#endif
// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
