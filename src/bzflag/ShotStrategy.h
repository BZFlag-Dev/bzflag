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

/*
 * ShotStrategy:
 *	Interface for all shot flight path strategies.  A
 *	strategy encapsulates the algorithm for computing
 *	the path taken by a shot.
 */

#ifndef BZF_SHOT_STRATEGY_H
#define BZF_SHOT_STRATEGY_H

#include "common.h"
#include "ShotPath.h"
#include "Obstacle.h"
#include "Teleporter.h"
#include "math3D.h"
#include "TimeKeeper.h"
#include <vector>

class BaseLocalPlayer;
class SceneNode;
class SceneNodeGroup;
class SceneNodeTransform;
class SceneNodeParameters;

class ShotStrategy {
public:
	ShotStrategy(ShotPath*);
	virtual ~ShotStrategy();

	virtual void		update(float dt) = 0;
	virtual float		checkHit(const BaseLocalPlayer*, float pos[3]) const = 0;
	virtual bool		isStoppedByHit() const;
	virtual void		addShot(SceneNodeGroup*, bool colorblind) = 0;
	virtual void		expire();
	virtual void		radarRender() const = 0;

	// prepare a shot update packet and send it. first part of message must be
	// the ShotUpdate portion of FiringInfo.
	virtual void		sendUpdate(const FiringInfo&) const;

	// update shot based on message.  code is the message code.  msg
	// points to the part of the message after the ShotUpdate portion.
	virtual void		readUpdate(uint16_t code, void* msg);

protected:
	const ShotPath&		getPath() const;
	FiringInfo&			getFiringInfo(ShotPath*) const;
	void				setReloadTime(float) const;
	void				setPosition(const float*) const;
	void				setVelocity(const float*) const;
	void				setExpiring() const;
	void				setExpired() const;

	const Obstacle*		getFirstBuilding(const Ray&, float min, float& t) const;
	const Teleporter*	getFirstTeleporter(
							const Ray&, float min, float& t, int& f) const;

	static SceneNode*	findShotModel(TeamColor team,
								const std::string& flagAbbrv);

private:
	ShotPath*			path;
};

class ShotPathSegment {
public:
	enum Reason { Initial, Through, Ricochet, Teleport };

	ShotPathSegment();
	ShotPathSegment(const TimeKeeper& start,
							const TimeKeeper& end,
							const Ray& r,
							Reason = Initial);
	ShotPathSegment(const ShotPathSegment&);
	~ShotPathSegment();
	ShotPathSegment&	operator=(const ShotPathSegment&);

public:
	TimeKeeper			start;
	TimeKeeper			end;
	Ray					ray;
	Reason				reason;
	float				bbox[2][3];
};

typedef std::vector<ShotPathSegment> ShotPathSegments;

class SegmentedShotStrategy : public ShotStrategy {
public:
	SegmentedShotStrategy(ShotPath*);
	~SegmentedShotStrategy();

	void				update(float dt);
	float				checkHit(const BaseLocalPlayer*, float[3]) const;
	void				addShot(SceneNodeGroup*, bool colorblind);
	void				radarRender() const;

protected:
	enum ObstacleEffect {
						Stop = 0,
						Through = 1,
						Reflect = 2
	};
	void				makeSegments(ObstacleEffect = Stop);
	const ShotPathSegments&		getSegments() const;
	static void			reflect(float* v, const float* n); // const

	void				setCurrentTime(const TimeKeeper&);
	const TimeKeeper&	getLastTime() const;

	bool				isOverlapping(const float (*bbox1)[3],
								const float (*bbox2)[3]) const;

	void				setCurrentSegment(int segment);

private:
	TimeKeeper			prevTime;
	TimeKeeper			currentTime;
	TimeKeeper			lastTime;
	int					segment, lastSegment;
	ShotPathSegments	segments;
	SceneNode*			teamSceneNode;
	SceneNode*			rogueSceneNode;
	SceneNodeTransform*	transformSceneNode;
	TeamColor			team;
	float				bbox[2][3];
	int					firstSegment;
};

class NormalShotStrategy : public SegmentedShotStrategy {
public:
	NormalShotStrategy(ShotPath*);
	~NormalShotStrategy();
};

class RapidFireStrategy : public SegmentedShotStrategy {
public:
	RapidFireStrategy(ShotPath*);
	~RapidFireStrategy();
};

class MachineGunStrategy : public SegmentedShotStrategy {
public:
	MachineGunStrategy(ShotPath*);
	~MachineGunStrategy();
};

class LaserStrategy : public SegmentedShotStrategy {
public:
	LaserStrategy(ShotPath*);
	~LaserStrategy();

	void				update(float dt);
	bool				isStoppedByHit() const;
	void				addShot(SceneNodeGroup*, bool colorblind);
	void				radarRender() const;

private:
	SceneNode*			createSegment(int) const;

private:
	float				cumTime;
	float				endTime;
	SceneNodeGroup*		nodes;
	SceneNodeGroup*		model;
	SceneNode*			teamSceneNode;
	SceneNode*			rogueSceneNode;
};

class ThiefStrategy : public SegmentedShotStrategy {
public:
	ThiefStrategy(ShotPath*);
	~ThiefStrategy();

	void				update(float dt);
	virtual bool		isStoppedByHit() const;
	void				addShot(SceneNodeGroup*, bool colorblind);
	void				radarRender() const;

private:
	SceneNode*			createSegment(int) const;

private:
	float				cumTime;
	float				endTime;
	SceneNodeGroup*		nodes;
	SceneNodeGroup*		model;
	SceneNode*			teamSceneNode;
	SceneNode*			rogueSceneNode;
};

class RicochetStrategy : public SegmentedShotStrategy {
public:
	RicochetStrategy(ShotPath*);
	~RicochetStrategy();
};

class SuperBulletStrategy : public SegmentedShotStrategy {
public:
	SuperBulletStrategy(ShotPath*);
	~SuperBulletStrategy();
};

class GuidedMissileStrategy : public ShotStrategy {
public:
	GuidedMissileStrategy(ShotPath*);
	~GuidedMissileStrategy();

	void				update(float dt);
	float				checkHit(const BaseLocalPlayer*, float[3]) const;
	void				sendUpdate(const FiringInfo&) const;
	void				readUpdate(uint16_t, void*);
	void				addShot(SceneNodeGroup*, bool colorblind);
	void				expire();
	void				radarRender() const;

private:
	float				checkBuildings(const Ray& ray);

private:
	TimeKeeper			prevTime;
	TimeKeeper			currentTime;
	ShotPathSegments	segments;
	int					renderTimes;
	float				azimuth;
	float				elevation;
	float				nextPos[3];
	int					earlySegment;
	SceneNode*			teamSceneNode;
	SceneNode*			rogueSceneNode;
	SceneNodeTransform*	transformSceneNode;

	bool				needUpdate;
	PlayerId			lastTarget;
};

class ShockWaveStrategy : public ShotStrategy {
public:
	ShockWaveStrategy(ShotPath*);
	~ShockWaveStrategy();

	void				update(float dt);
	float				checkHit(const BaseLocalPlayer*, float[3]) const;
	bool				isStoppedByHit() const;
	void				addShot(SceneNodeGroup*, bool colorblind);
	void				radarRender() const;

private:
	SceneNode*			teamSceneNode;
	SceneNode*			rogueSceneNode;
	SceneNodeTransform*	transformSceneNode;
	SceneNodeParameters*	parametersSceneNode;
	float				radius;
	float				radius2;
	TimeKeeper			startTime;
};

//
// ShotStrategy
//

inline const ShotPath&	ShotStrategy::getPath() const
{
	return *path;
}

#endif // BZF_SHOT_STRATEGY_H
// ex: shiftwidth=4 tabstop=4
