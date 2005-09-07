/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* SDLMedia:
 *	Media I/O on SDL
 */

#ifndef BZF_SDLMEDIA_H
#define	BZF_SDLMEDIA_H
#include "BzfMedia.h"
#include "bzfSDL.h"
#include <string>

class SDLMedia : public BzfMedia {
  public:
			SDLMedia();
			~SDLMedia() {};

    double		stopwatch(bool);
    bool		openAudio();
    void		closeAudio();
    bool		startAudioThread(void (*)(void*), void*)
			  {return false;};
    void		stopAudioThread() {};
    bool		hasAudioThread() const {return true;};
    void		startAudioCallback(bool (*proc)(void));
    bool		hasAudioCallback() const {return true;};

    void		writeSoundCommand(const void*, int);
    bool		readSoundCommand(void*, int);
    int			getAudioOutputRate() const;
    int			getAudioBufferSize() const;
    int			getAudioBufferChunkSize() const;
    bool		isAudioTooEmpty() const {return true;};
    void		writeAudioFrames(const float* samples, int numFrames);
    void		audioSleep(bool, double) {};
    void		setDriver(std::string driverName);
    void		setDevice(std::string deviceName);
    float*		doReadSound(const std::string& filename,
				    int& numFrames, int& rate) const;
    void		audioDriver(std::string& driverName);

  private:
    void		fillAudio (Uint8 *, int);
    static void	 fillAudioWrapper (void *, Uint8 *, int);
    bool		tooEmpty() const;

  private:
    bool		audioReady;
    int			audioOutputRate;
    int			audioBufferSize;

    short*		outputBuffer;
    int		 sampleToSend;  // next sample to send on outputBuffer

    Uint32		stopwatchTime;

    char		cmdQueue[2048]; // space to save temporary command
    int		 cmdFill;	// from 0 to cmdFill

    bool		(*userCallback)(void);
};

#endif // BZF_SDLMEDIA_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

