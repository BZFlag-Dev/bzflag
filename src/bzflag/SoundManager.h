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

#ifndef BZF_SOUND_MANAGER_H
#define BZF_SOUND_MANAGER_H

#include "common.h"
#include "bzfio.h"
#include "global.h"
#include <string>
#include <map>
#include <vector>
#ifndef WIN32
#include <multimap.h>
#endif

#define SOUNDMGR (SoundManager::getInstance())

const float			SpeedOfSound = 343.0;				// meters/sec
const float			MinEventDist = 20.0 * TankRadius;	// meters
const int			MaxEvents = 30;
const float			InterAuralDistance = 0.1f;			// meters

// sound queue commands
#define SQC_CLEAR		0		// no data
#define SQC_SET_POS		1		// 4xfloat, x,y,z,t
#define SQC_JUMP_POS	2		// 4xfloat, x,y,z,t
#define SQC_SET_VEL		3		// 3xfloat, x,y,z
#define SQC_SET_VOLUME	4		// 1xint, volume
#define SQC_LOCAL_SFX	5		// 1xjob
#define SQC_WORLD_SFX	6		// 3xfloat, 1xjob
#define SQC_IWORLD_SFX	7		// 3xfloat, 1xjob
#define SQC_DEL_JOB		8		// 1xjob
#define SQC_QUIT		9		// no data

struct AudioSamples {
public:
	long			length;		// total number of samples in data
	long			mlength;	// total number of samples in mono
	double			dmlength;	// mlength as a double minus one
	float*			data;		// left in even, right in odd
	float*			mono;		// avg of channels for world sfx
	float*			monoRaw;	// mono with silence before and after
	double			duration;	// time to play sound
};

struct AudioJob {
	// for now this structure is limited to 100 bytes.
	// make the best of it
	int				command;
	char			sound[35];
	char			id[35];
	bool			loop;
	int				code;
	float			data[4];
};

void			onVolumeChange(const std::string&, void*);
class SoundManager {
public:
	void			openSound(const char* pname);
	void			closeSound();
	bool			isOpen();

	// reposition sound receiver (no Doppler)
	void			moveSoundReceiver(float x, float y, float z,
										float t, bool discontinuity);
	// move sound reciever (w/Doppler effect)
	void			speedSoundReceiver(float vx, float vy, float vz);
	// sound effect event at given position in world
	void			playWorldSound(std::string sound, float x, float y,
										float z, bool important = false);
	// sound effect positioned at receiver
	void			playLocalSound(std::string sound);
	// looped local sound
	void			playLoopedLocalSound(std::string sound, std::string id);

	void			addJob(std::string jobname, AudioJob& job);
	void			removeJob(std::string jobname);

	void			setVolume(int _volume);
	int				getVolume();
	void			setMute(bool _mute);
	bool			getMute();

	// get the single instance
	static SoundManager*	getInstance();

public:
	typedef std::map<std::string, AudioSamples> 	SampleMap;

	SampleMap		sounds;

private:
	SoundManager();
	bool			allocAudioSamples();
	void			freeAudioSamples();

	int				resampleAudio(const float* in, int frames,
									int rate, AudioSamples* out);

	void			sendJob(int command, std::string sound, std::string id, bool loop, int code, float a, float b, float c, float d);

private:
	typedef std::multimap<std::string, AudioJob>	JobMap;

	JobMap			jobs;
	int				volume;		// 0 = mute, 1-10
	bool			mute;
	bool			usingAudio;

	static SoundManager*	mgr;
	static std::vector<std::string> soundFiles;
};

void audioLoop(void*);

#endif
// ex: shiftwidth=4 tabstop=4
