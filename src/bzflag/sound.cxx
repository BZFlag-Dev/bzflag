/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "sound.h"

#include "common.h"
#include "BzfMedia.h"
#include "PlatformFactory.h"

#include <iostream>

SoundManager* SoundManager::instance = 0;

SoundManager::SoundManager(bool initAudio) : error(AL_NO_ERROR), nextBuffer(0), usingAudio(false)
{
    // If we are not initializing the audio, just bail out
    if (!initAudio)
        return;

    // Initialize OpenAL via alure
    if (!alureInitDevice(nullptr, nullptr))
        return;

    // Generate our sources (that will play buffers)
    alGenSources((ALCuint)MAX_SOURCES, source);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return;

    // Generate our buffers (that store audio data)
    alGenBuffers((ALCuint)MAX_BUFFERS, buffer);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return;

    // Set our distance attenuation model
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return;

    // Yay, everything worked. We are using audio.
    usingAudio = true;
}

SoundManager::~SoundManager()
{
    // Clean up our sources and buffers
    alDeleteSources((ALCuint)MAX_SOURCES, source);
    alDeleteBuffers((ALCuint)MAX_BUFFERS, buffer);

    // Shut down our audio
    alureShutdownDevice();
}


SoundManager& SoundManager::getInstance(bool initAudio)
{
    if (instance == nullptr)
        instance = new SoundManager(initAudio);

    return *instance;
}

void SoundManager::destroyInstance()
{
    if (instance != nullptr)
        delete instance;
}

bool SoundManager::playSound(sm_SFX id, const float pos[3], const float vel[3], sm_Priority priority, bool localSound)
{
    if (localSound)
        return playLocalSound(id, priority);
    else
        return playWorldSound(id, pos, vel, priority);
}

bool SoundManager::playSound(std::string filename, const float pos[3], const float vel[3], sm_Priority priority,
                             bool localSound)
{
    if (localSound)
        return playLocalSound(filename, priority);
    else
        return playWorldSound(filename, pos, vel, priority);
}

bool SoundManager::playWorldSound(sm_SFX id, const float pos[3], const float vel[3], sm_Priority priority)
{
    if (soundFiles[id] == nullptr)
        return false;

    return playWorldSound(soundFiles[id], pos, vel, priority);
}

bool SoundManager::playWorldSound(std::string filename, const float pos[3], const float vel[3], sm_Priority priority)
{
    if (!isSoundOpen())
        return false;

    // Find a source we can use
    auto src = findSource(priority);
    if (src < 0)
        return false;

    // Find or load a buffer with the desired file
    auto buf = findBuffer(filename);
    if (buf < 0)
        return false;

    // Try to assign the buffer so we can play the sound
    alSourcei(source[src], AL_BUFFER, buffer[buf]);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return false;

    // Our sounds should be world relative, not listener relative
    alSourcei(source[src], AL_SOURCE_RELATIVE, AL_FALSE);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return false;

    // With a lower rolloff factor, the sound fades less over distance
    alSourcef(source[src], AL_ROLLOFF_FACTOR, 0.1f);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return false;

    alSourcei(source[src], AL_REFERENCE_DISTANCE, 1);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return false;

    /*
    alSourcei(source[src], AL_MAX_DISTANCE, 8000);
    error = alGetError();
    if (error != AL_NO_ERROR)
    return false;
    */

    // Set the location and velocity of the sound source
    alSourcefv(source[src], AL_POSITION, pos);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return false;

    alSourcefv(source[src], AL_VELOCITY, vel);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return false;

    if (!alurePlaySource(source[src], nullptr, nullptr) == AL_FALSE)
        return false;

    return true;
}

bool SoundManager::playLocalSound(sm_SFX id, sm_Priority priority)
{
    if (soundFiles[id] == nullptr)
        return false;

    return playLocalSound(soundFiles[id], priority);
}

bool SoundManager::playLocalSound(std::string filename, sm_Priority priority)
{
    if (!isSoundOpen())
        return false;

    // Find a source we can use
    auto src = findSource(priority);
    if (src < 0)
    {
        std::cerr << "Could not find source: " << filename << std::endl;
        return false;
    }

    // Find or load a buffer with the desired file
    auto buf = findBuffer(filename);
    if (buf < 0)
    {
        std::cerr << "Could not find buffer: " << filename << std::endl;
        return false;
    }

    // Try to assign the buffer so we can play the sound
    alSourcei(source[src], AL_BUFFER, buffer[buf]);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return false;

    // Make this source relative to the listener
    alSourcei(source[src], AL_SOURCE_RELATIVE, AL_TRUE);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return false;

    // We always want a gain of 1, so set the rolloff factor to 0
    alSourcei(source[src], AL_ROLLOFF_FACTOR, 0);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return false;

    // This is likely irrelevant, but set it regardless
    alSourcei(source[src], AL_REFERENCE_DISTANCE, 1);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return false;

    /*
    alSourcei(source[src], AL_MAX_DISTANCE, 8000);
    error = alGetError();
    if (error != AL_NO_ERROR)
    return false;
    */

    // Zero out the position and velocity
    alSource3i(source[src], AL_POSITION, 0, 0, 0);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return false;

    alSource3i(source[src], AL_VELOCITY,  0, 0, 0);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return false;

    if (!alurePlaySource(source[src], nullptr, nullptr) == AL_FALSE)
        return false;

    return true;
}

int SoundManager::findBuffer(std::string filename)
{
    auto pos = files.find(filename);
    if (pos != files.end())
        return pos->second;

    if (nextBuffer >= MAX_BUFFERS)
    {
        //std::cerr << "Exceeded maximum buffers. Ignoring new request for buffer." << std::endl;
        return -1;
    }

    std::string filepath = PlatformFactory::getMedia()->findSound(filename, "wav");

    if (filepath.length() == 0)
        return -1;

    buffer[nextBuffer] = alureCreateBufferFromFile(filepath.c_str());
    if (buffer[nextBuffer] != AL_NONE)
    {
        files[filename] = nextBuffer;
        return nextBuffer++;
    }
    else
        std::cerr << "Failed loading new audio file '" << filename << "'" << std::endl;

    return -1;
}

// TODO: Implement source priority/overwrite
int SoundManager::findSource(sm_Priority UNUSED(priority))
{
    ALint state;
    for (int i = 0; i < MAX_SOURCES; i++)
    {
        alGetSourcei(source[i], AL_SOURCE_STATE, &state);
        if (state == AL_INITIAL || state == AL_STOPPED)
            return i;
    }

    return -1;
}

int SoundManager::getVolume()
{
    if (!isSoundOpen())
        return 10;

    // Fetch the current gain setting on the listener
    ALfloat listenerGain;
    alGetListenerf(AL_GAIN, &listenerGain);


    // Since a gain of 1 is full volume, multiple by 10 to get our volume
    return (int)(listenerGain * 10.f);
}

bool SoundManager::setVolume(int volume)
{
    if (!isSoundOpen())
        return false;

    // Sanitize the volume to be a value from 0 to 10.
    if (volume > 10)
        volume = 10;
    else if (volume < 0)
        volume = 0;

    // Set the listener volume. Since a volume of 10 is full volume, and OpenAL
    // treats a gain of 1 as full volume, divide by 10.
    alListenerf(AL_GAIN, (ALfloat)(volume/10.0f));
    error = alGetError();
    return (error == AL_NO_ERROR);
}

// TODO: Should we create a version that orients in three dimenions instead of two?
bool SoundManager::moveSoundReceiver(const float pos[3], const float azimuth, int UNUSED(discontinuity))
{
    if (!isSoundOpen())
        return false;

    // Set the listener position
    alListenerfv(AL_POSITION, pos);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return false;

    // Calculate the listener orientation
    ALfloat orientation[] = { 0.f, 0.f, 0.f, 0.f, 0.f, 1.0f };
    orientation[0] = cosf(azimuth);
    orientation[1] = sinf(azimuth);

    // Set the listener orientation
    alListenerfv(AL_ORIENTATION, orientation);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return false;

    return true;
}

bool SoundManager::speedSoundReceiver(const float vel[3])
{
    if (!isSoundOpen())
        return false;

    // Set the listener velocity
    alListenerfv(AL_VELOCITY, vel);
    error = alGetError();
    if (error != AL_NO_ERROR)
        return false;

    return true;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
