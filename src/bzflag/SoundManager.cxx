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

#include "SoundManager.h"
#include "TimeKeeper.h"
#include "StateDatabase.h"
#include "MediaFile.h"
#include "PlatformMediaFactory.h"

SoundManager*	SoundManager::mgr = NULL;
std::vector<std::string> SoundManager::soundFiles;

void onVolumeChange(const std::string& name, void*)
{
	if (name == "audioVolume")
		SOUNDMGR->setVolume(atoi(BZDB->get(name).c_str()));
	else if (name == "audioMute")
		SOUNDMGR->setMute(BZDB->isTrue("audioMute"));
}

SoundManager::SoundManager()
{
	usingAudio = false;
	soundFiles.clear();
	soundFiles.push_back("fire");
	soundFiles.push_back("explosion");
	soundFiles.push_back("ricochet");
	soundFiles.push_back("flag_grab");
	soundFiles.push_back("flag_drop");
	soundFiles.push_back("flag_won");
	soundFiles.push_back("flag_lost");
	soundFiles.push_back("flag_alert");
	soundFiles.push_back("jump");
	soundFiles.push_back("land");
	soundFiles.push_back("teleport");
	soundFiles.push_back("laser");
	soundFiles.push_back("shock");
	soundFiles.push_back("pop");
	soundFiles.push_back("boom");
	soundFiles.push_back("killteam");
	soundFiles.push_back("phantom");
	soundFiles.push_back("missile");
	soundFiles.push_back("lock");
	soundFiles.push_back("thief");
//	soundFiles.push_back("rain");
}

void			SoundManager::openSound(const char*)
{
	if (usingAudio)
		return;			// already opened
	if (!MPLATFORM->openAudio())
		return;

	// open audio data files
	if (!allocAudioSamples()) {
		MPLATFORM->closeAudio();
		return;
	}

	// start audio thread
	if (!MPLATFORM->startAudioThread(audioLoop, NULL)) {
		MPLATFORM->closeAudio();
		freeAudioSamples();
		return;
	}

	// watch for volume changes
	BZDB->addCallback("audioVolume", onVolumeChange, NULL);
	BZDB->addCallback("audioMute", onVolumeChange, NULL);
	onVolumeChange("audioVolume", NULL);

	usingAudio = true;
}

void			SoundManager::closeSound()
{
	if (!usingAudio)
		return;

	// send quit command
	sendJob(SQC_QUIT, "", "", false, 0, 0, 0, 0, 0);

	// stop audio thread
	MPLATFORM->stopAudioThread();

	// reset hardware
	MPLATFORM->closeAudio();

	// free memory
	freeAudioSamples();

	usingAudio = false;
}

bool			SoundManager::isOpen()
{
	return usingAudio;
}

void			SoundManager::moveSoundReceiver(float x, float y, float z,
									float t, bool discontinuity)
{
	sendJob(discontinuity ? SQC_JUMP_POS : SQC_SET_POS, "", "", false, 0, x, y, z, t);
}

void			SoundManager::speedSoundReceiver(float vx, float vy, float vz)
{
	sendJob(SQC_SET_VEL, "", "", false, 0, vx, vy, vz, 0);
}

void			SoundManager::playWorldSound(std::string sound, float x, float y,
								float z, bool important)
{
	sendJob(important ? SQC_IWORLD_SFX : SQC_WORLD_SFX, sound, "", false,
				0, x, y, z, 0);
}

void			SoundManager::playLocalSound(std::string sound)
{
	sendJob(SQC_LOCAL_SFX, sound, "", false, 0, 0, 0, 0, 0);
}

void			SoundManager::playLoopedLocalSound(std::string sound, std::string id)
{
	sendJob(SQC_LOCAL_SFX, sound, id, true, 0, 0, 0, 0, 0);
}

void			SoundManager::addJob(std::string jobname, AudioJob& job)
{
	jobs.insert(std::pair<std::string, AudioJob>(jobname, job));
}

void			SoundManager::removeJob(std::string jobname)
{
	JobMap::iterator it = jobs.find(jobname);
	sendJob(SQC_DEL_JOB, "", jobname, false, 0, 0, 0, 0, 0);
	jobs.erase(it);
}

void			SoundManager::setVolume(int _volume)
{
	volume = _volume;
	if(volume < 0)
		volume = 0;
	if(volume > 10)
		volume = 10;

	// send command
	sendJob(SQC_SET_VOLUME, "", "", false, volume, 0, 0, 0, 0);
}

int				SoundManager::getVolume()
{
	return volume;
}

void			SoundManager::setMute(bool _mute)
{
	mute = _mute;
	setVolume(volume);
}

bool			SoundManager::getMute()
{
	return mute;
}

SoundManager*	SoundManager::getInstance()
{
	if (mgr == NULL)
		mgr = new SoundManager;
	return mgr;
}

bool			SoundManager::allocAudioSamples()
{
	AudioSamples samples;
	int frames, rate;
	float *samp;

	for (unsigned int i = 0; i < soundFiles.size(); i++) {
		samp = MediaFile::readSound(soundFiles[i], &frames, &rate);
		if (samp != NULL) {
			if (resampleAudio(samp, frames, rate, &samples))
				sounds[soundFiles[i]] = samples;
			else
				return false;
			delete [] samp;
		}
	}

	return true;
}

void			SoundManager::freeAudioSamples()
{
	for (SampleMap::iterator it = sounds.begin(); it != sounds.end(); it++) {
		delete [] it->second.data;
		delete [] it->second.monoRaw;
		sounds.erase(it);
	}
}

int				SoundManager::resampleAudio(const float* in, int frames,
									int rate, AudioSamples* out)
{
	// attenuation on all sounds
	static const float GlobalAtten = 0.5;

	if (rate != MPLATFORM->getAudioOutputRate()) {
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
				(float)MPLATFORM->getAudioOutputRate());

	// fill in samples structure
	out->length = 2 * frames;
	out->mlength = out->length >> 1;
	out->dmlength = (double)(out->mlength - 1);
	out->duration = (float)out->mlength /
						(float)MPLATFORM->getAudioOutputRate();
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
		out->data[dst]    = GlobalAtten * in[dst];
		out->data[dst+1]  = GlobalAtten * in[dst+1];
		out->mono[dst>>1] = 0.5f * (out->data[dst] + out->data[dst+1]);
	}

	// silence in safety margins
	for (int i = 0; i < safetyMargin; ++i)
		out->monoRaw[i] = out->monoRaw[out->mlength + safetyMargin + i] = 0.0f;

	return 1;
}

void			SoundManager::sendJob(int command, std::string sound, std::string id, bool loop, int code, float a, float b, float c, float d)
{
	AudioJob job;
	job.command = command;
	strcpy(job.sound, sound.c_str());
	strcpy(job.id, id.c_str());
	job.loop = loop;
	job.code = code;
	job.data[0] = a;
	job.data[1] = b;
	job.data[2] = c;
	job.data[3] = d;
	MPLATFORM->writeSoundCommand(&job, sizeof(job));
}

// below this point is for the real-time audio thread/process

#define SEF_WORLD		1
#define SEF_IGNORING	4
#define SEF_IMPORTANT	8

struct SoundEvent {
	std::string 	id;				// semi-unique id, used for removal
	AudioSamples*	samples;		// event sound
	bool			busy;			// true iff in use
	long			ptr;			// current sample
	double			ptrFracLeft;	// fractional step ptr
	double			ptrFracRight;	// fractional step ptr
	int				flags;			// state info
	float			x, y, z;		// event location
	double			time;			// time of event
	float			lastLeftAtten;
	float			lastRightAtten;
	float			dx, dy, dz;		// last relative position
	float			d;				// last relative distance
	float			dLeft;			// last relative distance
	float			dRight;			// last relative distance
	float			amplitude;		// last sfx amplitude
	bool			loop;			// loop?
};

// size of audio buffer
static long			audioBufferSize;

// list of events currently pending
static SoundEvent	events[MaxEvents];
static int			portUseCount;
static double		endTime;

// last position of the reciever
static float		lastX, lastY, lastZ, lastTheta;
static float		lastXLeft, lastYLeft;
static float		lastXRight, lastYRight;
static float		forwardX, forwardY;
static float		leftX, leftY;
static bool			positionDiscontinuity;

// motion info for doppler shift
static float		velX;
static float		velY;
//static float		velZ;

// volume
static float		volumeAtten = 1.0;
static bool			mutingOn = false;

// scratch buffer for adding contributions from sources
static float*		scratch;

// fade in/out table
const int			fadeDuration = 16;
static float		fadeIn[fadeDuration];
static float		fadeOut[fadeDuration];

// speed of sound stuff
static float		timeSizeOfWorld;	// in seconds
static TimeKeeper	startTime;
static double		prevTime, curTime;

static void recalcEventDistance(SoundEvent* e)
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

static int recalcEventIgnoring(SoundEvent* e)
{
	if(!(e->flags & SEF_WORLD))
		return 0;
	
	float travelTime = (float) (curTime - e->time);
	if (travelTime > e->samples->duration + timeSizeOfWorld) {
		// sound front has passed all points in world
		e->busy = false;
		return ((e->flags & SEF_IGNORING) ? 0 : -1);
	}

	int useChange = 0;
	float eventDistance = e->d / SpeedOfSound;
	if (travelTime < eventDistance) {
		if (e->flags & SEF_IGNORING) {
			// do nothing, still ignoring
		}
		else {
			e->flags |= SEF_IGNORING;
			useChange = -1;
		}
		// don't sleep past the time the sound front will pass by
		endTime = eventDistance;
	}
	else {
		float timeFromFront;
		if (e->flags & SEF_IGNORING) {
			// compute time from sound front
			timeFromFront = travelTime - e->dLeft / SpeedOfSound;
			if (!positionDiscontinuity && timeFromFront < 0.0)
				timeFromFront = 0.0;

			// recompute sample pointers
			e->ptrFracLeft = timeFromFront *
				(float) MPLATFORM->getAudioOutputRate();
			if (e->ptrFracLeft >= 0.0 && e->ptrFracLeft < e->samples->dmlength) {
				// not ignoring anymore
				e->flags &= ~SEF_IGNORING;
				useChange = 1;
			}

			// now do it again for the right ear
			timeFromFront = travelTime - e->dRight / SpeedOfSound;
			if (!positionDiscontinuity && timeFromFront < 0.0f)
				timeFromFront = 0.0f;
			e->ptrFracRight = timeFromFront *
				(float)MPLATFORM->getAudioOutputRate();
			if (e->ptrFracRight >= 0.0 && e->ptrFracRight < e->samples->dmlength) {
				e->flags &= ~SEF_IGNORING;
				useChange = 1;
			}
		}
		else {
			// do nothing, not ignoring
		}
	}
	return useChange;
}

static void recieverMoved(float x, float y, float z, float t)
{
	// save data
	lastX = x;
	lastY = y;
	lastZ = z;
	lastTheta = t;

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

static void recieverVelocity(float x, float y, float)
{
	static const float s = 1.0f / SpeedOfSound;

	velX = s * x;
	velY = s * y;
//	velZ = s * z;
}

static int addLocalContribution(SoundEvent* e, long& len)
{
	long			n, numSamples;
	float*			src;

	numSamples = e->samples->length - e->ptr;
	if (numSamples > audioBufferSize)
		numSamples = audioBufferSize;

	if (!mutingOn && numSamples != 0) {
		// initialize new areas of scratch space and adjust output sample count
		if (numSamples > len) {
			for (n = len; n < numSamples; n += 2)
				scratch[n] = scratch[n + 1] = 0.0;
			len = numSamples;
		}

		// add contribution -- conditionals outside loop for run-time efficiency
		src = e->samples->data + e->ptr;
		if (numSamples <= fadeDuration) {
			for (n = 0; n < numSamples; n += 2) {
				int fs = int(fadeDuration * float(n) / float(numSamples)) & ~1;
				scratch[n] += src[n] * (fadeIn[fs] * volumeAtten +
											fadeOut[fs] * e->lastLeftAtten);
				scratch[n + 1] += src[n + 1] * (fadeIn[fs] * volumeAtten +
											fadeOut[fs] * e->lastRightAtten);
			}
		}
		else {
			for (n = 0; n < fadeDuration; n += 2) {
				scratch[n] += src[n] * (fadeIn[n] * volumeAtten +
											fadeOut[n] * e->lastLeftAtten);
				scratch[n + 1] += src[n + 1] * (fadeIn[n] * volumeAtten +
											fadeOut[n] * e->lastRightAtten);
			}
			if (volumeAtten == 1.0) {
				for (; n < numSamples; n += 2) {
					scratch[n] += src[n];
					scratch[n + 1] += src[n + 1];
				}
			}
			else {
				for (; n < numSamples; n += 2) {
					scratch[n] += src[n] * volumeAtten;
					scratch[n + 1] += src[n + 1] * volumeAtten;
				}
			}
		}
	}

	// free event if ran out of samples
	if ((e->ptr += numSamples) == e->samples->length) {
		e->busy = false;
		return -1;
	}

	return 0;
}

static int addLoopedLocalContribution(SoundEvent* e, long& len)
{
	long			n, x;
	float*			src;

	len = audioBufferSize;

	// add contribution -- conditionals outside loop for run-time efficiency
	src = e->samples->data;
	x = 0;
	if (volumeAtten == 1.0) {
		for (n = e->ptr; n < e->ptr + audioBufferSize; n += 2) {
			scratch[x] += src[n % e->samples->length];
			scratch[x + 1] += src[(n + 1) % e->samples->length];
			x++;
		}
	}
	else {
		for (n = e->ptr; n < e->ptr + audioBufferSize; n += 2) {
			scratch[x] += src[n % e->samples->length] * volumeAtten;
			scratch[x + 1] += src[(n + 1) % e->samples->length] * volumeAtten;
			x++;
		}
	}
	e->ptr = (e->ptr + audioBufferSize) % e->samples->length;

	return 0;
}

static void getWorldStuff(SoundEvent* e, float* la, float* ra, double* sampleStep)
{
	float leftAtten, rightAtten;

	// compute left and right attenuation factors
	// FIXME -- should be a more general HRTF
	if (e->d == 0.0) {
		leftAtten = 1.0;
		rightAtten = 1.0;
	}
	else {
		const float ff = (2.0f + forwardX * e->dx + forwardY * e->dy) / 3.0f;
		const float fl = (2.0f + leftX * e->dx + leftY * e->dy) / 3.0f;
		leftAtten = ff * fl * e->amplitude;
		rightAtten = ff * (4.0f/3.0f - fl) * e->amplitude;
	}

	if (e->ptrFracLeft == 0.0 || e->ptrFracRight == 0.0) {
		e->lastLeftAtten = leftAtten;
		e->lastRightAtten = rightAtten;
	}

	*la = mutingOn ? 0.0 : leftAtten * volumeAtten;
	*ra = mutingOn ? 0.0 : rightAtten * volumeAtten;

	// compute doppler effect
	*sampleStep = double(1.0 + velX * e->dx + velY * e->dy);
}

static int addWorldContribution(SoundEvent* e, long& len)
{
	bool		fini = false;
	long		n, nmL, nmR;
	float*		src = e->samples->mono;
	float		leftAtten, rightAtten, fracL, fracR, fsampleL, fsampleR;
	double		sampleStep;

	if (e->flags & SEF_IGNORING)
		return 0;

	getWorldStuff(e, &leftAtten, &rightAtten, &sampleStep);
	if (sampleStep < 0.0)
		fini = true;

	// initialize new areas of scratch space and adjust output sample count
	if (len < audioBufferSize) {
		for (n = len; n < audioBufferSize; n += 2)
			scratch[n] = scratch[n + 1] = 0.0;
		len = audioBufferSize;
	}

	// add contribution with crossfade
	for (n = 0; !fini && n < fadeDuration; n += 2) {
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

	// NOTE: running out of samples just means the world sound front
	// has passed our location.  if we teleport it may pass us again.
	// so we can't free the event until the front passes out of the
	// world.  compute time remaining until that happens and set
	// endTime if smaller than current endTime.
	if (fini) {
		double et = e->samples->duration + timeSizeOfWorld - (prevTime - e->time);
		if (endTime == -1.0 || et < endTime)
			endTime = et;
		e->flags |= SEF_IGNORING;
		return -1;
	}

	return 0;
}

static int findBestWorldSlot()
{
	int i;

	// the best slot is an empty one
	for (i = 0; i < MaxEvents; i++)
		if (!events[i].busy)
			return i;

	// no available slots. find an existing slot that won't be missed
	// (much).  this will cause a pop or crackle if the replaced sound is
	// currently playing.  first see if there are any world events.
	for (i = 0; i < MaxEvents; i++)
		if (events[i].flags & SEF_WORLD)
			break;

	// give up if no (non-fixed) world events
	if (i == MaxEvents)
		return MaxEvents;

	// found a world event.  see if there's an event that's
	// completely passed us.
	const int first = i;
	for (i = first; i < MaxEvents; i++) {
		if (!(events[i].flags & SEF_WORLD))
			continue;
		if (!(events[i].flags & SEF_IGNORING))
			continue;
		const float travelTime = (float)(curTime - events[i].time);
		const float eventDistance = events[i].d / SpeedOfSound;
		if (travelTime > eventDistance)
			return i;
	}

	// if no sound front has completely passed our position
	// then pick the most distant one that hasn't reached us
	// yet that isn't important.
	int farthestEvent = -1;
	float farthestDistance = 0.0f;
	for (i = first; i < MaxEvents; i++) {
		if (events[i].flags & SEF_IMPORTANT)
			continue;
		if (!(events[i].flags & SEF_WORLD))
			continue;
		if (!(events[i].flags & SEF_IGNORING))
			continue;
		const float eventDistance = events[i].d / SpeedOfSound;
		if (eventDistance > farthestDistance) {
			farthestEvent = i;
			farthestDistance = eventDistance;
		}
	}
	if (farthestEvent != -1)
		return farthestEvent;

	// same thing but look at important sounds
	for (i = first; i < MaxEvents; i++) {
		if (!(events[i].flags & SEF_IMPORTANT))
			continue;
		if (!(events[i].flags & SEF_WORLD))
			continue;
		if (!(events[i].flags & SEF_IGNORING))
			continue;
		const float eventDistance = events[i].d / SpeedOfSound;
		if (eventDistance > farthestDistance) {
			farthestEvent = i;
			farthestDistance = eventDistance;
		}
	}
	if (farthestEvent != -1)
		return farthestEvent;
	
	// we've only got playing world sounds to choose from.  pick the
	// most distant one since it's probably the quietest.
	farthestEvent = first;
	farthestDistance = events[farthestEvent].d / SpeedOfSound;
	for (i = first + 1; i < MaxEvents; i++) {
		if (!(events[i].flags & SEF_WORLD))
			continue;
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

static int findBestLocalSlot()
{
	// better to lose a world sound
	int slot = findBestWorldSlot();
	if (slot != MaxEvents)
		return slot;

	// find the first local event
	int i;
	for (i = 0; i < MaxEvents; i++)
		if (!(events[i].flags & SEF_WORLD))
			break;
	
	// no available slot if only world sounds are playing (highly unlikely)
	if (i == MaxEvents)
		return MaxEvents;

	// find the local sound closest to completion.
	int minEvent = i;
	int minSamplesLeft = events[i].samples->length - events[i].ptr;
	for (i++; i < MaxEvents; i++) {
		if (events[i].flags & SEF_WORLD)
			continue;
		if (events[i].samples->length - events[i].ptr < minSamplesLeft) {
			minEvent = i;
			minSamplesLeft = events[i].samples->length - events[i].ptr;
		}
	}

	// replacing an active sound
	portUseCount--;
	return minEvent;
}

static bool						usingSameThread = false;

static bool audioInnerLoop(bool noWaiting)
{
	int i, j;

	// sleep until audio buffers hit low water mark or new command available
	MPLATFORM->audioSleep(true, noWaiting ? 0.0 : endTime);

	// get time step
	prevTime = curTime;
	curTime = TimeKeeper::getCurrent() - startTime;
	endTime = -1.0;
	positionDiscontinuity = 0;

	// get new commands from queue
	AudioJob job;
	SoundEvent* event;
	while (MPLATFORM->readSoundCommand(&job, sizeof(job))) {
		switch (job.command) {
			case SQC_CLEAR:
				// FIXME
				break;

			case SQC_SET_POS:
			case SQC_JUMP_POS:
				{
					recieverMoved(job.data[0], job.data[1], job.data[2], job.data[3]);
				}
				break;
				
			case SQC_SET_VEL:
				{
					recieverVelocity(job.data[0], job.data[1], job.data[2]);
				}
				break;

			case SQC_SET_VOLUME:
				{
					volumeAtten = 0.2f * (float) job.code;
					if (volumeAtten <= 0.0f) {
						mutingOn = true;
						volumeAtten = 0;
					}
					else if (volumeAtten >= 2.0f) {
						mutingOn = false;
						volumeAtten = 2.0;
					}
					else {
						mutingOn = false;
					}
				}
				break;

			case SQC_LOCAL_SFX:
				{
					i = findBestLocalSlot();
					if (i == MaxEvents)
						break;
					event = events + i;
					event->samples = &SOUNDMGR->sounds[job.sound];
					event->id = job.id;
					event->ptr = 0;
					event->flags = 0;
					event->time = curTime;
					event->busy = true;
					event->lastLeftAtten = event->lastRightAtten = volumeAtten;
					event->loop = job.loop;
					portUseCount++;
				}
				break;

			case SQC_IWORLD_SFX:
			case SQC_WORLD_SFX:
				{
					if (job.command == SQC_IWORLD_SFX) {
						i = findBestWorldSlot();
					}
					else {
						for (i = 0; i < MaxEvents; i++)
							if (!events[i].busy)
								break;
					}
					if (i == MaxEvents)
						break;
					event = events + i;
					event->samples = &SOUNDMGR->sounds[job.sound];
					event->ptrFracLeft = event->ptrFracRight = 0.0;
					event->flags = SEF_WORLD | SEF_IGNORING;
					if (job.command == SQC_IWORLD_SFX)
						event->flags |= SEF_IMPORTANT;
					event->x = job.data[0];
					event->x = job.data[1];
					event->x = job.data[2];
					event->time = curTime;
					event->busy = true;
					// don't increment use count because we're ignoring the sound
					recalcEventDistance(event);
				}
				break;

			case SQC_DEL_JOB:
				{
					for (int i = 0; i < MaxEvents; i++) {
						if (events[i].id == std::string(job.id))
							events[i].busy = false;
					}
				}
				break;
			case SQC_QUIT:
				return true;
		}
	}
	for (i = 0; i < MaxEvents; i++) {
		if (events[i].busy) {
			int deltaCount = recalcEventIgnoring(events + i);
			portUseCount += deltaCount;
		}
	}

	// sum contributions to the port and output samples
	if (MPLATFORM->isAudioTooEmpty()) {
		long numSamples = 0;
		if (portUseCount != 0) {
			for (j = 0; j < MaxEvents; j++) {
				if (!events[j].busy)
					continue;

				int deltaCount;
				if (events[j].flags & SEF_WORLD)
					deltaCount = addWorldContribution(events + j, numSamples);
				else
					if (events[j].loop)
						deltaCount = addLoopedLocalContribution(events + j, numSamples);
					else
						deltaCount = addLocalContribution(events + j, numSamples);
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
		MPLATFORM->writeAudioFrames(scratch, audioBufferSize >> 1);
	}

	return false;
}

void audioLoop(void*)
{
	audioBufferSize = MPLATFORM->getAudioBufferChunkSize() << 1;

	// initialize
	timeSizeOfWorld = 1.414 * WorldSize / SpeedOfSound;
	for (int i = 0; i < MaxEvents; i++) {
		events[i].samples = NULL;
		events[i].busy = false;
	}
	portUseCount = 0;
	for (int i = 0; i < fadeDuration; i+= 2) {
		fadeIn[i] = fadeIn[i+1] = sinf(M_PI / 2.0 * (float) i / (float) (fadeDuration - 2));
		fadeOut[i] = fadeOut[i + 1] = 1.0 - fadeIn[i];
	}

	scratch = new float[audioBufferSize];

	startTime = TimeKeeper::getCurrent();
	curTime = 0.0;
	endTime = -1.0;

	// if using same thread then return immediately
	usingSameThread = !MPLATFORM->hasAudioThread();
	if (usingSameThread)
		return;

	// loop until requested to stop
	bool done = false;
	while (!done) {
		if (audioInnerLoop(false))
			done = true;
	}

	delete [] scratch;
}

void updateSound()
{
	if (SOUNDMGR->isOpen() && usingSameThread)
		audioInnerLoop(true);
}
// ex: shiftwidth=4 tabstop=4
