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

#ifndef BZF_SCENE_NODE_PARTICLE_SYSTEM_H
#define BZF_SCENE_NODE_PARTICLE_SYSTEM_H

#include "common.h"
#include <assert.h>
#include <deque>
#include "math3D.h"
#include "SceneNode.h"

class SceneVisitorParams;
class SceneNodeParticleSystem;			// predeclaration for Particle class

class Particle {
public:
	float			previousLocation[3];
	float			location[3];
	float			velocity[3];

	float			color[4];
	float			colorCounter[4];
	float			size;
	float			sizeCounter;
	float			age;
	float			dyingAge;

	void			setParent(SceneNodeParticleSystem *parent_);
	void			create(SceneNodeParticleSystem *parent_, float timeCounter);
	bool			update(float timeCounter);

	Particle		operator = (const Particle&);
private:
	SceneNodeParticleSystem	*parent;
};

enum ParticleSystemUpdate { UpdateAndCreate, OnlyUpdate, OnlyCreate };
enum ParticleSystemType { CreateConstant, CreateBurst };

class SceneNodeParticleSystem : public SceneNode {
public:
				SceneNodeParticleSystem();

	// SceneNode overrides
	virtual bool		visit(SceneVisitor*);

	std::deque<Particle>	particles;

	float			previousLocation[3];
	float			location[3];
	float			velocity[3];

	float			startSize;
	float			sizeCounter;
	float			endSize;

	float			startColor[4];
	float			colorCounter[4];
	float			endColor[4];

	float			speed;
	float			speedCounter;

	float			life;
	float			lifeCounter;

	float			fieldAngle;

	int			spreadMin;
	int			spreadMax;
	float			spreadFactor;

	float			gravity[3];
	float			attractionPercent;

	bool			attracting;
	bool			stopped;
	ParticleSystemType	action;

	unsigned int		particlesPerSecond;

	// fields
	unsigned int		type;
	SceneNodeVFUInt		index;

	// visitor stuff
	SceneNodeVFFloat	colors;
	SceneNodeVFFloat	verteces;
	SceneNodeVFFloat	texcoords;

	// reader temporaries
	SceneNodeVFFloat	locationV;
	SceneNodeVFFloat	velocityV;
	SceneNodeVFFloat	startColorV;
	SceneNodeVFFloat	endColorV;
	SceneNodeSFFloat	startSizeV;
	SceneNodeSFFloat	endSizeV;
	SceneNodeVFFloat	gravityV;
	SceneNodeSFFloat	speedV;
	SceneNodeSFFloat	lifeV;
	SceneNodeSFFloat	fieldAngleV;
	SceneNodeSFFloat	attractionPercentV;
	SceneNodeSFUInt		creationSpeedV;
	SceneNodeSFUInt		burstSizeV;
	SceneNodeSFUInt		spreadMinV;
	SceneNodeSFUInt		spreadMaxV;
	SceneNodeSFFloat	spreadFactorV;

//	Property		property;

	Matrix			transformation;

	// particle functions
	bool			update(float newTime, const Matrix&);
	bool			update(float time, ParticleSystemUpdate flag, float numToCreate, const Matrix&);
	unsigned int		activeParticles();
	bool			isAttracting();
	bool			isStopped();

	// get the axis-aligned bounding box around all particles.  if there
	// are no particles then the box is point at the origin.
	void			getBoundingBox(BoundingBox* box);

	SceneNodeParticleSystem operator = (const SceneNodeParticleSystem &);

protected:
	virtual			~SceneNodeParticleSystem();

	unsigned int		particlesAlive;

	float			age;
	float			lastUpdate;
	float			emissionResidue;

	float			currentTime;
private:
	bool			boundingBoxDirty;
	BoundingBox		boundingBox;

	// keep track of whether or not we've created
	// the particles when in burst mode
	bool			burstCreated;
};

#endif
