/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "sound.h"
#include "global.h"
#include "TimeKeeper.h"
#include "PlatformFactory.h"
#include "BzfMedia.h"
#include <math.h>

const float		SpeedOfSound = 343.0f;			// meters/sec
const float		MinEventDist = 20.0f * TankRadius;	// meters
const int		MaxEvents = 30;
const float		InterAuralDistance = 0.1f;		// meters

/*
 * producer/consumer shared data types and defines
 */

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

struct SoundCommand {
  public:
    int			cmd;
    int			code;
    float		data[4];
};

typedef struct {
  long			length;		/* total number samples in data */
  long			mlength;	/* total number samples in mono */
  double		dmlength;	/* mlength as a double minus one */
  float*		data;		/* left in even, right in odd */
  float*		mono;		/* avg of channels for world sfx */
  float*		monoRaw;	/* mono with silence before & after */
  double		duration;	/* time to play sound */
} AudioSamples;


/*
 * local functions
 */

static void		sendSound(SoundCommand* s);
static void		audioLoop(void*);
static boolean		allocAudioSamples();
static void		freeAudioSamples(void);
static int		resampleAudio(const float* in,
				int frames, int rate, AudioSamples* out);


/*
 * general purpose audio stuff
 */

static int		usingAudio = 0;
static const char*	soundFiles[] = {
				"fire",
				"explosion",
				"ricochet",
				"flag_grab",
				"flag_drop",
				"flag_won",
				"flag_lost",
				"flag_alert",
				"jump",
				"land",
				"teleport",
				"laser",
				"shock",
				"pop",
				"explosion",
				"flag_grab",
				"boom",
				"killteam",
				"phantom",
				"missile",
				"lock"
			};
#define	SFX_COUNT	((int)(sizeof(soundFiles) / sizeof(soundFiles[0])))

/*
 * producer/consumer shared arena
 */

static AudioSamples	soundSamples[SFX_COUNT];
static long		audioBufferSize;
static int		soundLevel;


void			openSound(const char*)
{
  if (usingAudio) return;			// already opened

  if (!PlatformFactory::getMedia()->openAudio())
    return;

  // initialize buffers
  for (int i = 0; i < SFX_COUNT; i++) {
    soundSamples[i].data = NULL;
    soundSamples[i].mono = NULL;
    soundSamples[i].monoRaw = NULL;
  }

  // open audio data files
  if (!allocAudioSamples()) {
    PlatformFactory::getMedia()->closeAudio();
    return;					// couldn't get samples
  }

  // start audio thread
  if (!PlatformFactory::getMedia()->startAudioThread(audioLoop, NULL)) {
    PlatformFactory::getMedia()->closeAudio();
    freeAudioSamples();
    return;
  }

  setSoundVolume(10);

  usingAudio = 1;
}

void			closeSound(void)
{
  if (!usingAudio) return;

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

  // free memory used for sfx samples
  freeAudioSamples();

  usingAudio = 0;
}

boolean			isSoundOpen()
{
  return usingAudio != 0;
}

static boolean		allocAudioSamples()
{
  boolean anyFile = False;

  for (int i = 0; i < SFX_COUNT; i++) {
    // read it
    int numFrames, rate;
    float* samples = PlatformFactory::getMedia()->
				readSound(soundFiles[i], numFrames, rate);
    if (samples && resampleAudio(samples, numFrames, rate, soundSamples + i))
      anyFile = True;
    delete[] samples;
  }

  return anyFile;
}

static void		freeAudioSamples(void)
{
  for (int i = 0; i < SFX_COUNT; i++) {
    delete[] soundSamples[i].data;
    delete[] soundSamples[i].monoRaw;
  }
}

static int		resampleAudio(const float* in,
				int frames, int rate, AudioSamples* out)
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
 * sound fx producer stuff
 */

static void		sendSound(SoundCommand* s)
{
  if (!usingAudio) return;
  PlatformFactory::getMedia()->writeSoundCommand(s, sizeof(SoundCommand));
}

void			moveSoundReceiver(float x, float y, float z, float t,
							int discontinuity)
{
  SoundCommand s;
  s.cmd = discontinuity ? SQC_JUMP_POS : SQC_SET_POS;
  s.code = 0;
  s.data[0] = x;
  s.data[1] = y;
  s.data[2] = z;
  s.data[3] = t;
  sendSound(&s);
}

void			speedSoundReceiver(float vx, float vy, float vz)
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

void			playWorldSound(int soundCode,
				float x, float y, float z, boolean important)
{
  SoundCommand s;
  if (soundSamples[soundCode].length == 0) return;
  s.cmd = important ? SQC_IWORLD_SFX : SQC_WORLD_SFX;
  s.code = soundCode;
  s.data[0] = x;
  s.data[1] = y;
  s.data[2] = z;
  s.data[3] = 0.0f;
  sendSound(&s);
}

void			playLocalSound(int soundCode)
{
  SoundCommand s;
  if (soundSamples[soundCode].length == 0) return;
  s.cmd = SQC_LOCAL_SFX;
  s.code = soundCode;
  s.data[0] = 0.0f;
  s.data[1] = 0.0f;
  s.data[2] = 0.0f;
  s.data[3] = 0.0f;
  sendSound(&s);
}

void			playFixedSound(int soundCode,
						float x, float y, float z)
{
  SoundCommand s;
  if (soundSamples[soundCode].length == 0) return;
  s.cmd = SQC_FIXED_SFX;
  s.code = soundCode;
  s.data[0] = x;
  s.data[1] = y;
  s.data[2] = z;
  s.data[3] = 0.0f;
  sendSound(&s);
}

void			setSoundVolume(int newLevel)
{
  soundLevel = newLevel;
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

int			getSoundVolume()
{
  return soundLevel;
}


/*
 * Below this point is stuff for real-time audio thread
 */

#define	SEF_WORLD	1
#define	SEF_FIXED	2
#define	SEF_IGNORING	4
#define	SEF_IMPORTANT	8

/* NOTE:
 *	world sounds use the ptrFrac member, local sounds the ptr member.
 *	world sounds only use the monoaural samples so the ptrFrac member
 *	is incremented by 1 for each sample.  local sounds are in stereo
 *	and ptr is incremented by 2 for each (stereo) sample.
 */

typedef struct {
  AudioSamples*		samples;		/* event sound effect */
  boolean		busy;			/* true iff in use */
  long			ptr;			/* current sample */
  double		ptrFracLeft;		/* fractional step ptr */
  double		ptrFracRight;		/* fractional step ptr */
  int			flags;			/* state info */
  float			x, y, z;		/* event location */
  double		time;			/* time of event */
  float			lastLeftAtten;
  float			lastRightAtten;
  float			dx, dy, dz;		/* last relative position */
  float			d;			/* last relative distance */
  float			dLeft;			/* last relative distance */
  float			dRight;			/* last relative distance */
  float			amplitude;		/* last sfx amplitude */
} SoundEvent;

/* list of events currently pending */
static SoundEvent	events[MaxEvents];
static int		portUseCount;
static double		endTime;

/* last position of the receiver */
static float		lastX, lastY, lastZ, lastTheta;
static float		lastXLeft, lastYLeft;
static float		lastXRight, lastYRight;
static float		forwardX, forwardY;
static float		leftX, leftY;
static int		positionDiscontinuity;

/* motion info for Doppler shift */
static float		velX;
static float		velY;
//static float		velZ;

/* volume */
static float		volumeAtten = 1.0f;
static int		mutingOn = 0;

/* scratch buffer for adding contributions from sources */
static float*		scratch;

/* fade in/out table */
const int		FadeDuration = 16;
static float		fadeIn[FadeDuration];
static float		fadeOut[FadeDuration];

/* speed of sound stuff */
static float		timeSizeOfWorld;		/* in seconds */
static TimeKeeper	startTime;
static double		prevTime, curTime;

static void		recalcEventDistance(SoundEvent* e)
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
    e->amplitude = (e->d < MinEventDist) ? 1.0f : MinEventDist / e->d;

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

static int		recalcEventIgnoring(SoundEvent* e)
{
  if ((e->flags & SEF_FIXED) || !(e->flags & SEF_WORLD)) return 0;

  float travelTime = (float)(curTime - e->time);
  if (travelTime > e->samples->duration + timeSizeOfWorld) {
    // sound front has passed all points in world
    e->busy = False;
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

static void		receiverMoved(float* data)
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
    if (events[i].busy && events[i].flags & SEF_WORLD)
      recalcEventDistance(events + i);
}

static void		receiverVelocity(float* data)
{
  static const float s = 1.0f / SpeedOfSound;

  velX = s * data[0];
  velY = s * data[1];
//  velZ = s * data[2];
}

static int		addLocalContribution(SoundEvent* e, long* len)
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
    e->busy = False;
    return -1;
  }

  return 0;
}

static void		getWorldStuff(SoundEvent *e, float* la, float* ra,
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

static int		addWorldContribution(SoundEvent* e, long* len)
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
      fini = True;
    if ((e->ptrFracRight += sampleStep) >= e->samples->dmlength)
      fini = True;
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
      fini = True;
    if ((e->ptrFracRight += sampleStep) >= e->samples->dmlength)
      fini = True;
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

static int		addFixedContribution(SoundEvent* e, long* len)
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

static int		findBestWorldSlot()
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

static int		findBestLocalSlot()
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
static BzfMedia*	media = NULL;
static boolean		usingSameThread = False;

static boolean		audioInnerLoop(boolean noWaiting)
{
    int i, j;

    // sleep until audio buffers hit low water mark or new command available
    media->audioSleep(True, noWaiting ? 0.0 : endTime);

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
	  return True;

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
	    mutingOn = True;
	    volumeAtten = 0.0f;
	  }
	  else if (volumeAtten >= 2.0f) {
	    mutingOn = False;
	    volumeAtten = 2.0f;
	  }
	  else {
	    mutingOn = False;
	  }
	  break;

	case SQC_LOCAL_SFX:
	  i = findBestLocalSlot();
	  if (i == MaxEvents) break;
	  event = events + i;

	  event->samples = soundSamples + cmd.code;
	  event->ptr = 0;
	  event->flags = 0;
	  event->time = curTime;
	  event->busy = True;
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

	  event->samples = soundSamples + cmd.code;
	  event->ptrFracLeft = 0.0;
	  event->ptrFracRight = 0.0;
	  event->flags = SEF_WORLD | SEF_IGNORING;
	  if (cmd.cmd == SQC_IWORLD_SFX) event->flags |= SEF_IMPORTANT;
	  event->x = cmd.data[0];
	  event->y = cmd.data[1];
	  event->z = cmd.data[2];
	  event->time = curTime;
	  event->busy = True;
	  /* don't increment use count because we're ignoring the sound */
	  recalcEventDistance(event);
	  break;

	case SQC_FIXED_SFX:
	  for (i = 0; i < MaxEvents; i++)
	    if (!events[i].busy)
	      break;
	  if (i == MaxEvents) break;
	  event = events + i;

	  event->samples = soundSamples + cmd.code;
	  event->ptrFracLeft = 0.0;
	  event->ptrFracRight = 0.0;
	  event->flags = SEF_FIXED | SEF_WORLD;
	  event->x = cmd.data[0];
	  event->y = cmd.data[1];
	  event->z = cmd.data[2];
	  event->time = curTime;
	  event->busy = True;
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

    return False;
}

static void		audioLoop(void*)
{
  int i;

  media = PlatformFactory::getMedia();
  audioBufferSize = media->getAudioBufferChunkSize() << 1;

  /* initialize */
  timeSizeOfWorld = 1.414f * WorldSize / SpeedOfSound;
  for (i = 0; i < MaxEvents; i++) {
    events[i].samples = NULL;
    events[i].busy = False;
  }
  portUseCount = 0;
  for (i = 0; i < FadeDuration; i += 2) {
    fadeIn[i] = fadeIn[i+1] =
		sinf(M_PI / 2.0f * (float)i / (float)(FadeDuration-2));
    fadeOut[i] = fadeOut[i+1] = 1.0f - fadeIn[i];
  }
  scratch = new float[audioBufferSize];

  startTime = TimeKeeper::getCurrent();
  curTime = 0.0;
  endTime = -1.0;

  // if using same thread then return immediately
  usingSameThread = !media->hasAudioThread();
  if (usingSameThread) return;

  // loop until requested to stop
  boolean done = False;
  while (!done) {
    if (audioInnerLoop(False))
      done = True;
  }

  delete[] scratch;
}

void			updateSound()
{
  if (isSoundOpen() && usingSameThread)
    audioInnerLoop(True);
}
// ex: shiftwidth=2 tabstop=8
