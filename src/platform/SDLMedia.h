/* bzflag
 * Copyright (c) 1993-2025 Tim Riker
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
 *  Media I/O on SDL
 */

#ifndef BZF_SDLMEDIA_H
#define BZF_SDLMEDIA_H
#include "BzfMedia.h"
#include "bzfSDL.h"
#include <string>

class SDLMedia final : public BzfMedia
{
public:
    SDLMedia();
    ~SDLMedia() {};

    void        setMediaDirectory(const std::string&) override;
    double      stopwatch(bool) override;
    bool        openAudio() override;
    void        closeAudio() override;
    bool        startAudioThread(void (*)(void*), void*) override
    {
        return false;
    };
    void        stopAudioThread() override {};
    bool        hasAudioThread() const override
    {
        return true;
    };
    void        startAudioCallback(bool (*proc)(void)) override;
    bool        hasAudioCallback() const override
    {
        return true;
    };

    void        writeSoundCommand(const void*, int) override;
    bool        readSoundCommand(void*, int) override;
    int         getAudioOutputRate() const override;
    int         getAudioBufferSize() const override;
    int         getAudioBufferChunkSize() const override;
    bool        isAudioTooEmpty() const override
    {
        return true;
    };
    void        writeAudioFrames(const float* samples, int numFrames) override;
    void        audioSleep(bool, double) override {};
    void        setDriver(std::string driverName) override;
    void        setDevice(std::string deviceName) override;
    float*      doReadSound(const std::string& filename,
                            int& numFrames, int& rate) const override;
    void        audioDriver(std::string& driverName) override;

private:
    void        fillAudio (Uint8 *, int);
    static void  fillAudioWrapper (void *, Uint8 *, int);
    bool        tooEmpty() const;

private:
    bool        audioReady;
    int         audioOutputRate;

    bool        outputBufferEmpty;

    Uint64      stopwatchTime;

    char        cmdQueue[2048]; // space to save temporary command
    int      cmdFill;   // from 0 to cmdFill

    bool        (*userCallback)(void);
    SDL_AudioCVT    convert;
};

#endif // BZF_SDLMEDIA_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
