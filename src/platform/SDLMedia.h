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

/* SDLMedia:
 *	Media I/O on SDL
 */

#ifndef BZF_SDLMEDIA_H
#define	BZF_SDLMEDIA_H
#include "BzfMedia.h"
#include "bzfSDL.h"

class SDLMedia : public BzfMedia {
  public:
			SDLMedia();
			~SDLMedia();

    double		stopwatch(bool);
    void		sleep(float);
    bool		openAudio();
    void		closeAudio();
    bool		startAudioThread(void (*)(void*), void*);
    void		stopAudioThread();
    bool		hasAudioThread() const;
    void		writeSoundCommand(const void*, int);
    bool		readSoundCommand(void*, int);
    int			getAudioOutputRate() const;
    int			getAudioBufferSize() const;
    int			getAudioBufferChunkSize() const;
    bool		isAudioTooEmpty() const;
    void		writeAudioFrames(const float* samples, int numFrames);
    void		audioSleep(bool checkLowWater, double maxTime);
    void                setDriver(std::string driverName);
    void                setDevice(std::string deviceName);
    float*	        doReadSound(const char* filename,
				    int& numFrames, int& rate) const;

  private:
    void                fillAudio (Uint8 *, int);
    static void         fillAudioWrapper (void *, Uint8 *, int);
    bool		tooEmpty() const;

  private:
    bool		audioReady;
    int			audioOutputRate;
    int			audioBufferSize;

    short*		outputBuffer;
    bool                filledBuffer;  // buffer filled
    int                 sampleToSend;  // next sample to send on outputBuffer

    Uint32		stopwatchTime;

    SDL_Thread*         ThreadId;

    char                cmdQueue[2048]; // space to save temporary command
    int                 cmdFill;        // from 0 to cmdFill

    SDL_mutex *         sndMutex;
    SDL_mutex *         cmdMutex;

    SDL_cond *          wakeCond;
    SDL_cond *          fillCond;

    bool                waitingWake;
    bool                waitingData; // Waiting for data
};

#endif // BZF_SDLMEDIA_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

