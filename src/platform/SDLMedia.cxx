/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

#ifdef HAVE_SDL
#include "SDLMedia.h"
#include "ErrorHandler.h"

#ifdef HALF_RATE_AUDIO
static const int defaultAudioRate=11025;
#else
static const int defaultAudioRate=22050;
#endif

//
// SDLMedia
//

SDLMedia::SDLMedia() : BzfMedia()
{
  cmdFill       = 0;
  sndMutex      = SDL_CreateMutex();
  cmdMutex      = SDL_CreateMutex();
  fillCond      = SDL_CreateCond();
  wakeCond      = SDL_CreateCond();
  filledBuffer  = false;
  audioReady    = false;
  waitingData   = false;
  waitingWake   = false;
}

SDLMedia::~SDLMedia()
{
  SDL_DestroyCond(fillCond);
  SDL_DestroyCond(wakeCond);
  SDL_DestroyMutex(sndMutex);
}

double			SDLMedia::stopwatch(bool start)
{
  Uint32 currentTick = SDL_GetTicks(); //msec

  if (start) {
    stopwatchTime = currentTick;
    return 0.0;
  }
  if (currentTick >= stopwatchTime)
    return (double) (currentTick - stopwatchTime) * 0.001; // sec
  else
    //Clock is wrapped : happens after 49 days
    //Should be "wrap value" - stopwatchtime. Now approx.
    return (double) currentTick * 0.001;
}

void			SDLMedia::sleep(float timeInSeconds)
{
  // Not used ... however ... here it is
  SDL_Delay((Uint32) (timeInSeconds * 1000.0));
}

bool			SDLMedia::openAudio()
{
  // don't re-initialize
  if (audioReady) return false;

  if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
    printFatalError("Could not initialize SDL-Audio: %s.\n", SDL_GetError());
    exit(-1);
  }; 

  static SDL_AudioSpec desired;
 
  // what the frequency?
  audioOutputRate = defaultAudioRate;

  // how big a fragment to use?  we want to hold at around 1/10th of
  // a second.
  int fragmentSize = (int)(0.08f * (float)audioOutputRate);
  int n;

  n = 0;
  while ((1 << n) < fragmentSize)
    ++n;

  // samples are two bytes each so double the size
  audioBufferSize = 1 << (n + 1);

  desired.freq       = audioOutputRate;
  desired.format     = AUDIO_S16SYS;
  desired.channels   = 2;
  desired.samples    = audioBufferSize >> 1; // In stereo samples
  desired.callback   = &fillAudioWrapper;
  desired.userdata   = (void *) this;        // To handle Wrap of func

  /* Open the audio device, forcing the desired format */
  if (SDL_OpenAudio(&desired, NULL) < 0) {
    fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
    return false;
  }

  // make an output buffer
  outputBuffer  = new short[audioBufferSize];

  // Stop sending silence and start calling audio callback
  SDL_PauseAudio(0);

  // ready to go
  audioReady = true;

  return true;
}

void			SDLMedia::closeAudio()
{
  // Stop Audio to avoid callback
  SDL_PauseAudio(1);

  // Trying to cleanly wake up and exit the eventually running callback
  if (SDL_mutexP(sndMutex) == -1) {
    fprintf(stderr, "Couldn't lock mutex\n");
    exit(-1);
  }
  filledBuffer  = true;
  if (waitingData)
    // Signal data are there - fake data but who cares
    if (SDL_CondSignal(fillCond) == -1) {
      fprintf(stderr, "Couldn't Cond Signal\n");
      exit(-1);
    };
  if (SDL_mutexV(sndMutex) == -1) {
    fprintf(stderr, "Couldn't unlock mutex\n");
    exit(-1);
  }
  
  SDL_CloseAudio();
  delete [] outputBuffer;
  outputBuffer  = 0;
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
  audioReady    = false;
}

bool			SDLMedia::startAudioThread(
				void (*proc)(void*), void* data)
{
  ThreadId = SDL_CreateThread( (int (*) (void *))proc, data);
  return ThreadId != NULL;
}

void			SDLMedia::stopAudioThread()
{
  SDL_WaitThread(ThreadId, NULL);
}

bool			SDLMedia::hasAudioThread() const
{
  return true;
}

void			SDLMedia::writeSoundCommand(const void* cmd, int len)
{
  if (!audioReady) return;

   if (SDL_mutexP(cmdMutex) == -1) {
     fprintf(stderr, "Couldn't lock mutex\n");
     exit(-1);
   }
   // Discard command if full
   if ((cmdFill + len) < 2048) {
     memcpy(&cmdQueue[cmdFill], cmd, len);
     // We should awake audioSleep - but game become unplayable
     // using here an SDL_CondSignal(wakeCond)
     cmdFill += len;
   }
   if (SDL_mutexV(cmdMutex) == -1) {
     fprintf(stderr, "Couldn't unlock mutex\n");
     exit(-1);
   }
}

bool			SDLMedia::readSoundCommand(void* cmd, int len)
{
  bool result = false;

  if (SDL_mutexP(cmdMutex) == -1) {
    fprintf(stderr, "Couldn't lock mutex\n");
    exit(-1);
  }
  if (cmdFill >= len) {
    memcpy(cmd, cmdQueue, len);
    // repack list of command waiting to be processed
    memmove(cmdQueue, &cmdQueue[len], cmdFill - len);
    cmdFill -= len;
    result = true;
  }
  if (SDL_mutexV(cmdMutex) == -1) {
    fprintf(stderr, "Couldn't unlock mutex\n");
    exit(-1);
  }
  return result;
}

int			SDLMedia::getAudioOutputRate() const
{
  return audioOutputRate;
}

int			SDLMedia::getAudioBufferSize() const
{
  return audioBufferSize;
}

int			SDLMedia::getAudioBufferChunkSize() const
{
  return audioBufferSize>>1;
}

bool			SDLMedia::isAudioTooEmpty() const
{
  return !filledBuffer;
}

void SDLMedia::fillAudio (Uint8 * stream, int len)
{
  Uint8* soundBuffer        = stream;

  if (SDL_mutexP(sndMutex) == -1) {
    fprintf(stderr, "Couldn't lock mutex\n");
    exit(-1);
  }

  while (!filledBuffer) {
    // Hurry up. We need data soon
    waitingData = true;
    if (waitingWake)
      // Signal we are waiting for data and
      if (SDL_CondSignal(wakeCond) == -1) {
	fprintf(stderr, "Couldn't Cond Signal\n");
	exit(-1);
      };
    
    // wait for someone to fill data
    while (SDL_CondWait(fillCond, sndMutex) == -1) ;
  }
  waitingData = false;

  if (SDL_mutexV(sndMutex) == -1) {
    fprintf(stderr, "Couldn't unlock mutex\n");
    exit(-1);
  }

  int transferSize = (audioBufferSize - sampleToSend) * 2;
  if (transferSize > len)
    transferSize   = len;
  // just copying into the soundBuffer is enough, SDL is looking for
  // something different from silence sample
  memcpy(soundBuffer,
	 (Uint8 *) &outputBuffer[sampleToSend],
	 transferSize);
  sampleToSend    += transferSize / 2;
  soundBuffer     += transferSize;
  len             -= transferSize;

  if (sampleToSend == audioBufferSize) {
    filledBuffer = false;
  }
}

void SDLMedia::fillAudioWrapper (void * userdata, Uint8 * stream, int len)
{
  SDLMedia * me = (SDLMedia *) userdata;
  me->fillAudio(stream, len);
};

void			SDLMedia::writeAudioFrames(
				const float* samples, int numFrames)
{
  int numSamples = 2 * numFrames;
  int limit;

  if (filledBuffer) {
    fprintf(stderr, "Called but buffer is already filled\n");
    return;
  }

  while (numSamples > 0) {
    if (numSamples>audioBufferSize)
      limit=audioBufferSize;
    else
      limit=numSamples;
    for (int j = 0; j < limit; j++) {
      if (samples[j] < -32767.0)
	outputBuffer[j] = -32767;
      else
	if (samples[j] > 32767.0)
	  outputBuffer[j] = 32767;
	else
	  outputBuffer[j] = short(samples[j]);
    }

    // fill out the chunk (we never write a partial chunk)
    if (limit < audioBufferSize) {
      for (int j = limit; j < audioBufferSize; ++j)
	outputBuffer[j] = 0;
    }

    if (SDL_mutexP(sndMutex) == -1) {
      fprintf(stderr, "Couldn't lock mutex\n");
      exit(-1);
    }

    filledBuffer = true;
    sampleToSend = 0;
    if (waitingData) 
      if (SDL_CondSignal(fillCond) == -1) {
	fprintf(stderr, "Couldn't Cond Signal\n");
	exit(-1);
      };

    if (SDL_mutexV(sndMutex) == -1) {
      fprintf(stderr, "Couldn't unlock mutex\n");
      exit(-1);
    }
    samples    += audioBufferSize;
    numSamples -= audioBufferSize;
  }
}

void			SDLMedia::audioSleep(bool, double endTime)
{
  if (SDL_mutexP(sndMutex) == -1) {
    fprintf(stderr, "Couldn't lock mutex\n");
    exit(-1);
  }

  if ((cmdFill <= 0) && filledBuffer) {
    waitingWake = true;
    if (endTime < 0.0) {
      if (SDL_CondWait(wakeCond, sndMutex) == -1) {
	fprintf(stderr, "Couldn't CondWait on wakeCond\n");
	exit(-1);
      };
    } else {
      if (SDL_CondWaitTimeout(wakeCond,
			      sndMutex,
			      (int) (1.0e3 * endTime)) == -1) {
	fprintf(stderr, "Couldn't CondWaitTimeout on wakeCond\n");
	exit(-1);
      };
    }
    waitingWake = false;
  }
  
  if (SDL_mutexV(sndMutex) == -1) {
    fprintf(stderr, "Couldn't unlock mutex\n");
    exit(-1);
  }
}

// Setting Audio Driver
void        SDLMedia::setDriver(std::string driverName) {
  char envAssign[256];
  std::string envVar = "SDL_AUDIODRIVER=" + driverName;
  strncpy(envAssign, envVar.c_str(), 255);
  envAssign[255]     = '\0';
  putenv(envAssign);
};

// Setting Audio Device
void        SDLMedia::setDevice(std::string) {
};

#endif //HAVE_SDL
// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

