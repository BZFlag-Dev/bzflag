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

// bzflag global common header
#include "common.h"
#include "global.h"

// interface header
#include "effectsRenderer.h"

// system headers
#include <string>
#include <vector>

// common impl headers
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "OpenGLMaterial.h"
#include "TextureManager.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "TimeKeeper.h"
#include "TextUtils.h"
#include "ParseColor.h"

// local impl headers
#include "SceneRenderer.h"
#include "Intersect.h"
#include "RoofTops.h"

EffectsRenderer::EffectsRenderer()
{
}

EffectsRenderer::~EffectsRenderer()
{
	for ( unsigned int i = 0; i < effectsList.size(); i++ )
		delete(effectsList[i]);

	effectsList.clear();
}

void EffectsRenderer::init(void)
{
	for ( unsigned int i = 0; i < effectsList.size(); i++ )
		delete(effectsList[i]);

	effectsList.clear();
}

void EffectsRenderer::update(void)
{
	tvEffectsList::iterator itr = effectsList.begin();

	float time = (float)TimeKeeper::getCurrent().getSeconds();

	while ( itr != effectsList.end() )
	{
		if ( (*itr)->update(time) )
		{
			delete((*itr));
			itr = effectsList.erase(itr);
		}
		else
			itr++;
	}
}

void EffectsRenderer::draw(const SceneRenderer& sr)
{
	// really should check here for only the things that are VISIBILE!!!

	for ( unsigned int i = 0; i < effectsList.size(); i++ )
		effectsList[i]->draw(sr);
}

void EffectsRenderer::freeContext(void)
{
	for ( unsigned int i = 0; i < effectsList.size(); i++ )
		effectsList[i]->freeContext();
}

void EffectsRenderer::rebuildContext(void)
{
	for ( unsigned int i = 0; i < effectsList.size(); i++ )
		effectsList[i]->rebuildContext();
}

void EffectsRenderer::addSpawnFlash ( int team, float* pos )
{
	SpawnFlashEffect	*flash = new SpawnFlashEffect;
	flash->setPos(pos,NULL);
	flash->setStartTime((float)TimeKeeper::getCurrent().getSeconds());
	flash->setTeam(team);

	effectsList.push_back(flash);
}

//****************** effects base class*******************************
BasicEffect::BasicEffect()
{
	position[0] = position[1] = position[2] = 0.0f;
	rotation[0] = rotation[1] = rotation[2] = 0.0f;
	teamColor = -1;
	startTime = (float)TimeKeeper::getCurrent().getSeconds();

	lifetime = 0;
	lastTime = startTime;
	deltaTime = 0;
}

void BasicEffect::setPos ( float *pos, float *rot )
{
	if (pos)
	{
		position[0] = pos[0];
		position[1] = pos[1];
		position[2] = pos[2];
	}

	if (rot)
	{
		rotation[0] = rot[0];
		rotation[1] = rot[1];
		rotation[2] = rot[2];
	}
}

void BasicEffect::setTeam ( int team )
{
	teamColor = team;
}

void BasicEffect::setStartTime ( float time )
{
	startTime = time;
	lastTime = time;
	deltaTime = 0;
}

bool BasicEffect::update( float time )
{
	age = time - startTime;

	if ( age >= lifetime)
		return true;

	deltaTime = time - lastTime;
	lastTime = time;
	return false;
}

SpawnFlashEffect::SpawnFlashEffect() : BasicEffect()
{
	texture = TextureManager::instance().getTextureID("blend_flash",false);
	lifetime = 2.0f;
	radius = 1.75f;


	OpenGLGStateBuilder gstate;
	gstate.reset();
	gstate.setShading();
	gstate.setBlending((GLenum) GL_SRC_ALPHA,(GLenum) GL_ONE_MINUS_SRC_ALPHA);
	gstate.setAlphaFunc();

	if (texture >-1)
		gstate.setTexture(texture);

	ringState = gstate.getState();
}

SpawnFlashEffect::~SpawnFlashEffect()
{
}

bool SpawnFlashEffect::update ( float time )
{
	// see if it's time to die
	// if not update all those fun times
	if ( BasicEffect::update(time))
		return true;

	// nope it's not.
	// we live another day
	// do stuff that maybe need to be done every time to animage

	radius += deltaTime*5;
	return false;
}

void SpawnFlashEffect::draw ( const SceneRenderer& sr )
{
	glPushMatrix();

	glTranslatef(position[0],position[1],position[2]+0.1f);

	ringState.setState();

	float color[3] = {0};
	switch(teamColor)
	{
	default:
		color[0] = color[1] = color[2] = 1;
		break;

	case BlueTeam:
		color[0] = 0.35f;
		color[1] = 0.35f;
		color[2] = 1;
		break;

	case GreenTeam:
		color[0] = 0.25f;
		color[1] = 1;
		color[2] = 0.25f;
		break;

	case RedTeam:
		color[0] = 1;
		color[1] = 0.35f;
		color[2] = 0.35f;
		break;

	case PurpleTeam:
		color[0] = 1;
		color[1] = 0.35f;
		color[2] = 1.0f;
		break;

	case RogueTeam:
		color[0] = 0.5;
		color[1] = 0.5f;
		color[2] = 0.5f;
		break;
	}

	float ageParam = age/lifetime;

	glColor4f(color[0],color[1],color[2],1.0f-(age/lifetime));
	glDepthMask(0);

	drawRing(radius*0.1f,2.5f+(age*2));
	drawRing(radius*0.5f,1.5f + (ageParam/1.0f * 2),0.5f,0.5f);
	drawRing(radius,2);

	glColor4f(1,1,1,1);
	glDepthMask(1);
	glPopMatrix();
}

#define deg2Rad 0.017453292519943295769236907684886f

void RadialToCartesian ( float angle, float rad, float *pos )
{
	pos[0] = sin(angle*deg2Rad)*rad;
	pos[1] = cos(angle*deg2Rad)*rad;
}

void SpawnFlashEffect::drawRing ( float rad, float z, float topsideOffset, float bottomUV )
{
	int segements = 32;

	for ( int i = 0; i < segements; i ++)
	{
		float thisAng = 360.0f/segements * i;
		float nextAng = 360.0f/segements * (i+1);
		if ( i+1 >= segements )
			nextAng = 0;

		float thispos[2];
		float nextPos[2];

		float thispos2[2];
		float nextPos2[2];

		float thisNormal[3] = {0};
		float nextNormal[3] = {0};

		RadialToCartesian(thisAng,rad,thispos);
		RadialToCartesian(thisAng,1,thisNormal);
		RadialToCartesian(nextAng,rad,nextPos);
		RadialToCartesian(nextAng,1,nextNormal);

		RadialToCartesian(thisAng,rad+topsideOffset,thispos2);
		RadialToCartesian(nextAng,rad+topsideOffset,nextPos2);

		glBegin(GL_QUADS);

		// the "inside"
			glNormal3f(-thisNormal[0],-thisNormal[1],-thisNormal[2]);
			glTexCoord2f(0,bottomUV);
			glVertex3f(thispos[0],thispos[1],0);

			glNormal3f(-nextNormal[0],-nextNormal[1],-nextNormal[2]);
			glTexCoord2f(1,bottomUV);
			glVertex3f(nextPos[0],nextPos[1],0);

			glNormal3f(-nextNormal[0],-nextNormal[1],-nextNormal[2]);
			glTexCoord2f(1,1);
			glVertex3f(nextPos2[0],nextPos2[1],z);

			glNormal3f(-thisNormal[0],-thisNormal[1],-thisNormal[2]);
			glTexCoord2f(0,1);
			glVertex3f(thispos2[0],thispos2[1],z);

		// the "outside"

			glNormal3f(thisNormal[0],thisNormal[1],thisNormal[2]);
			glTexCoord2f(0,1);
			glVertex3f(thispos2[0],thispos2[1],z);

			glNormal3f(nextNormal[0],nextNormal[1],nextNormal[2]);
			glTexCoord2f(1,1);
			glVertex3f(nextPos2[0],nextPos2[1],z);

			glNormal3f(nextNormal[0],nextNormal[1],nextNormal[2]);
			glTexCoord2f(1,bottomUV);
			glVertex3f(nextPos[0],nextPos[1],0);

			glNormal3f(thisNormal[0],thisNormal[1],thisNormal[2]);
			glTexCoord2f(0,bottomUV);
			glVertex3f(thispos[0],thispos[1],0);

		glEnd();

	}
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

