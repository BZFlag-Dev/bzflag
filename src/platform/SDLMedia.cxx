
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

#include "common.h"

#ifdef HAVE_SDL
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include "SDLMedia.h"
#include "ErrorHandler.h"

/* ugh, more mixing with libCommon */
#include "StateDatabase.h"


static const int defaultAudioRate=22050;

//
// SDLMedia
//

SDLMedia::SDLMedia() : BzfMedia()
{
  cmdFill       = 0;
  audioReady    = false;
  convert.buf   = 0;
}

void SDLMedia::setMediaDirectory(const std::string& _dir)
{
  struct stat statbuf;
  const char *mdir = _dir.c_str();

#ifdef __APPLE__
  extern char *GetMacOSXDataPath(void);
  if ((stat(mdir, &statbuf) != 0) || !S_ISDIR(statbuf.st_mode)) {
    /* try the Resource folder if invoked from a .app bundled */
    mdir = GetMacOSXDataPath();
    if (mdir) {
      BZDB.set("directory", mdir);
      BZDB.setPersistent("directory", false);
    }
  }
#endif
  if ((stat(mdir, &statbuf) != 0) || !S_ISDIR(statbuf.st_mode)) {
    /* try a simple 'data' dir in current directory (source build invocation) */
    std::string defaultdir = DEFAULT_MEDIA_DIR;
    mdir = defaultdir.c_str();
    if (mdir) {
      BZDB.set("directory", mdir);
      BZDB.setPersistent("directory", false);
    }
  }
  if ((stat(mdir, &statbuf) != 0) || !S_ISDIR(statbuf.st_mode)) {
    /* give up, revert to passed in directory */
    mdir = _dir.c_str();
  }

  mediaDir = mdir;
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

bool			SDLMedia::openAudio()
{
  // don't re-initialize
  if (audioReady) return false;

  if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
    printFatalError("Could not initialize SDL-Audio: %s.\n", SDL_GetError());
    exit(-1);
  };

  static SDL_AudioSpec desired;
  static SDL_AudioSpec obtained;

  // what the frequency?
  audioOutputRate = defaultAudioRate;

  // how big a fragment to use?  we want to hold at around 1/10th of
  // a second.
  // probably SDL is using multiple buffering, make it a 3rd
  int fragmentSize = (int)(0.03f * (float)audioOutputRate);
  int n;

  n = 0;
  while ((1 << n) < fragmentSize)
    ++n;

  desired.freq       = audioOutputRate;
  desired.format     = AUDIO_S16SYS;
  desired.channels   = 2;
  desired.samples    = 1 << n; // In stereo samples
  desired.callback   = &fillAudioWrapper;
  desired.userdata   = (void *) this;	// To handle Wrap of func

  /* Open the audio device */
  if (SDL_OpenAudio(&desired, &obtained) < 0) {
    fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
    return false;
  }
  if (audioOutputRate != obtained.freq) {
    SDL_CloseAudio();
    audioOutputRate = obtained.freq;
    fragmentSize    = (int)(0.03f * (float)audioOutputRate);

    n = 0;
    while ((1 << n) < fragmentSize)
      ++n;

    desired.freq       = audioOutputRate;
    desired.samples    = 1 << n;
    /* Open the audio device */
    if (SDL_OpenAudio(&desired, &obtained) < 0) {
      fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
      return false;
    }
  }
  audioOutputRate = obtained.freq;
  if (SDL_BuildAudioCVT
      (&convert,
       desired.format, desired.channels, obtained.freq,
       obtained.format, obtained.channels, obtained.freq) == -1) {
    /* Check that the convert was built */
    printFatalError("Could not build converter for audio ", SDL_GetError());
    closeAudio();
    return false;
  }
  convert.len = (int)((double)obtained.size / convert.len_ratio);
  convert.buf = (Uint8 *)malloc(convert.len * convert.len_mult);

  // make an output buffer
  outputBufferEmpty = true;

  // ready to go
  audioReady = true;

  return true;
}

void			SDLMedia::closeAudio()
{
  // Stop Audio to avoid callback
  SDL_PauseAudio(1);

  SDL_CloseAudio();
  free(convert.buf);
  convert.buf = 0;
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
  audioReady    = false;
}

void			SDLMedia::startAudioCallback(bool (*proc)(void))
{
  userCallback = proc;
  // Stop sending silence and start calling audio callback
  SDL_PauseAudio(0);
}

void			SDLMedia::writeSoundCommand(const void* cmd, int len)
{
  if (!audioReady) return;

  SDL_LockAudio();

  // Discard command if full
  if ((cmdFill + len) < 2048) {
    memcpy(&cmdQueue[cmdFill], cmd, len);
    // We should awake audioSleep - but game become unplayable
    // using here an SDL_CondSignal(wakeCond)
    cmdFill += len;
  }

  SDL_UnlockAudio();
}

bool			SDLMedia::readSoundCommand(void* cmd, int len)
{
  bool result = false;

  if (cmdFill >= len) {
    memcpy(cmd, cmdQueue, len);
    // repack list of command waiting to be processed
    memmove(cmdQueue, &cmdQueue[len], cmdFill - len);
    cmdFill -= len;
    result = true;
  }
  return result;
}

int			SDLMedia::getAudioOutputRate() const
{
  return audioOutputRate;
}

int			SDLMedia::getAudioBufferSize() const
{
  return convert.len >> 1;
}

int			SDLMedia::getAudioBufferChunkSize() const
{
  return convert.len >> 2;
}

void SDLMedia::fillAudio (Uint8 * stream, int len)
{
  static int sampleToSend;  // next sample to send on output buffer
  if (outputBufferEmpty) {
    userCallback();
    sampleToSend = 0;
  }

  int transferSize = convert.len_cvt - sampleToSend;
  if (transferSize > len) {
    transferSize = len;
    outputBufferEmpty = false;
  } else {
    outputBufferEmpty = true;
  }

  // just copying into the soundBuffer is enough, SDL is looking for
  // something different from silence sample
  memcpy(stream, convert.buf + sampleToSend, transferSize);
  sampleToSend += transferSize;
}

void SDLMedia::fillAudioWrapper (void * userdata, Uint8 * stream, int len)
{
  SDLMedia * me = (SDLMedia *) userdata;
  me->fillAudio(stream, len);
}

void SDLMedia::writeAudioFrames(const float* samples, int)
{
  int    audioBufferSize = convert.len >> 1;
  short *buffer          = (short *)convert.buf;

  for (int j = 0; j < audioBufferSize; j++) {
    if (samples[j] < -32767.0)
      buffer[j] = -32767;
    else if (samples[j] > 32767.0)
      buffer[j] = 32767;
    else
      buffer[j] = short(samples[j]);
  }
  SDL_ConvertAudio(&convert);
}

// Setting Audio Driver
void	SDLMedia::setDriver(std::string driverName) {
  static char envAssign[256];
  std::string envVar = "SDL_AUDIODRIVER=" + driverName;
  strncpy(envAssign, envVar.c_str(), 255);
  envAssign[255]     = '\0';
  putenv(envAssign);
}

// Setting Audio Device
void	SDLMedia::setDevice(std::string deviceName) {
  static char envAssign[256];
  std::string envVar = "AUDIODEV=" + deviceName;
  strncpy(envAssign, envVar.c_str(), 255);
  envAssign[255]     = '\0';
  putenv(envAssign);
}

float*	    SDLMedia::doReadSound(const std::string &filename, int &numFrames,
				  int &rate) const
{
  SDL_AudioSpec wav_spec;
  Uint32	wav_length;
  Uint8	*wav_buffer;
  int	   ret;
  SDL_AudioCVT  wav_cvt;
  int16_t      *cvt16;
  int	   i;

  float	*data = NULL;
  rate	= defaultAudioRate;
  if (SDL_LoadWAV(filename.c_str(), &wav_spec, &wav_buffer, &wav_length)) {
    /* Build AudioCVT */
    ret = SDL_BuildAudioCVT(&wav_cvt,
			    wav_spec.format, wav_spec.channels, wav_spec.freq,
			    AUDIO_S16SYS, 2, audioOutputRate);
    /* Check that the convert was built */
    if (ret == -1) {
      printFatalError("Could not build converter for Wav file %s: %s.\n",
		      filename.c_str(), SDL_GetError());
    } else {
      /* Setup for conversion */
      wav_cvt.buf = (Uint8*)malloc(wav_length * wav_cvt.len_mult);
      wav_cvt.len = wav_length;
      memcpy(wav_cvt.buf, wav_buffer, wav_length);
      /* And now we're ready to convert */
      SDL_ConvertAudio(&wav_cvt);
      numFrames = (int)(wav_length * wav_cvt.len_ratio / 4);
      cvt16     = (int16_t *)wav_cvt.buf;
      data      = new float[numFrames * 2];
      for (i = 0; i < numFrames * 2; i++)
	data[i] = cvt16[i];
      free(wav_cvt.buf);
    }
    SDL_FreeWAV(wav_buffer);
  }
  return data;
}

void SDLMedia::audioDriver(std::string& driverName)
{
  char driver[128];
  char *result = SDL_AudioDriverName(driver, sizeof(driver));
  if (result)
    driverName = driver;
  else
    driverName = "audio not available";
}

#endif //HAVE_SDL
// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
