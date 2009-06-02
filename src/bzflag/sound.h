/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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

#include "common.h"
#include "Singleton.h"
#include <string>
#include <vector>
#include <string>

extern int  SFX_FIRE;		  /* shell fired */
extern int  SFX_EXPLOSION;	  /* something other than me blew up */
extern int  SFX_RICOCHET;	  /* shot bounced off building */
extern int  SFX_GRAB_FLAG;	  /* grabbed a good flag */
extern int  SFX_DROP_FLAG;	  /* dropped a flag */
extern int  SFX_CAPTURE;	  /* my team captured enemy flag */
extern int  SFX_LOSE;		  /* my flag captured */
extern int  SFX_ALERT;		  /* my team flag grabbed by enemy */
extern int  SFX_JUMP;		  /* jumping sound */
extern int  SFX_LAND;		  /* landing sound */
extern int  SFX_TELEPORT;	  /* teleporting sound */
extern int  SFX_LASER;		  /* laser fired sound */
extern int  SFX_SHOCK;		  /* shockwave fired sound */
extern int  SFX_POP;		  /* tank appeared sound */
extern int  SFX_DIE;		  /* my tank exploded */
extern int  SFX_GRAB_BAD;	  /* grabbed a bad flag */
extern int  SFX_SHOT_BOOM;	  /* shot exploded */
extern int  SFX_KILL_TEAM;	  /* shot a teammate */
extern int  SFX_PHANTOM;	  /* Went into Phantom zone */
extern int  SFX_MISSILE;	  /* guided missile fired */
extern int  SFX_LOCK;		  /* missile locked on me */
extern int  SFX_TEAMGRAB;	  /* grabbed an opponents team flag */
extern int  SFX_HUNT;		  /* hunting sound */
extern int  SFX_HUNT_SELECT;	  /* hunt target selected */
extern int  SFX_RUNOVER;	  /* steamroller sound */
extern int  SFX_THIEF;		  /* thief sound */
extern int  SFX_BURROW;		  /* burrow sound */
extern int  SFX_MESSAGE_PRIVATE;  /* private message received */
extern int  SFX_MESSAGE_TEAM;	  /* team message received */
extern int  SFX_MESSAGE_ADMIN;	  /* admin message received */
extern int  SFX_FLAP;		  /* wings flapping sound  */
extern int  SFX_BOUNCE;		  /* bouncing sound */
extern int  SFX_HIT;		  /* struck by a shot but not dead yet sound */

class SoundSystem
{
public:
  virtual ~SoundSystem() {};

  virtual const char* name(void) = 0;

  virtual bool startup(void) = 0;
  virtual void shutdown(void) = 0;
  virtual bool active(void) = 0;

  virtual void setReceiver(float x, float y, float z, float t, int discontinuity) = 0;
  virtual void setReceiverVec(float vx, float vy, float vz) = 0;

  virtual int play(int soundID,
                   const float *pos = NULL,
                   bool important = false,
                   bool localSound = true,
                   bool repeat = false) = 0;

  virtual int play(const char* _name,
                   const float *pos = NULL,
                   bool important = false,
                   bool localSound = true,
                   bool repeat = false) {
    return play(getID(_name), pos, important, localSound, repeat);
  }

  virtual int play(const std::string &_name,
                   const float *pos = NULL,
                   bool important = false,
                   bool localSound = true,
                   bool repeat = false) {
    return play(getID(_name), pos, important, localSound, repeat);
  }

  virtual void setVolume(float volume) = 0;
  virtual float getVolume(void) = 0;

  virtual bool update(double time) = 0;

  virtual int getID(const char* name) = 0;
  virtual int getID(const std::string &_name) {
    return getID(_name.c_str());
  }

};

class SoundManager : public Singleton<SoundManager>
{
public:
  SoundSystem& getSystem(void);
  void registerSystem(SoundSystem *);

  std::vector<SoundSystem*> listSystems(void);

  void activateSoundSystem(SoundSystem* sys);
  void activateSoundSystem(const std::string &name);
  void activateSoundSystem(const char *name) {
    if (name) activateSoundSystem(std::string(name));
  }

  std::vector<std::string> getStdSounds(void);
protected:
  friend class Singleton<SoundManager>;

  SoundManager();
  virtual ~SoundManager();

  std::vector<SoundSystem*> soundSystems;
  SoundSystem* currentSystem;

private:
  void addStandardSound(const char* name);
  std::vector<std::string> standardSounds;
};

#define SOUNDSYSTEM SoundManager::instance().getSystem()

#endif // BZF_SOUND_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
