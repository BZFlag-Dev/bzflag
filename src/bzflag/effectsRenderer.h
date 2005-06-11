/* bzflag
* Copyright (c) 1993 - 2005 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
* EffectsRenderer:
*	Encapsulates rendering of effects ( spawn flashes, sparks, explosions, etc...)
*
*/

#ifndef	BZF_EFFECTS_RENDERER_H
#define	BZF_EFFECTS_RENDERER_H

#include "common.h"

/* system headers */
#include <string>
#include <vector>

/* common interface headers */
#include "bzfgl.h"
#include "Extents.h"
#include "OpenGLGState.h"
#include "SceneRenderer.h"

class BasicEffect
{
public:
	BasicEffect();
	virtual ~BasicEffect(){};

	virtual void setPos ( float *pos, float *rot );
	virtual void setTeam ( int team );
	virtual void setStartTime ( float time );

	virtual void freeContext(void){};
	virtual void rebuildContext(void){};

	virtual bool update ( float time );
	virtual void draw ( const SceneRenderer& sr ){};

protected:

	float	position[3];
	float	rotation[3];
	int		teamColor;
	float	startTime;
	float	lifetime;
	float	lastTime;
	float	deltaTime;
	float	age;
};

typedef std::vector<BasicEffect*>	tvEffectsList;

class SpawnFlashEffect : public BasicEffect
{
public:
	SpawnFlashEffect();
	virtual ~SpawnFlashEffect();

	virtual bool update ( float time );
	virtual void draw ( const SceneRenderer& sr );

protected:
	int				texture;
	OpenGLGState	ringState;

	float			radius;

	void drawRing ( float rad, float z, float topsideOffset = 0, float bottomUV = 0);
};

class EffectsRenderer {
public:
	EffectsRenderer();
	~EffectsRenderer();

	// called once to setup the effects system
	void init(void);

	// called to update the various effects
	void update(void);

	// called to draw all the current effects
	void draw(const SceneRenderer& sr);

	// called when the GL lists need to be deleted
	void freeContext(void);

	// called when the GL lists need to be remade
	void rebuildContext(void);

	void addSpawnFlash ( int team, float* pos );

protected:
	tvEffectsList	effectsList;
};

#endif // BZF_EFFECTS_RENDERER_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

