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
#include "SceneNodeBillboard.h"
#include "XMLTree.h"

class SceneVisitorParams;
class SceneNodeParticleSystem;			// predeclaration for Particle class

class Particle : public SceneNode {
public:
	Particle();
	virtual ~Particle();
	void				setColor(Real cr, Real cg, Real cb, Real ca);
	void				getColor(Real& cr, Real& cg, Real& cb, Real& ca);
	void				setSize(Real _w, Real _h);
	void				calculate();

	// SceneNode overrides
	virtual bool		visit(SceneVisitor*);

	virtual Particle*	clone();

	Real				ttl;
	Vec3				position;
	Vec3				direction;
	Real				w, h;

	// visitor stuff
	SceneNodeVFFloat*	color;
	SceneNodeVFFloat*	normal;
	SceneNodeVFFloat*	vertex;
	SceneNodeVFUInt		index;

protected:
};

class ParticleAffector {
public:
						ParticleAffector();
	virtual				~ParticleAffector();

	virtual void		affectParticles(SceneNodeParticleSystem* system, Real time) = 0;
	virtual void		parse(XMLTree::iterator);

	virtual ParticleAffector*	clone() = 0;

protected:
};

class ParticleAffectorLinearForce : public ParticleAffector {
public:
						ParticleAffectorLinearForce();
	virtual				~ParticleAffectorLinearForce();

	virtual void		affectParticles(SceneNodeParticleSystem* system, Real time);
	virtual void		parse(XMLTree::iterator);

	virtual ParticleAffector*	clone();

protected:
	enum Application {
		Average,
		Add
	};

	Application			application;
	SceneNodeVFFloat	force;
};

class ParticleAffectorColorFader : public ParticleAffector {
public:
						ParticleAffectorColorFader();
	virtual				~ParticleAffectorColorFader();

	virtual void		affectParticles(SceneNodeParticleSystem* system, Real time);
	virtual void		parse(XMLTree::iterator);

	virtual ParticleAffector*	clone();

protected:
	SceneNodeVFFloat delta;
};

class ParticleAffectorFactory {
public:
	ParticleAffector*	create(const std::string&) const;
};

class ParticleEmitter {
public:
						ParticleEmitter();
	virtual 			~ParticleEmitter();

	virtual Particle*	createParticle() = 0;
	virtual void		parse(XMLTree::iterator);

	virtual ParticleEmitter*	clone() = 0;

	virtual unsigned int	getCount(Real time) = 0;

	SceneNodeVFFloat	position;
	SceneNodeSFFloat	rate;
	SceneNodeVFFloat	direction;
	SceneNodeVFFloat	up;
	SceneNodeSFFloat	angle;
	SceneNodeSFFloat	minSpeed;
	SceneNodeSFFloat	maxSpeed;
	SceneNodeSFFloat	minTTL;
	SceneNodeSFFloat	maxTTL;
	SceneNodeVFFloat	colorStart;
	SceneNodeVFFloat	colorEnd;

protected:
	void				createPosition(Vec3& pos);
	void				createDirection(Vec3& dir);
	void				createVelocity(Vec3& vel);
	void				createTTL(Real& ttl);
	void				createColor(SceneNodeVFFloat& col);
	unsigned short		createConstantEmissionCount(Real time);
};

class ParticleEmitterPoint : public ParticleEmitter {
public:
						ParticleEmitterPoint();
	virtual				~ParticleEmitterPoint();

	virtual Particle*	createParticle();
	virtual void		parse(XMLTree::iterator);

	virtual ParticleEmitter*	clone();

	virtual unsigned int	getCount(Real time);
};

class ParticleEmitterBox : public ParticleEmitter {
public:
						ParticleEmitterBox();
	virtual				~ParticleEmitterBox();

	virtual Particle*	createParticle();
	virtual void		parse(XMLTree::iterator);

	virtual ParticleEmitter*	clone();

	virtual unsigned int	getCount(Real time);

protected:
	SceneNodeVFFloat	size;
};

class ParticleEmitterFactory {
public:
	ParticleEmitter*	create(const std::string&) const;
};

class SceneNodeParticleSystem : public SceneNodeBillboard {
public:
				SceneNodeParticleSystem();

	typedef std::deque<Particle*>			ParticleList;
	typedef std::deque<ParticleAffector *>	AffectorList;
	typedef std::deque<ParticleEmitter *>	EmitterList;

	ParticleList		particles;
	AffectorList		affectors;
	EmitterList			emitters;

	void				setQuota(unsigned int _quota);
	unsigned int		getQuota();
	void				update(Real newTime);

	// SceneNode overrides
	virtual bool		visit(SceneVisitor*);

	// SceneNodeGroup overrides
	virtual bool		descend(SceneVisitor*, const SceneVisitorParams&);

	SceneNodeSFUInt		quota;
	SceneNodeSFFloat	width;
	SceneNodeSFFloat	height;


	// get the axis-aligned bounding box around all particles.  if there
	// are no particles then the box is point at the origin.
	void			getBoundingBox(BoundingBox* box);

	SceneNodeParticleSystem operator = (const SceneNodeParticleSystem &);

protected:
	virtual			~SceneNodeParticleSystem();

	void			expire(Real time);
	void			applyMotion(Real time);
	void			triggerAffectors(Real time);
	void			triggerEmitters(Real time);

private:
	bool			boundingBoxDirty;
	BoundingBox		boundingBox;

	Real			currentTime;
};

#endif
// ex: shiftwidth=4 tabstop=4
