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

#include "SceneNodeGeometry.h"
#include "SceneVisitor.h"
#include "SceneVisitorParams.h"
#include "math3D.h"
#include <iostream>

//
// Particle
//
void Particle::setParent(SceneNodeParticleSystem *parent_)
{
	parent = parent_;
}

void Particle::create(SceneNodeParticleSystem *parent_, float /*timeCounter*/)
{
	float randomYaw;
	float randomPitch;
	float newSpeed;
	float temp;

	parent = parent_;

	age = 0.0;
	dyingAge = (parent->life) + (parent->lifeCounter);

	color[0] = parent->startColor[0];
	color[1] = parent->startColor[1];
	color[2] = parent->startColor[2];
	color[3] = parent->startColor[3];
	colorCounter[0] = (parent->endColor[0]-color[0]) / dyingAge;
	colorCounter[1] = (parent->endColor[1]-color[1]) / dyingAge;
	colorCounter[2] = (parent->endColor[2]-color[2]) / dyingAge;
	colorCounter[3] = (parent->endColor[3]-color[3]) / dyingAge;

	size = parent->startSize + parent->sizeCounter;
	sizeCounter = (parent->endSize-size) / dyingAge;

	location[0] = parent->location[0];
	location[1] = parent->location[1];
	location[2] = parent->location[2];

	temp = (bzfrand() * ((float) (parent->spreadMax - parent->spreadMin))) + parent->spreadMin;
	location[0] += temp / parent->spreadFactor;
	temp = (bzfrand() * ((float) (parent->spreadMax - parent->spreadMin))) + parent->spreadMin;
	location[1] += temp / parent->spreadFactor;
	temp = (bzfrand() * ((float) (parent->spreadMax - parent->spreadMin))) + parent->spreadMin;
	location[2] += temp / parent->spreadFactor;

	randomPitch = bzfrand() * M_PI * 2.0;
	randomYaw = (M_PI / 180.0) * bzfrand() * parent->fieldAngle;

	velocity[0] = (cosf(randomPitch) * (parent->velocity[0]));
	velocity[1] = (sinf(randomPitch) * cosf(randomYaw) * (parent->velocity[1]));
	velocity[2] = (sinf(randomPitch) * sinf(randomYaw) * (parent->velocity[2]));

	newSpeed = parent->speed + parent->speedCounter;
	velocity[0] *= newSpeed;
	velocity[1] *= newSpeed;
	velocity[2] *= newSpeed;
}

bool Particle::update(float timeCounter)
{
	static float attractLocation[3];
	static float attractNormal[3];

	age += timeCounter;
	if(age >= dyingAge) {
		age = -1.0;
		return false;
	}

	previousLocation[0] = location[0];
	previousLocation[1] = location[1];
	previousLocation[2] = location[2];

	location[0] += velocity[0] * timeCounter;
	location[1] += velocity[1] * timeCounter;
	location[2] += velocity[2] * timeCounter;

	velocity[0] += (parent->gravity[0] * timeCounter);
	velocity[1] += (parent->gravity[1] * timeCounter);
	velocity[2] += (parent->gravity[2] * timeCounter);

	if(parent->isAttracting()) {
		attractLocation[0] = parent->location[0];
		attractLocation[1] = parent->location[1];
		attractLocation[2] = parent->location[2];

		attractNormal[0] = attractLocation[0] - location[0];
		attractNormal[1] = attractLocation[1] - location[1];
		attractNormal[2] = attractLocation[2] - location[2];

		velocity[0] += attractNormal[0] * 5.0 * timeCounter;
		velocity[1] += attractNormal[1] * 5.0 * timeCounter;
		velocity[2] += attractNormal[2] * 5.0 * timeCounter;
	}

	color[0] += colorCounter[0] * timeCounter;
	color[1] += colorCounter[1] * timeCounter;
	color[2] += colorCounter[2] * timeCounter;
	color[3] += colorCounter[3] * timeCounter;

	size += sizeCounter * timeCounter;

	return true;
}

Particle Particle::operator = (const Particle &p)
{
	previousLocation[0] = p.previousLocation[0];
	previousLocation[1] = p.previousLocation[1];
	previousLocation[2] = p.previousLocation[2];
	location[0] = p.location[0];
	location[1] = p.location[1];
	location[2] = p.location[2];
	velocity[0] = p.velocity[0];
	velocity[1] = p.velocity[1];
	velocity[2] = p.velocity[2];
	color[0] = p.color[0];
	color[1] = p.color[1];
	color[2] = p.color[2];
	color[3] = p.color[3];
	colorCounter[0] = p.colorCounter[0];
	colorCounter[1] = p.colorCounter[1];
	colorCounter[2] = p.colorCounter[2];
	colorCounter[3] = p.colorCounter[3];
	size = p.size;
	sizeCounter = p.sizeCounter;
	age = p.age;
	dyingAge = p.dyingAge;
	parent = p.parent;
	return *this;
}

//
// SceneNodeParticleSystem
//

SceneNodeParticleSystem::SceneNodeParticleSystem():
			index("snpsindex", 0, 0, 1),
			colors("snpscolors", 0, 0, 4),
			verteces("snpsverteces", 0, 0, 3),
			texcoords("snpstexcoords", 0, 0, 2),
			locationV("location", 0, 0, 3),
			velocityV("velocity", 0, 0, 3),
			startColorV("startcolor", 0, 0, 4),
			endColorV("endcolor", 0, 0, 4),
			startSizeV("startsize", 0),
			endSizeV("endsize", 0),
			gravityV("gravity", 0, 0, 3),
			speedV("speed", 0.0),
			lifeV("life", 0.0),
			fieldAngleV("fieldangle", 90.0),
			attractionPercentV("attraction", 0.0),
			creationSpeedV("creationspeed", 0),
			burstSizeV("burstsize", 0),
			spreadMinV("spreadmin", 0),
			spreadMaxV("spreadmax", 0),
			spreadFactorV("spreadfactor", 0)
{
	attracting = false;
	stopped = false;
	particlesPerSecond = 0;
	particlesAlive = 0;
	startSize = 0;
	sizeCounter = 0;
	endSize = 0;
	speed = 0;
	speedCounter = 0;
	life = 0;
	lifeCounter = 0;
	fieldAngle = 0;
	age = 0;
	lastUpdate = 0;
	emissionResidue = 0;
	spreadMin = 0;
	spreadMax = 0;
	spreadFactor = 1.0;
	attractionPercent = 0;
	burstCreated = false;
	action = CreateConstant;
}

SceneNodeParticleSystem::~SceneNodeParticleSystem()
{
}

bool SceneNodeParticleSystem::visit(SceneVisitor* visitor)
{
	return visitor->visit(this);
}

bool SceneNodeParticleSystem::update(float newTime, const Matrix& matrix)
{
	if(stopped)
		return false;

	float time = newTime - currentTime;
	currentTime = newTime;

	// recompute counter values
	sizeCounter = (endSize - startSize) / life;
	speedCounter = speed / life;
	lifeCounter = 0;

	if(action == CreateConstant) {
		return update(time, UpdateAndCreate, 0, matrix);
	}
	else if(action == CreateBurst) {
		if(!burstCreated) {
			// keep in mind that this is not really
			// particles per second, but really the
			// total particles to create. I'm just
			// too lazy to use a different variable
			update(time, OnlyCreate, particlesPerSecond, matrix);
			burstCreated = true;
		}
		return update(time, OnlyUpdate, 0, matrix);
	}
	else {
		return true;
	}
}

bool SceneNodeParticleSystem::update(float time, ParticleSystemUpdate flag, float numToCreate, const Matrix& matrix)
{
	// valid values for flag are OnlyUpdate, UpdateAndCreate, and OnlyCreate
	unsigned int particlesCreated;
	float timeCounter;
	float particlesNeeded = numToCreate;
	bool itsAlive;
	bool killed = true;
	float size;
	int numkilled = 0;

	timeCounter = time;

	if(flag == OnlyUpdate || flag == UpdateAndCreate) {
		for(unsigned int i = 0; i < particles.size(); i++) {
			itsAlive = particles[i].update(timeCounter);
			if(killed) {
				if(itsAlive) {
					killed = false;
				}
				else {
					numkilled++;
					particles.pop_front();
					i--;
				}
			}
		}
	}
	if(flag == OnlyCreate || flag == UpdateAndCreate) {
		particlesNeeded += particlesPerSecond * timeCounter + emissionResidue;
		particlesCreated = (unsigned int) particlesNeeded;
		
		if(!stopped) {
			emissionResidue = particlesNeeded - particlesCreated;
		}
		else {
			emissionResidue = particlesNeeded;
			particlesCreated = 0;
		}
		if(particlesCreated < 1) {
			previousLocation[0] = location[0];
			previousLocation[1] = location[1];
			previousLocation[2] = location[2];
		}
		for(unsigned int i = 0; i < particlesCreated; i++) {
			Particle temp;
			temp.create(this, timeCounter);
			particles.push_back(temp);
		}
	}
	verteces.clear();
	colors.clear();
	texcoords.clear();
	index.clear();

	float m[16];
	matrix.get(m);
	for(unsigned int i = 0; i < particles.size(); i++) {
		const float* p = particles[i].location;
		size = particles[i].size / 4;
		verteces.push(p[0] + size * ( m[0] - m[8]));
		verteces.push(p[1] + size * ( m[1] - m[9]));
		verteces.push(p[2] + size * ( m[2] - m[10]));
		verteces.push(p[0] + size * ( m[0] + m[8]));
		verteces.push(p[1] + size * ( m[1] + m[9]));
		verteces.push(p[2] + size * ( m[2] + m[10]));
		verteces.push(p[0] + size * (-m[0] + m[8]));
		verteces.push(p[1] + size * (-m[1] + m[9]));
		verteces.push(p[2] + size * (-m[2] + m[10]));
		verteces.push(p[0] + size * (-m[0] - m[8]));
		verteces.push(p[1] + size * (-m[1] - m[9]));
		verteces.push(p[2] + size * (-m[2] - m[10]));

		texcoords.push(1.0f, 0.0f);
		texcoords.push(1.0f, 1.0f);
		texcoords.push(0.0f, 1.0f);
		texcoords.push(0.0f, 0.0f);
		for(int j = 0; j < 4; j++) {
			colors.push(particles[i].color[0]);
			colors.push(particles[i].color[1]);
			colors.push(particles[i].color[2]);
			colors.push(particles[i].color[3]);
			index.push((i * 4) + j);
		}
	}

	return true;
}

unsigned int SceneNodeParticleSystem::activeParticles()
{
	return particles.size();
}

bool SceneNodeParticleSystem::isAttracting()
{
	return attracting;
}

bool SceneNodeParticleSystem::isStopped()
{
	return stopped;
}

void SceneNodeParticleSystem::getBoundingBox(BoundingBox* box)
{
	assert(box != NULL);

	if(boundingBoxDirty) {
		boundingBoxDirty = false;

		// find first particle with some vertices
		std::deque<Particle>::iterator index;

		if(particles.size() == 0) {
			boundingBox.set(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		}
		else {
			// find extents over all particles
			float xMin = index->location[0];
			float yMin = index->location[1];
			float zMin = index->location[2];
			float xMax = xMin;
			float yMax = yMin;
			float zMax = zMin;
			for(index = particles.begin(); index != particles.end(); ++index) {
				const float *v = index->location;
				if(xMin > v[0])
					xMin = v[0];
				if(xMax < v[0])
					xMax = v[0];
				if(yMin > v[1])
					yMin = v[1];
				if(yMax < v[1])
					yMax = v[1];
				if(zMin > v[2])
					zMin = v[2];
				if(zMax < v[2])
					zMax = v[2];
			}
			boundingBox.set(xMin, yMin, zMin, xMax, yMax, zMax);
		}
	}
	*box = boundingBox;
}

SceneNodeParticleSystem SceneNodeParticleSystem::operator = (const SceneNodeParticleSystem &p)
{
	particles = p.particles;
	previousLocation[0] = p.previousLocation[0];
	previousLocation[1] = p.previousLocation[1];
	previousLocation[2] = p.previousLocation[2];
	location[0] = p.location[0];
	location[1] = p.location[1];
	location[2] = p.location[2];
	velocity[0] = p.velocity[0];
	velocity[1] = p.velocity[1];
	velocity[2] = p.velocity[2];
	startSize = p.startSize;
	sizeCounter = p.sizeCounter;
	endSize = p.endSize;
	startColor[0] = p.startColor[0];
	startColor[1] = p.startColor[1];
	startColor[2] = p.startColor[2];
	startColor[3] = p.startColor[3];
	colorCounter[0] = p.colorCounter[0];
	colorCounter[1] = p.colorCounter[1];
	colorCounter[2] = p.colorCounter[2];
	colorCounter[3] = p.colorCounter[3];
	endColor[0] = p.endColor[0];
	endColor[1] = p.endColor[1];
	endColor[2] = p.endColor[2];
	endColor[3] = p.endColor[3];
	speed = p.speed;
	speedCounter = p.speedCounter;
	life = p.life;
	lifeCounter = p.lifeCounter;
	fieldAngle = p.fieldAngle;
	spreadMin = p.spreadMin;
	spreadMax = p.spreadMax;
	spreadFactor = p.spreadFactor;
	gravity[0] = p.gravity[0];
	gravity[1] = p.gravity[1];
	gravity[2] = p.gravity[2];
	attractionPercent = p.attractionPercent;
	attracting = p.attracting;
	stopped = p.stopped;
	particlesPerSecond = p.particlesPerSecond;
	type = p.type;
	index = p.index;
	colors = p.colors;
	texcoords = p.texcoords;
	age = p.age;
	lastUpdate = p.lastUpdate;
	emissionResidue = p.emissionResidue;
	currentTime = p.currentTime;
	boundingBoxDirty = p.boundingBoxDirty;
	boundingBox = p.boundingBox;
	return *this;
}
// ex: shiftwidth=4 tabstop=4
