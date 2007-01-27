/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* BeOSMedia:
 *	BeOS media stuff.
 */

#ifndef BZF_BEOS_MEDIA_H
#define BZF_BEOS_MEDIA_H

#include <OS.h>
#include <SoundPlayer.h>
#include "BzfMedia.h"


class BeOSMedia : public BzfMedia {
public:
  BeOSMedia();
  ~BeOSMedia();
  /*
    BzfDisplay*			createDisplay(const char* name, const char*videoFormat);
    BzfVisual*	createVisual(const BzfDisplay*);
    BzfWindow*	createWindow(const BzfDisplay*, BzfVisual*);
  */
  bool				openAudio();
  void				closeAudio();
  bool				startAudioThread(void (*)(void*), void*);
  void				stopAudioThread();
  bool				hasAudioThread() const;
  void				writeSoundCommand(const void*, int);
  bool				readSoundCommand(void*, int);
  int					getAudioOutputRate() const;
  int					getAudioBufferSize() const;
  int					getAudioBufferChunkSize() const;
  bool				isAudioTooEmpty() const;
  void				writeAudioFrames(const float* samples, int numFrames);
  void				audioSleep(bool checkLowWater, double maxTime);

  // sleep for given number of seconds
  double	stopwatch(bool start);
private:
  BeOSMedia(const BeOSMedia&);
  BeOSMedia& operator=(const BeOSMedia&);
  /*
    bool				checkForAudioHardware();
    bool				openAudioHardware();
    bool				openIoctl(int cmd, void* value, bool req = true);
  */
  static void			audioThreadInit(void*);
  /*
    void				writeAudioFrames8Bit(
    const float* samples, int numFrames);
    void				writeAudioFrames16Bit(
    const float* samples, int numFrames);
  */
  static void			audioplay_callback(void *cookie, void *buffer, size_t bufferSize,
						   const media_raw_audio_format &format);

private:
  bool				audioReady;
  int				audioOutputRate;
  int				audioBufferSize;
  int				audioLowWaterMark;

  port_id			audioQueuePort;
  int				audioQueueMaxCmds;

  void*				outputBuffer;
  thread_id			childThreadID;
  //  void			*audioThreadCookie;
  BSoundPlayer			*soundPlayer;
  bool				audioHasQuit;
  bool				checkLowWater;
  sem_id			lowWaterSem;

  /* ring buffer */
  sem_id			audioInputSem;
  int				audioInputIndex;
  sem_id			audioOutputSem;
  int				audioOutputIndex;

  bigtime_t			stopWatchStart;
  /*
    int				chunksPending;
    double			chunkTime;
    double			chunksPerSecond;
  */
};

#endif // BZF_BEOS_MEDIA_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
