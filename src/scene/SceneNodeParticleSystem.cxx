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
#include "SceneReader.h"
#include "SceneNodeFieldReader.h"
#include "math3D.h"
#include <iostream>

//
// Particle
//
Particle::Particle() :
			ttl(10),
			position(0, 0, 0),
			direction(0, 0, 0),
			index("index", 0, 0, 1)
{
	// set sizes - we're using quads
	color  = new SceneNodeVFFloat("color", 0, 0, 4);
	color->resize(16);
	normal = new SceneNodeVFFloat("normal", 0, 0, 3);
	normal->resize(3);
	vertex = new SceneNodeVFFloat("vertex", 0, 0, 3);
	vertex->resize(12);
	index.resize(4);

	// set normal
	normal->set(0, 0.0f);
	normal->set(1, 1.0f);
	normal->set(2, 0.0f);

	index.set((unsigned int) 0,  0);
	index.set(1,  1);
	index.set(2,  2);
	index.set(3,  3);
}

Particle::~Particle()
{
	delete color;
	delete normal;
	delete vertex;
}

void Particle::setColor(Real cr, Real cg, Real cb, Real ca)
{
	for (int i = 0; i < 4; i++) {
		color->set((i * 4) + 0, cr);
		color->set((i * 4) + 1, cg);
		color->set((i * 4) + 2, cb);
		color->set((i * 4) + 3, ca);
	}
}

void Particle::getColor(Real& cr, Real& cg, Real& cb, Real& ca)
{
	cr = color->get(0);
	cg = color->get(1);
	cb = color->get(2);
	ca = color->get(3);
}

void Particle::setSize(Real _w, Real _h)
{
	w = _w;
	h = _h;
}

void Particle::calculate()
{
	// upper left
	vertex->set(0,  position[0] - w);
	vertex->set(1,  position[1]);
	vertex->set(2,  position[2] + h);
	// lower left
	vertex->set(3,  position[0] - w);
	vertex->set(4,  position[1]);
	vertex->set(5,  position[2] - h);
	// upper right
	vertex->set(6,  position[0] + w);
	vertex->set(7,  position[1]);
	vertex->set(8,  position[2] + h);
	// lower right
	vertex->set(9,  position[0] + w);
	vertex->set(10, position[1]);
	vertex->set(11, position[2] - h);
}

bool Particle::visit(SceneVisitor* visitor)
{
	return visitor->visit(this);
}

Particle* Particle::clone()
{
	Particle* newp = new Particle();
	newp->setSize(w, h);
	newp->color->set(*color);
	newp->ttl = ttl;
	newp->position = position;
	newp->direction = direction;
	newp->calculate();
	return newp;
}

//
// ParticleAffector
//
ParticleAffector::ParticleAffector()
{
}

ParticleAffector::~ParticleAffector()
{
}

void		ParticleAffector::parse(XMLTree::iterator)
{
}

//
// ParticleAffectorLinearForce
//
ParticleAffectorLinearForce::ParticleAffectorLinearForce() :
								force("force", 0, 0, 3)
{
}

ParticleAffectorLinearForce::~ParticleAffectorLinearForce()
{
}

void	ParticleAffectorLinearForce::affectParticles(SceneNodeParticleSystem* system, Real time)
{
	SceneNodeParticleSystem::ParticleList::iterator it;
	Vec3 forceVector, scaledVector;

	forceVector[0] = force.get(0);
	forceVector[1] = force.get(1);
	forceVector[2] = force.get(2);
	scaledVector = forceVector * time;
	
	for (it = system->particles.begin(); it != system->particles.end(); it++) {
		Particle* p = static_cast<Particle*> (*it);
		if (application == Add) {
			p->direction += scaledVector;
		}
		else {
			p->direction = (p->direction + forceVector) * 0.5;
		}
	}
}

void	ParticleAffectorLinearForce::parse(XMLTree::iterator xml)
{
	static const XMLParseEnumList<Application> s_enumForceApplication[] = {
		{"average",	Average},
		{"add",		Add}
	};

	if (xml->value == "application") {
		xml->getAttribute("method", xmlParseEnum(s_enumForceApplication,
										xmlSetVar(application)));
	}
	else if (xml->value == "force") {
		SceneNodeVectorReader<float> reader(&force);
		reader.parse(xml);
	}
}

ParticleAffector* ParticleAffectorLinearForce::clone()
{
	ParticleAffector *newp = new ParticleAffectorLinearForce;
	((ParticleAffectorLinearForce*) newp)->application = application;
	((ParticleAffectorLinearForce*) newp)->force.set(force);
	return newp;
}

//
// ParticleAffectorFactory
//
ParticleAffector*	ParticleAffectorFactory::create(const std::string& typespecifier) const
{
	if (typespecifier == "linearforce") {
		return new ParticleAffectorLinearForce();
	}
	return NULL;
}

//
// ParticleEmitter
//
ParticleEmitter::ParticleEmitter() :
							position("position", 0, 0, 3),
							rate("rate"),
							direction("direction", 0, 0, 3),
							up("up", 0, 0, 3),
							angle("angle"),
							minSpeed("minvelocity"),
							maxSpeed("maxvelocity"),
							minTTL("mintimetolive"),
							maxTTL("maxtimetolive"),
							colorStart("mincolor", 0, 0, 4),
							colorEnd("maxcolor", 0, 0, 4)
{
}

ParticleEmitter::~ParticleEmitter()
{
}

void			ParticleEmitter::createPosition(Vec3& pos)
{
	if (position.getNum() == 0) {
		pos = Vec3(0, 0, 0);
	}
	else {
		pos[0] = position.get()[0];
		pos[1] = position.get()[1];
		pos[2] = position.get()[2];
	}
}

void			ParticleEmitter::createDirection(Vec3& dir)
{
	if (angle.get() != 0) {
		Real newangle = bzfrand() * angle.get();
		Vec3 vup; vup[0] = up.get()[0]; vup[1] = up.get()[1]; vup[2] = up.get()[2];
		//
		// FIXME
	}
	else {
		dir[0] = direction.get()[0];
		dir[1] = direction.get()[1];
		dir[2] = direction.get()[2];
	}
}

void			ParticleEmitter::createVelocity(Vec3& vel)
{
	if (minSpeed.get() != maxSpeed.get()) {
		vel *= (minSpeed.get() + (bzfrand() * (maxSpeed.get() - minSpeed.get())));
	}
	else {
		vel *= minSpeed.get();
	}
}

void			ParticleEmitter::createTTL(Real& ttl)
{
	if (minTTL.get() != maxTTL.get()) {
		ttl = (minTTL.get() + (bzfrand() * (maxTTL.get() - minTTL.get())));
	}
	else {
		ttl = minTTL.get();
	}
}

void			ParticleEmitter::createColor(SceneNodeVFFloat& col)
{
	if (!((colorStart.get(0) == colorEnd.get(0)) &&
		  (colorStart.get(1) == colorEnd.get(1)) &&
		  (colorStart.get(2) == colorEnd.get(2)) &&
		  (colorStart.get(3) == colorEnd.get(3)))) {
		col.set(colorStart.get(0) + (bzfrand() * (colorEnd.get(0) - colorStart.get(0))), 0);
		col.set(colorStart.get(1) + (bzfrand() * (colorEnd.get(1) - colorStart.get(1))), 1);
		col.set(colorStart.get(2) + (bzfrand() * (colorEnd.get(2) - colorStart.get(2))), 2);
		col.set(colorStart.get(3) + (bzfrand() * (colorEnd.get(3) - colorStart.get(3))), 3);
	}
	else {
		col = colorStart;
	}
}

unsigned short	ParticleEmitter::createConstantEmissionCount(Real time)
{
	static Real remainder = 0;
	unsigned short intRequest;

	remainder += rate.get() * time;
	intRequest = (unsigned short) remainder;
	remainder -= intRequest;
	return intRequest;
}

void			ParticleEmitter::parse(XMLTree::iterator xml)
{
	if (xml->value == "position") {
		SceneNodeVectorReader<float> reader(&position);
		reader.parse(xml);
	}
	else if (xml->value == "rate") {
		SceneNodeScalarReader<float> reader(&rate);
		reader.parse(xml);
	}
	else if (xml->value == "direction") {
		SceneNodeVectorReader<float> reader(&direction);
		reader.parse(xml);
	}
	else if (xml->value == "up") {
		SceneNodeVectorReader<float> reader(&up);
		reader.parse(xml);
	}
	else if (xml->value == "angle") {
		SceneNodeScalarReader<float> reader(&angle);
		reader.parse(xml);
	}
	else if (xml->value == "velocity") {
		SceneNodeScalarReader<float> reader(&minSpeed);
		reader.parse(xml);
		maxSpeed = minSpeed;
	}
	else if (xml->value == "minvelocity") {
		SceneNodeScalarReader<float> reader(&minSpeed);
		reader.parse(xml);
	}
	else if (xml->value == "maxvelocity") {
		SceneNodeScalarReader<float> reader(&maxSpeed);
		reader.parse(xml);
	}
	else if (xml->value == "timetolive") {
		SceneNodeScalarReader<float> reader(&minTTL);
		reader.parse(xml);
		maxTTL = minTTL;
	}
	else if (xml->value == "mintimetolive") {
		SceneNodeScalarReader<float> reader(&minTTL);
		reader.parse(xml);
	}
	else if (xml->value == "maxtimetolive") {
		SceneNodeScalarReader<float> reader(&maxTTL);
		reader.parse(xml);
	}
	else if (xml->value == "color") {
		SceneNodeVectorReader<float> reader(&colorStart);
		reader.parse(xml);
		colorEnd = colorStart;
	}
	else if (xml->value == "mincolor") {
		SceneNodeVectorReader<float> reader(&colorStart);
		reader.parse(xml);
	}
	else if (xml->value == "maxcolor") {
		SceneNodeVectorReader<float> reader(&colorEnd);
		reader.parse(xml);
	}
}

//
// ParticleEmitterPoint
//

ParticleEmitterPoint::ParticleEmitterPoint()
{
}

ParticleEmitterPoint::~ParticleEmitterPoint()
{
}

Particle*		ParticleEmitterPoint::createParticle()
{
	Particle*	p = new Particle;
	createDirection(p->direction);
	createPosition(p->position);
	createColor(*(p->color));
	createTTL(p->ttl);
	return p;
}

void			ParticleEmitterPoint::parse(XMLTree::iterator xml)
{
	ParticleEmitter::parse(xml);
}

unsigned int	ParticleEmitterPoint::getCount(Real time)
{
	return createConstantEmissionCount(time);
}

ParticleEmitter*	ParticleEmitterPoint::clone()
{
	ParticleEmitter* newp = new ParticleEmitterPoint;
	newp->position.set(position);
	newp->rate.set(rate.get());
	newp->direction.set(direction);
	newp->up.set(up);
	newp->angle.set(angle.get());
	newp->minSpeed.set(minSpeed.get());
	newp->maxSpeed.set(maxSpeed.get());
	newp->minTTL.set(minTTL.get());
	newp->maxTTL.set(maxTTL.get());
	newp->colorStart.set(colorStart);
	newp->colorEnd.set(colorEnd);
	return newp;
}

//
// ParticleEmitterBox
//
ParticleEmitterBox::ParticleEmitterBox() :
								size("size", 0, 0, 3)
{
}

ParticleEmitterBox::~ParticleEmitterBox()
{
}

Particle*		ParticleEmitterBox::createParticle()
{
	Particle*	p = new Particle;
	createDirection(p->direction);
	if (position.getNum() == 0) {
		if (size.getNum() == 0) {
			p->position = Vec3(0, 0, 0);
		}
		else {
			p->position[0] = ((rand() % 2) ? bzfrand() * size.get(0) : -bzfrand() * size.get(0));
			p->position[1] = ((rand() % 2) ? bzfrand() * size.get(1) : -bzfrand() * size.get(1));
			p->position[2] = ((rand() % 2) ? bzfrand() * size.get(2) : -bzfrand() * size.get(2));
		}
	}
	else {
		if (size.getNum() == 0) {
			p->position[0] = position.get()[0];
			p->position[1] = position.get()[1];
			p->position[2] = position.get()[2];
		}
		else {
			Real xo, yo, zo;
			xo = ((rand() % 2) ? bzfrand() * size.get(0) : -bzfrand() * size.get(0));
			yo = ((rand() % 2) ? bzfrand() * size.get(1) : -bzfrand() * size.get(1));
			zo = ((rand() % 2) ? bzfrand() * size.get(2) : -bzfrand() * size.get(2));
			p->position[0] = position.get()[0] + xo;
			p->position[1] = position.get()[1] + yo;
			p->position[2] = position.get()[2] + zo;
		}
	}
	// position
	createColor(*(p->color));
	createTTL(p->ttl);
	return p;
}

void			ParticleEmitterBox::parse(XMLTree::iterator xml)
{
	ParticleEmitter::parse(xml);
	if (xml->value == "size") {
		SceneNodeVectorReader<float> reader(&size);
		reader.parse(xml);
	}
}

ParticleEmitter*	ParticleEmitterBox::clone()
{
	ParticleEmitter* newp = new ParticleEmitterBox;
	newp->position.set(position);
	newp->rate.set(rate.get());
	newp->direction.set(direction);
	newp->up.set(up);
	newp->angle.set(angle.get());
	newp->minSpeed.set(minSpeed.get());
	newp->maxSpeed.set(maxSpeed.get());
	newp->minTTL.set(minTTL.get());
	newp->maxTTL.set(maxTTL.get());
	newp->colorStart.set(colorStart);
	newp->colorEnd.set(colorEnd);
	((ParticleEmitterBox*) newp)->size.set(size);
	return newp;
}

unsigned int	ParticleEmitterBox::getCount(Real time)
{
	return createConstantEmissionCount(time);
}

//
// ParticleEmitterFactory
//

ParticleEmitter*	ParticleEmitterFactory::create(const std::string& typespecifier) const
{
	if (typespecifier == "point") {
		return new ParticleEmitterPoint();
	}
	else if (typespecifier == "box") {
		return new ParticleEmitterBox();
	}
	return NULL;
}

//
// SceneNodeParticleSystem
//

SceneNodeParticleSystem::SceneNodeParticleSystem() :
									quota("quota"),
									width("width"),
									height("height")
{
	axis.resize(3);
	axis.set(0, 0.0f); axis.set(1, 0.0f); axis.set(2, 0.0f);
	turn.set(true);
}

SceneNodeParticleSystem::~SceneNodeParticleSystem()
{
}

bool SceneNodeParticleSystem::visit(SceneVisitor* visitor)
{
	return visitor->visit(this);
}

bool SceneNodeParticleSystem::descend(SceneVisitor* visitor, const SceneVisitorParams&)
{
	for (ParticleList::const_iterator index = particles.begin();
							index != particles.end(); ++index) {
		if (!(*index)->visit(visitor))
			return false;
	}
	return true;
}

void SceneNodeParticleSystem::update(Real newTime)
{
	Real timeDifference = newTime - currentTime;
	currentTime = newTime;
	expire(timeDifference);
	triggerEmitters(timeDifference);
	applyMotion(timeDifference);
	triggerAffectors(timeDifference);

	for (ParticleList::iterator it = particles.begin(); it != particles.end(); ++it) {
		(*it)->calculate();
	}
}

void SceneNodeParticleSystem::getBoundingBox(BoundingBox* box)
{
	assert(box != NULL);

	if(boundingBoxDirty) {
		boundingBoxDirty = false;

		if(particles.size() == 0) {
			boundingBox.set(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		}
		else {
			ParticleList::iterator i = particles.begin();
			Particle* p = static_cast<Particle*> (*i);

			// find extents over all particles
			float xMin = p->position[0];
			float yMin = p->position[1];
			float zMin = p->position[2];
			float xMax = xMin;
			float yMax = yMin;
			float zMax = zMin;
			for(; i!= particles.end(); ++i) {
				p = static_cast<Particle*> (*i);
				if(xMin > p->position[0])
					xMin = p->position[0];
				if(xMax < p->position[0])
					xMax = p->position[0];
				if(yMin > p->position[1])
					yMin = p->position[1];
				if(yMax < p->position[1])
					yMax = p->position[1];
				if(zMin > p->position[2])
					zMin = p->position[2];
				if(zMax < p->position[2])
					zMax = p->position[2];
			}
			boundingBox.set(xMin, yMin, zMin, xMax, yMax, zMax);
		}
	}
	*box = boundingBox;
}

SceneNodeParticleSystem SceneNodeParticleSystem::operator = (const SceneNodeParticleSystem &p)
{
	for (unsigned int i = 0; i < p.particles.size(); i++) {
		particles.push_back(p.particles[i]->clone());
	}
	for (unsigned int i = 0; i < p.affectors.size(); i++) {
		affectors.push_back(p.affectors[i]->clone());
	}
	for (unsigned int i = 0; i < p.emitters.size(); i++) {
		emitters.push_back(p.emitters[i]->clone());
	}
	quota = p.quota;
	width = p.width;
	height = p.height;
	boundingBoxDirty = p.boundingBoxDirty;
	boundingBox = p.boundingBox;
	currentTime = p.currentTime;
	return *this;
}

void					SceneNodeParticleSystem::expire(Real time)
{
	ParticleList::iterator i;
	Particle* p;

	for (i = particles.begin(); i != particles.end(); i++) {
		p = static_cast<Particle*>(*i);
		if (p != NULL) {
			if (p->ttl < time) {
				delete p;
				particles.erase(i);
			}
			else {
				p->ttl -= time;
			}
		}
	}
}

void					SceneNodeParticleSystem::applyMotion(Real time)
{
	ParticleList::iterator i;
	Particle* p;

	for (i = particles.begin(); i != particles.end(); i++) {
		p = static_cast<Particle*>(*i);
		p->position += (p->direction * time);
	}
}

void					SceneNodeParticleSystem::triggerAffectors(Real time)
{
	AffectorList::iterator i;

	for (i = affectors.begin(); i != affectors.end(); i++) {
		(*i)->affectParticles(this, time);
	}
}

void					SceneNodeParticleSystem::triggerEmitters(Real time)
{
	EmitterList::iterator i;
	std::vector<unsigned int> requestedParticles;
	unsigned int totalReq, emissionAllowed;

	emissionAllowed = quota.get() - particles.size();
	totalReq = 0;
	int pos;

	if (requestedParticles.size() != emitters.size())
		requestedParticles.resize(emitters.size());

	for (i = emitters.begin(), pos = 0; i != emitters.end(); ++i, ++pos) {
		requestedParticles[pos] = (*i)->getCount(time);
		totalReq += requestedParticles[pos];
	}


	if (totalReq > emissionAllowed) {
		Real ratio = ((Real) emissionAllowed) / ((Real) totalReq);
		for(unsigned int j = 0; j < requestedParticles.size(); j++) {
			requestedParticles[j] = (unsigned int) (((Real) requestedParticles[j]) * ratio);
		}
	}

	for (i = emitters.begin(), pos = 0; i != emitters.end(); ++i, ++pos) {
		for (unsigned int j = 0; j < requestedParticles[pos]; j++) {
			Particle* p = (*i)->createParticle();
			p->setSize(width.get(), height.get());
			particles.push_back(p);
			// translate particle into world space?
		}
	}
}

// ex: shiftwidth=4 tabstop=4
