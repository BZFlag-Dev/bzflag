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

// template for new platform media code.  it may be more helpful
// to use code for an existing platform as a template.

#include "NewMedia.h"

#if defined(HALF_RATE_AUDIO)
static const int defaultAudioRate=11025;
#else
static const int defaultAudioRate=22050;
#endif

//
// NewMedia
//

NewMedia::NewMedia() : BzfMedia()
{
  // do nothing
}

NewMedia::~NewMedia()
{
  // do nothing
}

double			NewMedia::stopwatch(bool start)
{
  if (start) {
    // XXX -- save current time
    return 0.0;
  }
  // XXX -- return time since last stopwatch(true)
  return 0.0;
}

bool			NewMedia::openAudio()
{
  // XXX -- open audio device and prepare for IPC with audio thread
  return false;
}

void			NewMedia::closeAudio()
{
  // XXX -- shut down audio device and IPC stuff
}

bool			NewMedia::startAudioThread(
				void (*proc)(void*), void* data)
{
  // if no audio thread then just call proc and return
  if (!hasAudioThread()) {
    proc(data);
    return true;
  }

  // XXX -- spawn a thread that calls proc, passing it data.
  // we will need to communicate with this thread.  return true
  // if successfully started.
  return false;
}

void			NewMedia::stopAudioThread()
{
  // XXX -- terminate audio thread
}

bool			NewMedia::hasAudioThread() const
{
  // XXX -- adjust this if the system always uses or never uses a thread
#if defined(NO_AUDIO_THREAD)
  return false;
#else
  return true;
#endif
}

void			NewMedia::writeSoundCommand(const void* cmd, int len)
{
  // XXX -- send a command to the audio thread
}

bool			NewMedia::readSoundCommand(void* cmd, int len)
{
  // XXX -- read a sent command
  return false;
}

int			NewMedia::getAudioOutputRate() const
{
  // XXX -- return the audio output sample rate
  return 0;
}

int			NewMedia::getAudioBufferSize() const
{
  // XXX -- return the total size of the audio buffer in frames
  return 0;
}

int			NewMedia::getAudioBufferChunkSize() const
{
  // XXX -- return the size of an audio buffer chunk.  audio is
  // broken into chunks for latency reasons.  if we sent an entire
  // sound to the audio subsystem at once then that sound wouldn't
  // be affected by changes to the audio filter (which can change
  // at any time) and we wouldn't be able to mix in new sounds that
  // start while the old sound is playing.
  //
  // so we break the audio buffer into several equal sized chunks.
  // at any time, the audio subsystem is playing one chunk, another
  // chunk is ready and waiting to be played, and other chunks are
  // being created.  it's important that the chunk after the one
  // being played is ready to avoid pops and dropouts in the sound.
  // so at least 3 chunks are needed for reliable sound.
  return 0;
}

bool			NewMedia::isAudioTooEmpty() const
{
  // XXX -- return true if the audio subsystem has fallen below
  // the low water mark of samples.  that is, when the number of
  // samples left to play has fallen low enough that we should
  // start preparing more audio chunks.
  return false;
}

void			NewMedia::writeAudioFrames(
				const float* samples, int numFrames)
{
  // XXX -- write the given samples to the audio subsystem.  a
  // frame is one sample for mono sound and two samples for
  // stereo sound (one for each channel).
}

void			NewMedia::audioSleep(
				bool checkLowWater, double endTime)
{
  // XXX -- wait for an audio command to appear.  stop waiting
  // if checkLowWater is true and the audio has drained to the
  // low water mark.  stop waiting if endTime >= 0.0 and
  // endTime seconds have passed since this method was called.
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
