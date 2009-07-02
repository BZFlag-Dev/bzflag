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

// interface header
#include "PlatformSound.h"

// common headers
#include "PlatformFactory.h"
#include "BZDBCache.h"
#include "TextUtils.h"
#include <cstring>

static const float SpeedOfSound = 343.0f;		// meters/sec
static const float InterAuralDistance = 0.1f;		// meters

/* sound queue commands */
#define	SQC_CLEAR	0		/* no code; no data */
#define	SQC_SET_POS	1		/* no code; x,y,z,t */
#define	SQC_SET_VEL	2		/* no code; x,y,z */
#define	SQC_SET_VOLUME	3		/* code = new volume; no data */
#define	SQC_LOCAL_SFX	4		/* code=sfx; no data */
#define	SQC_WORLD_SFX	5		/* code=sfx; x,y,z of sfx source */
#define	SQC_FIXED_SFX	6		/* code=sfx; x,y,z of sfx source */
#define	SQC_JUMP_POS	7		/* no code; x,y,z,t */
#define	SQC_QUIT	8		/* no code; no data */
#define	SQC_IWORLD_SFX	9		/* code=sfx; x,y,z of sfx source */

PlatformSound::AudioSamples::AudioSamples()
{
  length = 0;
  mlength = 0;
  rmlength = 0;
  dmlength = 0;
  data = NULL;
  mono = NULL;
  monoRaw = NULL;
  duration = 0;
}

PlatformSound::AudioSamples::~AudioSamples()
{
  if (data)
    delete[] data;

  if (monoRaw)
    delete[] monoRaw;
}

PlatformSound::AudioSamples& PlatformSound::AudioSamples::operator = ( const AudioSamples& r)
{
  if (data)
    delete[] data;

  if (monoRaw)
    delete[] monoRaw;

  length = r.length;
  mlength = r.mlength;
  dmlength = r.dmlength;
  rmlength = r.rmlength;
  data = new float[length];
  memcpy(data,r.data,sizeof(float)*length);
  monoRaw = new float[rmlength];
  memcpy(monoRaw,r.monoRaw,sizeof(float)*rmlength);
  duration = r.duration;

  mono = monoRaw + (r.mono-r.monoRaw);
  return *this;
}

PlatformSound::AudioSamples::AudioSamples ( const AudioSamples& r)
{
  length = r.length;
  mlength = r.mlength;
  dmlength = r.dmlength;
  rmlength = r.rmlength;
  data = new float[length];
  memcpy(data,r.data,sizeof(float)*length);
  mono = new float[mlength];
  memcpy(mono,r.mono,sizeof(float)*mlength);
  monoRaw = new float[rmlength];
  memcpy(monoRaw,r.monoRaw,sizeof(float)*rmlength);
  duration = r.duration;
}

/*
 * local functions
 */

static void		audioLoop(void*);

PlatformSound		*platformSound = NULL;

static bool audioClassInnerLoop()
{
  if (platformSound)
    return platformSound->audioInnerLoop();
  return false;
}

PlatformSound::PlatformSound()
{
  soundStarted = false;
  usingSameThread = false;

  volumeAtten = 1.0f;
  mutingOn = 0;
  media = NULL;
}

PlatformSound::~PlatformSound()
{
  shutdown();
}

bool PlatformSound::startup ( void )
{
  platformSound = this;

  unsigned int i;

  if (active())
    return true;			// already opened

  media = PlatformFactory::getMedia();
  if (!media->openAudio())
    return false;

  soundStarted = true;

  // open audio data files
  if (!setStandardSoundIDs())
  {
    media->closeAudio();
#ifndef DEBUG
    std::cout << "WARNING: Unable to open audio data files" << std::endl;
#endif
    return false;					// couldn't get samples
  }

  audioBufferSize = media->getAudioBufferChunkSize() << 1;

  /* initialize */
  float worldSize = BZDBCache::worldSize;
  timeSizeOfWorld = 1.414f * worldSize / SpeedOfSound;
  for (i = 0; i < (int)MaxEvents; i++)
  {
    events[i].samples = NULL;
    events[i].busy = false;
  }
  portUseCount = 0;
  for (i = 0; i < (int)FadeDuration; i += 2)
  {
    fadeIn[i] = fadeIn[i+1] =
      sinf((float)(M_PI / 2.0 * (double)i / (double)(FadeDuration-2)));
    fadeOut[i] = fadeOut[i+1] = 1.0f - fadeIn[i];
  }
  scratch = new float[audioBufferSize];

  startTime = TimeKeeper::getCurrent();
  curTime = 0.0;
  endTime = -1.0;

  usingSameThread = !media->hasAudioThread();

  if (media->hasAudioCallback())
    media->startAudioCallback(audioClassInnerLoop);
  else
  {
    // start audio thread
    if (!usingSameThread && !media->startAudioThread(audioLoop, NULL)) {
      media->closeAudio();
      freeAudioSamples();
#ifndef DEBUG
      std::cout << "WARNING: Unable to start the audio thread" << std::endl;
#endif
      return false;
    }
  }

  setVolume(1.0f);

  return active();
}

void PlatformSound::shutdown ( void )
{
  platformSound = NULL;
  if (active())
  {
    // send stop command to audio thread
    SoundCommand s;
    s.cmd = SQC_QUIT;
    s.code = 0;
    s.data[0] = 0.0f;
    s.data[1] = 0.0f;
    s.data[2] = 0.0f;
    s.data[3] = 0.0f;
    sendSound(&s);

    // stop audio thread
    PlatformFactory::getMedia()->stopAudioThread();

    // reset audio hardware
    PlatformFactory::getMedia()->closeAudio();

    delete[] scratch;

    // free memory used for sfx samples
    freeAudioSamples();

    soundStarted = false;
  }
}

bool PlatformSound::active ( void )
{
  return soundStarted;
}


int PlatformSound::loadAudioSample( const std::string& fileName )
{
  int soundCode = -1;
  int numFrames, rate;

  float* samples = PlatformFactory::getMedia()->readSound(fileName.c_str(),
                                                          numFrames, rate,
                                                          true);
  AudioSamples* newSample = new AudioSamples;
  if (samples && resampleAudio(samples, numFrames, rate, newSample)) {
    soundCode = (int)soundSamples.size();
    soundSamples.push_back(newSample);
    idMap[fileName] = soundCode;
  }
  else {
    delete newSample;
  }

  delete[] samples;

  return soundCode;
}


void PlatformSound::freeAudioSamples( void )
{
  for (unsigned int i = 0; i < soundSamples.size(); i++) {
    delete(soundSamples[i]);
  }
  soundSamples.clear();
}

void PlatformSound::sendSound(SoundCommand* s)
{
  if (!active())
    return;
  PlatformFactory::getMedia()->writeSoundCommand(s, sizeof(SoundCommand));
}

void PlatformSound::setReceiver(float x, float y, float z, float t, int discontinuity)
{
  if (soundLevel <= 0) {
    return;
  }
  SoundCommand s;
  s.cmd = discontinuity ? SQC_JUMP_POS : SQC_SET_POS;
  s.code = 0;
  s.data[0] = x;
  s.data[1] = y;
  s.data[2] = z;
  s.data[3] = t;
  sendSound(&s);
}

void PlatformSound::setReceiverVec(float vx, float vy, float vz)
{
  SoundCommand s;
  s.cmd = SQC_SET_VEL;
  s.code = 0;
  s.data[0] = vx;
  s.data[1] = vy;
  s.data[2] = vz;
  s.data[3] = 0.0f;
  sendSound(&s);
}

int PlatformSound::play(int soundID, const float *pos,
                        bool important, bool localSound, bool repeat)
{
  if (soundLevel <= 0)
    return 0;

  SoundCommand s;
  if (soundID < 0 ||( int)soundSamples.size() <= soundID)
    return 0;
  if (soundSamples[soundID]->length == 0)
    return 0;

  if (repeat)
    s.cmd = SQC_FIXED_SFX;
  else if (localSound || !pos)
    s.cmd = SQC_LOCAL_SFX;
  else
    s.cmd = important ? SQC_IWORLD_SFX : SQC_WORLD_SFX;

  s.code = soundID;
  if(pos)
  {
    s.data[0] = pos[0];
    s.data[1] = pos[1];
    s.data[2] = pos[2];
  }
  s.data[3] = 0.0f;
  sendSound(&s);

  return 0;
}

void PlatformSound::setVolume ( float volume )
{
  soundLevel = (int)(volume*10.0f);
  if (soundLevel < 0) soundLevel = 0;
  else if (soundLevel > 10) soundLevel = 10;

  SoundCommand s;
  s.cmd = SQC_SET_VOLUME;
  s.code = soundLevel;
  s.data[0] = 0.0f;
  s.data[1] = 0.0f;
  s.data[2] = 0.0f;
  s.data[3] = 0.0f;
  sendSound(&s);
}

float PlatformSound::getVolume ( void )
{
  return soundLevel / 10.0f;
}

bool PlatformSound::update ( double /*time*/ )
{
  if (!active() || !usingSameThread)
    return false;

  // sleep until audio buffers hit low water mark or new command available
  media->audioSleep(true, 0.0);
  audioInnerLoop();
  return true;
}


int PlatformSound::getID ( const char* _name )
{
  if (_name == NULL) {
    return -1;
  }

  const std::string sound = TextUtils::tolower(_name);

  IdMap::const_iterator it = idMap.find(sound);
  if (it != idMap.end()) {
    return it->second;
  }

  return loadAudioSample(sound);
}


bool PlatformSound::setStandardSoundIDs ( void )
{
  SFX_SHOT_BOOM       = getID("boom");
  SFX_BOUNCE          = getID("bounce");
  SFX_BURROW          = getID("burrow");
  SFX_DIE             = getID("explosion");
  SFX_EXPLOSION       = getID("explosion");
  SFX_FIRE            = getID("fire");
  SFX_ALERT           = getID("flag_alert");
  SFX_DROP_FLAG       = getID("flag_drop");
  SFX_GRAB_BAD        = getID("flag_grab");
  SFX_GRAB_FLAG       = getID("flag_grab");
  SFX_LOSE            = getID("flag_lost");
  SFX_CAPTURE         = getID("flag_won");
  SFX_FLAP            = getID("flap");
  SFX_HIT             = getID("hit");
  SFX_HUNT            = getID("hunt");
  SFX_HUNT_SELECT     = getID("hunt_select");
  SFX_JUMP            = getID("jump");
  SFX_KILL_TEAM       = getID("killteam");
  SFX_LAND            = getID("land");
  SFX_LASER           = getID("laser");
  SFX_LOCK            = getID("lock");
  SFX_MESSAGE_ADMIN   = getID("message_admin");
  SFX_MESSAGE_PRIVATE = getID("message_team");
  SFX_MESSAGE_TEAM    = getID("message_team");
  SFX_MISSILE         = getID("missile");
  SFX_PHANTOM         = getID("phantom");
  SFX_POP             = getID("pop");
  SFX_RICOCHET        = getID("ricochet");
  SFX_SHOCK           = getID("shock");
  SFX_RUNOVER         = getID("steamroller");
  SFX_TEAMGRAB        = getID("teamgrab");
  SFX_TELEPORT        = getID("teleport");
  SFX_THIEF           = getID("thief");

  return !idMap.empty();
}

int PlatformSound::resampleAudio(const float* in, int frames, int rate, PlatformSound::AudioSamples* out)
{
  // attenuation on all sounds
  static const float GlobalAtten = 0.5f;

  if (rate != PlatformFactory::getMedia()->getAudioOutputRate()) {
    // FIXME -- should resample  -- let it through at wrong sample rate
    // return 0;
  }

  // compute safety margin for left/right ear time discrepancy.  since
  // each ear can hear a sound at slightly different times one ear might
  // be hearing a sound before the other hears it or one ear may no longer
  // be hearing a sound that the other is still hearing.  if we didn't
  // account for this we'd end up sampling outside the mono buffer (and
  // only the mono buffer because the data buffer is only used for local
  // sounds, which don't have this effect).  to avoid doing a lot of tests
  // in inner loops, we'll just put some silent samples before and after
  // the sound effect.  here we compute how many samples can be needed.
  const int safetyMargin = (int)(1.5f + InterAuralDistance / SpeedOfSound *
    (float)PlatformFactory::getMedia()->getAudioOutputRate());

  // fill in samples structure
  out->length = 2 * frames;
  out->mlength = out->length >> 1;
  out->dmlength = double(out->mlength - 1);
  out->duration = (float)out->mlength /
    (float)PlatformFactory::getMedia()->getAudioOutputRate();
  out->data = new float[out->length];
  out->monoRaw = new float[out->mlength + 2 * safetyMargin];
  out->mono = out->monoRaw + safetyMargin;
  if (!out->data || !out->monoRaw) {
    delete[] out->data;
    delete[] out->monoRaw;
    return 0;
  }

  // filter samples
  for (long dst = 0; dst < out->length; dst += 2) {
    out->data[dst] = GlobalAtten * in[dst];
    out->data[dst+1] = GlobalAtten * in[dst+1];
    out->mono[dst>>1] = 0.5f * (out->data[dst] + out->data[dst+1]);
  }

  // silence in safety margins
  for (int i = 0; i < safetyMargin; ++i)
    out->monoRaw[i] = out->monoRaw[out->mlength + safetyMargin + i] = 0.0f;

  return 1;
}

/*
 * Below this point is stuff for real-time audio thread
 */

#define	SEF_WORLD	1
#define	SEF_FIXED	2
#define	SEF_IGNORING	4
#define	SEF_IMPORTANT	8

void PlatformSound::recalcEventDistance(SoundEvent* e)
{
  e->dx = e->x - lastX;
  e->dy = e->y - lastY;
  e->dz = e->z - lastZ;
  const float d2 = e->dx * e->dx + e->dy * e->dy;
  const float d3 = e->dz * e->dz;
  if (d2 <= 1.0f) {
    e->d = 0.0f;
    e->dLeft = 0.0f;
    e->dRight = 0.0f;
  }
  else {
    const float d = 1.0f / sqrtf(d2);
    e->dx *= d;
    e->dy *= d;
    e->d = sqrtf(d2 + d3);
    // Since we are on a forked process, we cannot use BZDB_TANKRADIUS
    // We put in the old tankRadius value (4.32)
    float minEventDist = 20.0f * 4.32f;
    e->amplitude = (e->d < minEventDist) ? 1.0f : minEventDist / e->d;

    // compute distance to each ear
    float dx, dy;
    dx = e->x - lastXLeft;
    dy = e->y - lastYLeft;
    e->dLeft = sqrtf(dx * dx + dy * dy + d3);
    dx = e->x - lastXRight;
    dy = e->y - lastYRight;
    e->dRight = sqrtf(dx * dx + dy * dy + d3);
  }
}

int PlatformSound::recalcEventIgnoring(SoundEvent* e)
{
  if ((e->flags & SEF_FIXED) || !(e->flags & SEF_WORLD)) return 0;

  float travelTime = (float)(curTime - e->time);
  if (travelTime > e->samples->duration + timeSizeOfWorld) {
    // sound front has passed all points in world
    e->busy = false;
    return (e->flags & SEF_IGNORING) ? 0 : -1;
  }

  int useChange = 0;
  float eventDistance = e->d / SpeedOfSound;
  if (travelTime < eventDistance) {
    if (e->flags & SEF_IGNORING) {
      /* do nothing -- still ignoring */
    }
    else {
      /* ignoring again */
      e->flags |= SEF_IGNORING;
      useChange = -1;
    }
    /* don't sleep past the time the sound front will pass by */
    endTime = eventDistance;
  }
  else {
    float timeFromFront;
    if (e->flags & SEF_IGNORING) {
      /* compute time from sound front */
      timeFromFront = travelTime - e->dLeft / SpeedOfSound;
      if (!positionDiscontinuity && timeFromFront < 0.0f) timeFromFront = 0.0f;

      /* recompute sample pointers */
      e->ptrFracLeft = timeFromFront *
	(float)PlatformFactory::getMedia()->getAudioOutputRate();
      if (e->ptrFracLeft >= 0.0 && e->ptrFracLeft < e->samples->dmlength) {
	/* not ignoring anymore */
	e->flags &= ~SEF_IGNORING;
	useChange = 1;
      }

      /* now do it again for right ear */
      timeFromFront = travelTime - e->dRight / SpeedOfSound;
      if (!positionDiscontinuity && timeFromFront < 0.0f) timeFromFront = 0.0f;
      e->ptrFracRight = timeFromFront *
	(float)PlatformFactory::getMedia()->getAudioOutputRate();
      if (e->ptrFracRight >= 0.0 && e->ptrFracRight < e->samples->dmlength) {
	e->flags &= ~SEF_IGNORING;
	useChange = 1;
      }
    }
    else {
      /* do nothing -- still not ignoring */
    }
  }
  return useChange;
}

void PlatformSound::receiverMoved(float* data)
{
  // save data
  lastX = data[0];
  lastY = data[1];
  lastZ = data[2];
  lastTheta = data[3];

  // compute forward and left vectors
  forwardX = cosf(lastTheta);
  forwardY = sinf(lastTheta);
  leftX = -forwardY;
  leftY = forwardX;

  // compute position of each ear
  lastXLeft = lastX + 0.5f * InterAuralDistance * leftX;
  lastYLeft = lastY + 0.5f * InterAuralDistance * leftY;
  lastXRight = lastX - 0.5f * InterAuralDistance * leftX;
  lastYRight = lastY - 0.5f * InterAuralDistance * leftY;

  for (int i = 0; i < MaxEvents; i++)
    if (events[i].busy && (events[i].flags & SEF_WORLD))
      recalcEventDistance(events + i);
}

void PlatformSound::receiverVelocity(float* data)
{
  static const float s = 1.0f / SpeedOfSound;

  velX = s * data[0];
  velY = s * data[1];
  //  velZ = s * data[2];
}

int PlatformSound::addLocalContribution(SoundEvent* e, long* len)
{
  long		n, numSamples;
  float*	src;

  numSamples = e->samples->length - e->ptr;
  if (numSamples > audioBufferSize) numSamples = audioBufferSize;

  if (!mutingOn && numSamples != 0) {
    /* initialize new areas of scratch space and adjust output sample count */
    if (numSamples > *len) {
      for (n = *len; n < numSamples; n += 2)
	scratch[n] = scratch[n+1] = 0.0f;
      *len = numSamples;
    }

    // add contribution -- conditionals outside loop for run-time efficiency
    src = e->samples->data + e->ptr;
    if (numSamples <= FadeDuration) {
      for (n = 0; n < numSamples; n += 2) {
	int fs = int(FadeDuration * float(n) / float(numSamples)) & ~1;
	scratch[n] += src[n] * (fadeIn[fs] * volumeAtten +
	  fadeOut[fs] * e->lastLeftAtten);
	scratch[n+1] += src[n+1] * (fadeIn[fs] * volumeAtten +
	  fadeOut[fs] * e->lastRightAtten);
      }
    }
    else {
      for (n = 0; n < FadeDuration; n += 2) {
	scratch[n] += src[n] * (fadeIn[n] * volumeAtten +
	  fadeOut[n] * e->lastLeftAtten);
	scratch[n+1] += src[n+1] * (fadeIn[n] * volumeAtten +
	  fadeOut[n] * e->lastRightAtten);
      }
      if (volumeAtten == 1.0f) {
	for (; n < numSamples; n += 2) {
	  scratch[n] += src[n];
	  scratch[n+1] += src[n+1];
	}
      }
      else {
	for (; n < numSamples; n += 2) {
	  scratch[n] += src[n] * volumeAtten;
	  scratch[n+1] += src[n+1] * volumeAtten;
	}
      }
    }

    e->lastLeftAtten = e->lastRightAtten = volumeAtten;
  }

  /* free event if ran out of samples */
  if ((e->ptr += numSamples) == e->samples->length) {
    e->busy = false;
    return -1;
  }

  return 0;
}

void PlatformSound::getWorldStuff(SoundEvent *e, float* la, float* ra,
				      double* sampleStep)
{
  float leftAtten, rightAtten;

  // compute left and right attenuation factors
  // FIXME -- should be a more general HRTF
  if (e->d == 0.0f) {
    leftAtten = 1.0f;
    rightAtten = 1.0f;
  }
  else {
    const float ff = (2.0f + forwardX * e->dx + forwardY * e->dy) / 3.0f;
    const float fl = (2.0f + leftX * e->dx + leftY * e->dy) / 3.0f;
    leftAtten = ff * fl * e->amplitude;
    rightAtten = ff * (4.0f/3.0f - fl) * e->amplitude;
  }
  if (e->ptrFracLeft == 0.0f || e->ptrFracRight == 0.0f) {
    e->lastLeftAtten = leftAtten;
    e->lastRightAtten = rightAtten;
  }
  *la = mutingOn ? 0.0f : leftAtten * volumeAtten;
  *ra = mutingOn ? 0.0f : rightAtten * volumeAtten;

  /* compute doppler effect */
  // FIXME -- should be per ear
  *sampleStep = double(1.0 + velX * e->dx + velY * e->dy);
}

int PlatformSound::addWorldContribution(SoundEvent* e, long* len)
{
  int		fini = 0;
  long		n, nmL, nmR;
  float*	src = e->samples->mono;
  float		leftAtten, rightAtten, fracL, fracR, fsampleL, fsampleR;
  double	sampleStep;

  if (e->flags & SEF_IGNORING) return 0;

  getWorldStuff(e, &leftAtten, &rightAtten, &sampleStep);
  if (sampleStep <= 0.0) fini = 1;

  /* initialize new areas of scratch space and adjust output sample count */
  if (audioBufferSize > *len) {
    for (n = *len; n < audioBufferSize; n += 2)
      scratch[n] = scratch[n+1] = 0.0f;
    *len = audioBufferSize;
  }

  // add contribution with crossfade
  for (n = 0; !fini && n < FadeDuration; n += 2) {
    // get sample position (to subsample resolution)
    nmL = (long)e->ptrFracLeft;
    nmR = (long)e->ptrFracRight;
    fracL = (float)(e->ptrFracLeft - floor(e->ptrFracLeft));
    fracR = (float)(e->ptrFracRight - floor(e->ptrFracRight));

    // get sample (lerp closest two samples)
    fsampleL = (1.0f - fracL) * src[nmL] + fracL * src[nmL+1];
    fsampleR = (1.0f - fracR) * src[nmR] + fracR * src[nmR+1];

    // filter and accumulate
    scratch[n] += fsampleL * (fadeIn[n] * leftAtten +
      fadeOut[n] * e->lastLeftAtten);
    scratch[n+1] += fsampleR * (fadeIn[n] * rightAtten +
      fadeOut[n] * e->lastRightAtten);

    // next sample
    if ((e->ptrFracLeft += sampleStep) >= e->samples->dmlength)
      fini = true;
    if ((e->ptrFracRight += sampleStep) >= e->samples->dmlength)
      fini = true;
  }

  // add contribution
  for (; !fini && n < audioBufferSize; n += 2) {
    // get sample position (to subsample resolution)
    nmL = (long)e->ptrFracLeft;
    nmR = (long)e->ptrFracRight;
    fracL = (float)(e->ptrFracLeft - floor(e->ptrFracLeft));
    fracR = (float)(e->ptrFracRight - floor(e->ptrFracRight));

    // get sample (lerp closest two samples)
    fsampleL = (1.0f - fracL) * src[nmL] + fracL * src[nmL+1];
    fsampleR = (1.0f - fracR) * src[nmR] + fracR * src[nmR+1];

    // filter and accumulate
    scratch[n] += fsampleL * leftAtten;
    scratch[n+1] += fsampleR * rightAtten;

    // next sample
    if ((e->ptrFracLeft += sampleStep) >= e->samples->dmlength)
      fini = true;
    if ((e->ptrFracRight += sampleStep) >= e->samples->dmlength)
      fini = true;
  }
  e->lastLeftAtten = leftAtten;
  e->lastRightAtten = rightAtten;

  /* NOTE: running out of samples just means the world sound front
  *	has passed our location.  if we teleport it may pass us again.
  *	so we can't free the event until the front passes out of the
  *	world.  compute time remaining until that happens and set
  *	endTime if smaller than current endTime. */
  if (fini) {
    double et = e->samples->duration + timeSizeOfWorld - (prevTime - e->time);
    if (endTime == -1.0 || et < endTime) endTime = et;
    e->flags |= SEF_IGNORING;
    return -1;
  }

  return 0;
}

int PlatformSound::addFixedContribution(SoundEvent* e, long* len)
{
  long		n, nmL, nmR;
  float*	src = e->samples->mono;
  float		leftAtten, rightAtten, fracL, fracR, fsampleL, fsampleR;
  double	sampleStep;

  getWorldStuff(e, &leftAtten, &rightAtten, &sampleStep);

  /* initialize new areas of scratch space and adjust output sample count */
  if (audioBufferSize > *len) {
    for (n = *len; n < audioBufferSize; n += 2)
      scratch[n] = scratch[n+1] = 0.0f;
    *len = audioBufferSize;
  }

  // add contribution with crossfade
  for (n = 0; n < FadeDuration; n += 2) {
    // get sample position (to subsample resolution)
    nmL = (long)e->ptrFracLeft;
    nmR = (long)e->ptrFracRight;
    fracL = (float)(e->ptrFracLeft - floor(e->ptrFracLeft));
    fracR = (float)(e->ptrFracRight - floor(e->ptrFracRight));

    // get sample (lerp closest two samples)
    fsampleL = (1.0f - fracL) * src[nmL] + fracL * src[nmL+1];
    fsampleR = (1.0f - fracR) * src[nmR] + fracR * src[nmR+1];

    // filter and accumulate
    scratch[n] += fsampleL * (fadeIn[n] * leftAtten +
      fadeOut[n] * e->lastLeftAtten);
    scratch[n+1] += fsampleR * (fadeIn[n] * rightAtten +
      fadeOut[n] * e->lastRightAtten);

    // next sample
    if ((e->ptrFracLeft += sampleStep) >= e->samples->dmlength)
      e->ptrFracLeft -= e->samples->dmlength;
    if ((e->ptrFracRight += sampleStep) >= e->samples->dmlength)
      e->ptrFracRight -= e->samples->dmlength;
  }

  // add contribution
  for (; n < audioBufferSize; n += 2) {
    // get sample position (to subsample resolution)
    nmL = (long)e->ptrFracLeft;
    nmR = (long)e->ptrFracRight;
    fracL = (float)(e->ptrFracLeft - floor(e->ptrFracLeft));
    fracR = (float)(e->ptrFracRight - floor(e->ptrFracRight));

    // get sample (lerp closest two samples)
    fsampleL = (1.0f - fracL) * src[nmL] + fracL * src[nmL+1];
    fsampleR = (1.0f - fracR) * src[nmR] + fracR * src[nmR+1];

    // filter and accumulate
    scratch[n] += fsampleL * leftAtten;
    scratch[n+1] += fsampleR * rightAtten;

    // next sample
    if ((e->ptrFracLeft += sampleStep) >= e->samples->dmlength)
      e->ptrFracLeft -= e->samples->dmlength;
    if ((e->ptrFracRight += sampleStep) >= e->samples->dmlength)
      e->ptrFracRight -= e->samples->dmlength;
  }
  e->lastLeftAtten = leftAtten;
  e->lastRightAtten = rightAtten;

  return 0;
}

int PlatformSound::findBestWorldSlot()
{
  int i;

  // the best slot is an empty one
  for (i = 0; i < MaxEvents; i++)
    if (!events[i].busy)
      return i;

  // no available slots.  find an existing sound that won't be missed
  // (much).  this will cause a pop or crackle if the replaced sound is
  // currently playing.  first see if there are any world events.
  for (i = 0; i < MaxEvents; i++)
    if ((events[i].flags & (SEF_WORLD | SEF_FIXED)) == SEF_WORLD)
      break;

  // give up if no (non-fixed) world events
  if (i == MaxEvents) return MaxEvents;

  // found a world event.  see if there's an event that's
  // completely passed us.
  const int first = i;
  for (i = first; i < MaxEvents; i++) {
    if ((events[i].flags & (SEF_WORLD | SEF_FIXED)) != SEF_WORLD) continue;
    if (!(events[i].flags & SEF_IGNORING)) continue;
    const float travelTime = (float)(curTime - events[i].time);
    const float eventDistance = events[i].d / SpeedOfSound;
    if (travelTime > eventDistance) return i;
  }

  // if no sound front has completely passed our position
  // then pick the most distant one that hasn't reached us
  // yet that isn't important.
  int farthestEvent = -1;
  float farthestDistance = 0.0f;
  for (i = first; i < MaxEvents; i++) {
    if (events[i].flags & SEF_IMPORTANT) continue;
    if ((events[i].flags & (SEF_WORLD | SEF_FIXED)) != SEF_WORLD) continue;
    if (!(events[i].flags & SEF_IGNORING)) continue;
    const float eventDistance = events[i].d / SpeedOfSound;
    if (eventDistance > farthestDistance) {
      farthestEvent = i;
      farthestDistance = eventDistance;
    }
  }
  if (farthestEvent != -1) return farthestEvent;

  // same thing but look at important sounds
  for (i = first; i < MaxEvents; i++) {
    if (!(events[i].flags & SEF_IMPORTANT)) continue;
    if ((events[i].flags & (SEF_WORLD | SEF_FIXED)) != SEF_WORLD) continue;
    if (!(events[i].flags & SEF_IGNORING)) continue;
    const float eventDistance = events[i].d / SpeedOfSound;
    if (eventDistance > farthestDistance) {
      farthestEvent = i;
      farthestDistance = eventDistance;
    }
  }
  if (farthestEvent != -1) return farthestEvent;

  // we've only got playing world sounds to choose from.  pick the
  // most distant one since it's probably the quietest.
  farthestEvent = first;
  farthestDistance = events[farthestEvent].d / SpeedOfSound;
  for (i = first + 1; i < MaxEvents; i++) {
    if ((events[i].flags & (SEF_WORLD | SEF_FIXED)) != SEF_WORLD) continue;
    const float eventDistance = events[i].d / SpeedOfSound;
    if (eventDistance > farthestDistance) {
      farthestEvent = i;
      farthestDistance = eventDistance;
    }
  }

  // replacing an active sound
  portUseCount--;
  return farthestEvent;
}

int PlatformSound::findBestLocalSlot()
{
  // better to lose a world sound
  int slot = findBestWorldSlot();
  if (slot != MaxEvents) return slot;

  // find the first local event
  int i;
  for (i = 0; i < MaxEvents; i++)
    if (!(events[i].flags & SEF_FIXED))
      break;

  // no available slot if only fixed sounds are playing (highly unlikely)
  if (i == MaxEvents) return MaxEvents;

  // find the local sound closest to completion.
  int minEvent = i;
  int minSamplesLeft = events[i].samples->length - events[i].ptr;
  for (i++; i < MaxEvents; i++) {
    if (events[i].flags & SEF_FIXED) continue;
    if (events[i].samples->length - events[i].ptr < minSamplesLeft) {
      minEvent = i;
      minSamplesLeft = events[i].samples->length - events[i].ptr;
    }
  }

  // replacing an active sound
  portUseCount--;
  return minEvent;
}

//
// audioLoop() simply generates samples and keeps the audio hw fed
//
bool PlatformSound::audioInnerLoop()
{
  if (!active())
    return false;

  int i, j;

  /* get time step */
  prevTime = curTime;
  curTime = TimeKeeper::getCurrent() - startTime;
  endTime = -1.0;
  positionDiscontinuity = 0;

  /* get new commands from queue */
  SoundCommand cmd;
  SoundEvent* event;
  while (media->readSoundCommand(&cmd, sizeof(SoundCommand))) {
    switch (cmd.cmd) {
	case SQC_QUIT:
	  return true;

	case SQC_CLEAR:
	  /* FIXME */
	  break;

	case SQC_SET_POS:
	case SQC_JUMP_POS: {
	  positionDiscontinuity = (cmd.cmd == SQC_JUMP_POS);
	  receiverMoved(cmd.data);
	  break;
			   }

	case SQC_SET_VEL:
	  receiverVelocity(cmd.data);
	  break;

	case SQC_SET_VOLUME:
	  volumeAtten = 0.2f * (float)cmd.code;
	  if (volumeAtten <= 0.0f) {
	    mutingOn = true;
	    volumeAtten = 0.0f;
	  }
	  else if (volumeAtten >= 2.0f) {
	    mutingOn = false;
	    volumeAtten = 2.0f;
	  }
	  else {
	    mutingOn = false;
	  }
	  break;

	case SQC_LOCAL_SFX:
	  i = findBestLocalSlot();
	  if (i == MaxEvents) break;
	  event = events + i;

	  event->samples = soundSamples[cmd.code];
	  event->ptr = 0;
	  event->flags = 0;
	  event->time = curTime;
	  event->busy = true;
	  event->lastLeftAtten = event->lastRightAtten = volumeAtten;
	  portUseCount++;
	  break;

	case SQC_IWORLD_SFX:
	case SQC_WORLD_SFX:
	  if (cmd.cmd == SQC_IWORLD_SFX) {
	    i = findBestWorldSlot();
	  }
	  else {
	    for (i = 0; i < MaxEvents; i++)
	      if (!events[i].busy)
		break;
	  }
	  if (i == MaxEvents) break;
	  event = events + i;

	  event->samples = soundSamples[cmd.code];
	  event->ptrFracLeft = 0.0;
	  event->ptrFracRight = 0.0;
	  event->flags = SEF_WORLD | SEF_IGNORING;
	  if (cmd.cmd == SQC_IWORLD_SFX) event->flags |= SEF_IMPORTANT;
	  event->x = cmd.data[0];
	  event->y = cmd.data[1];
	  event->z = cmd.data[2];
	  event->time = curTime;
	  event->busy = true;
	  /* don't increment use count because we're ignoring the sound */
	  recalcEventDistance(event);
	  break;

	case SQC_FIXED_SFX:
	  for (i = 0; i < MaxEvents; i++)
	    if (!events[i].busy)
	      break;
	  if (i == MaxEvents) break;
	  event = events + i;

	  event->samples = soundSamples[cmd.code];
	  event->ptrFracLeft = 0.0;
	  event->ptrFracRight = 0.0;
	  event->flags = SEF_FIXED | SEF_WORLD;
	  event->x = cmd.data[0];
	  event->y = cmd.data[1];
	  event->z = cmd.data[2];
	  event->time = curTime;
	  event->busy = true;
	  portUseCount++;
	  recalcEventDistance(event);
	  break;
    }
  }
  for (i = 0; i < MaxEvents; i++)
    if (events[i].busy) {
      int deltaCount = recalcEventIgnoring(events + i);
      portUseCount += deltaCount;
    }

    /* sum contributions to the port and output samples */
    if (media->isAudioTooEmpty()) {
      long numSamples = 0;
      if (portUseCount != 0) {
	for (j = 0; j < MaxEvents; j++) {
	  if (!events[j].busy) continue;

	  int deltaCount;
	  if (events[j].flags & SEF_WORLD)
	    if (events[j].flags & SEF_FIXED)
	      deltaCount = addFixedContribution(events + j, &numSamples);
	    else
	      deltaCount = addWorldContribution(events + j, &numSamples);
	  else
	    deltaCount = addLocalContribution(events + j, &numSamples);
	  portUseCount += deltaCount;
	}
      }

      // replace all samples with silence if muting is on
      if (mutingOn)
	numSamples = 0;

      // fill out partial buffers with silence
      for (j = numSamples; j < audioBufferSize; j++)
	scratch[j] = 0.0f;

      // write samples
      media->writeAudioFrames(scratch, audioBufferSize >> 1);
    }

    return false;
}

static void		audioLoop(void*)
{
  // loop until requested to stop
  while (true) {
    // sleep until audio buffers hit low water mark or new command available
    if(platformSound)
      platformSound->sleep();

    if (platformSound && platformSound->audioInnerLoop())
      break;
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
