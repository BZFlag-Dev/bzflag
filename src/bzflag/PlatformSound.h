/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
 * Sound engine that uses the native platform system and does all it's own software sound
 */

#ifndef PLATFORM_SOUND_H
#define PLATFORM_SOUND_H

#include "common.h"
#include "sound.h"
#include "BzfMedia.h"
#include "BzTime.h"

#include <string>
#include <vector>
#include <set>
#include <map>

class PlatformSound: public SoundSystem {
  public:
    PlatformSound();
    virtual ~PlatformSound();

    virtual const char* name(void) {return "PlatformSound";}

    virtual bool startup();
    virtual void shutdown();
    virtual bool active();

    virtual void setReceiver(float x, float y, float z, float t, int discontinuity);
    virtual void setReceiverVec(float vx, float vy, float vz);

    virtual int play(int soundID, const float* pos = NULL,
                     bool important = false,
                     bool localSound = true,
                     bool repeat = false,
                     bool ignoreMute = false);

    virtual void setVolume(float volume);
    virtual float getVolume();

    virtual void setMute(bool value);
    virtual bool getMute() const { return muting; }

    virtual bool update(double time);

    virtual int getID(const char* name);

    void sleep() { media->audioSleep(true, endTime); }

    bool audioInnerLoop();

  protected:

    // the fileName to ID map
    // (IDs are indices into the soundSamples vector)
    typedef std::map<std::string, int> IdMap;
    IdMap idMap;

    BzfMedia* media;

    struct SoundCommand {
      public:
        int   cmd;
        int   code;
        float data[4];
    };

    class AudioSamples {
      public:
        AudioSamples();
        AudioSamples(const AudioSamples& r);
        ~AudioSamples();
        AudioSamples& operator = (const AudioSamples& r);

        long    length;    /* total number samples in data */
        long    mlength;   /* total number samples in mono */
        long    rmlength;  /* total number samples in monoRaw */
        double  dmlength;  /* mlength as a double minus one */
        float*  data;      /* left in even, right in odd */
        float*  mono;      /* avg of channels for world sfx */
        float*  monoRaw;   /* mono with silence before & after */
        double  duration;  /* time to play sound */
    };

    std::vector<AudioSamples*> soundSamples;

    long audioBufferSize;
    int  soundLevel;

    /* speed of sound stuff */
    float      timeSizeOfWorld;   /* in seconds */
    BzTime startTime;
    double     prevTime, curTime;

    typedef struct {
      AudioSamples* samples; /* event sound effect */
      bool    busy;           /* true if in use */
      bool    muted;          /* true if muted */
      long    ptr;            /* current sample */
      double  ptrFracLeft;    /* fractional step ptr */
      double  ptrFracRight;   /* fractional step ptr */
      int     flags;          /* state info */
      float   x, y, z;        /* event location */
      double  time;           /* time of event */
      float   lastLeftAtten;
      float   lastRightAtten;
      float   dx, dy, dz;     /* last relative position */
      float   d;              /* last relative distance */
      float   dLeft;          /* last relative distance */
      float   dRight;         /* last relative distance */
      float   amplitude;      /* last sfx amplitude */
    } SoundEvent;

#define  MaxEvents 30

    /* list of events currently pending */
    SoundEvent events[MaxEvents];
    int        portUseCount;
    double     endTime;

    /* fade in/out table */
#define FadeDuration 16
    float fadeIn[FadeDuration];
    float fadeOut[FadeDuration];

    /* scratch buffer for adding contributions from sources */
    float* scratch;

    bool usingSameThread;
    bool soundStarted;

    int  loadAudioSample(const std::string& fileName);
    void freeAudioSamples(void);
    void sendSound(SoundCommand* s);

    std::set<std::string> sampleNames;

    // thread stuff
    int  findBestLocalSlot();
    int  findBestWorldSlot();
    int  addFixedContribution(SoundEvent* e, long* len);
    int  addWorldContribution(SoundEvent* e, long* len);
    void getWorldStuff(SoundEvent* e, float* la, float* ra, double* sampleStep);
    int  addLocalContribution(SoundEvent* e, long* len);
    void receiverVelocity(float* data);
    void receiverMoved(float* data);
    int  recalcEventIgnoring(SoundEvent* e);
    void recalcEventDistance(SoundEvent* e);

    int resampleAudio(const float* in, int frames, int rate, AudioSamples* out);


    /* NOTE:
     *  world sounds use the ptrFrac member, local sounds the ptr member.
     *  world sounds only use the monoaural samples so the ptrFrac member
     *  is incremented by 1 for each sample.  local sounds are in stereo
     *  and ptr is incremented by 2 for each (stereo) sample.
     */

    /* last position of the receiver */
    float lastX, lastY, lastZ, lastTheta;
    float lastXLeft, lastYLeft;
    float lastXRight, lastYRight;
    float forwardX, forwardY;
    float leftX, leftY;
    int   positionDiscontinuity;

    /* motion info for Doppler shift */
    float velX;
    float velY;
    //static float velZ;

    /* volume */
    float volumeAtten;
    int   mutingOn;
    bool  muting;
};

#endif // PLATFORM_SOUND_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
