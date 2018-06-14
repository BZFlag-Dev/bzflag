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

/*
 * Sound effect stuff
 */

#ifndef BZF_SOUND_H
#define BZF_SOUND_H

#include <AL/alure.h>

#include <map>
#include <string>

#define MAX_SOURCES 32
#define MAX_BUFFERS 32

typedef enum
{
    SM_PRI_LOWEST = 0,
    SM_PRI_LOW,
    SM_PRI_NORMAL,
    SM_PRI_HIGH,
    SM_PRI_HIGHEST
} sm_Priority;

typedef enum
{
    SFX_FIRE =   0,       /* shell fired */
    SFX_EXPLOSION,          /* something other than me blew up */
    SFX_RICOCHET,           /* shot bounced off building */
    SFX_GRAB_FLAG,          /* grabbed a good flag */
    SFX_DROP_FLAG,          /* dropped a flag */
    SFX_CAPTURE,        /* my team captured enemy flag */
    SFX_LOSE,           /* my flag captured */
    SFX_ALERT,          /* my team flag grabbed by enemy */
    SFX_JUMP,           /* jumping sound */
    SFX_LAND,           /* landing sound */
    SFX_TELEPORT,          /* teleporting sound */
    SFX_LASER,         /* laser fired sound */
    SFX_SHOCK,         /* shockwave fired sound */
    SFX_POP,           /* tank appeared sound */
    SFX_DIE,           /* my tank exploded */
    SFX_GRAB_BAD,          /* grabbed a bad flag */
    SFX_SHOT_BOOM,         /* shot exploded */
    SFX_KILL_TEAM,         /* shot a teammate */
    SFX_PHANTOM,       /* Went into Phantom zone */
    SFX_MISSILE,       /* guided missile fired */
    SFX_LOCK,          /* missile locked on me */
    SFX_TEAMGRAB,          /* grabbed an opponents team flag */
    SFX_HUNT,          /* hunting sound */
    SFX_HUNT_SELECT,       /* hunt target selected */
    SFX_RUNOVER,             /* steamroller sound */
    SFX_THIEF,             /* thief sound */
    SFX_BURROW,        /* burrow sound */
    SFX_MESSAGE_PRIVATE,   /* private message received */
    SFX_MESSAGE_TEAM,      /* team message received */
    SFX_MESSAGE_ADMIN,     /* admin message received */
    SFX_FLAP,          /* wings flapping sound  */
    SFX_BOUNCE        /* bouncing sound */
} sm_SFX;

class SoundManager
{
private:
    // Singleton instance
    static SoundManager* instance;

    // OpenAL variables
    ALuint source[MAX_SOURCES];
    ALuint buffer[MAX_BUFFERS];
    ALCenum error;

    // Stores a mapping of filenames to buffer IDs
    std::map<std::string, int> files;

    // Next buffer to load into
    int nextBuffer;

    bool usingAudio;

public:
    SoundManager(bool initAudio);
    ~SoundManager();
    static SoundManager& getInstance(bool initAudio = false);
    static void destroyInstance();

    bool isSoundOpen()
    {
        return usingAudio;
    }

    int getVolume();
    bool setVolume(int volume);

    bool playSound(sm_SFX id, const float pos[3], const float vel[3], sm_Priority priority, bool localSound);
    bool playSound(std::string filename, const float pos[3], const float vel[3], sm_Priority priority, bool localSound);

    bool playWorldSound(sm_SFX id, const float pos[3], const float vel[3], sm_Priority priority = SM_PRI_NORMAL);
    bool playWorldSound(std::string filename, const float pos[3], const float vel[3], sm_Priority priority = SM_PRI_NORMAL);

    bool playLocalSound(sm_SFX id, sm_Priority priority = SM_PRI_NORMAL);
    bool playLocalSound(std::string filename, sm_Priority priority = SM_PRI_NORMAL);

    bool moveSoundReceiver(const float pos[3], const float azimuth, int discontinuity);
    bool speedSoundReceiver(const float vel[3]);

private:
    int findBuffer(std::string filename);
    int findSource(sm_Priority priority);

    const char* soundFiles[32] =
    {
        "fire", // 0
        "explosion",
        "ricochet",
        "flag_grab",
        "flag_drop",
        "flag_won",
        "flag_lost",
        "flag_alert",
        "jump",
        "land",
        "teleport", // 10
        "laser",
        "shock",
        "pop",
        "explosion",
        "flag_grab",
        "boom",
        "killteam",
        "phantom",
        "missile",
        "lock", // 20
        "teamgrab",
        "hunt",
        "hunt_select",
        "steamroller",
        "thief",
        "burrow",
        "message_private",
        "message_team",
        "message_admin",
        "flap", // 30
        "bounce"
    };
};

#endif // BZF_SOUND_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
