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

template <>
EffectsRenderer* Singleton<EffectsRenderer>::_instance = (EffectsRenderer*)0;

// utils for geo
void drawRingYZ ( float rad, float z, float topsideOffset = 0, float bottomUV = 0, float ZOffset = 0);
void drawRingXY ( float rad, float z, float topsideOffset = 0, float bottomUV = 0);
void RadialToCartesian ( float angle, float rad, float *pos );
void getSpawnTeamColor ( int teamColor, float *color );

#define deg2Rad 0.017453292519943295769236907684886f


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

void EffectsRenderer::addSpawnEffect ( int team, const float* pos )
{
	if (!BZDB.isTrue("useFancyEffects"))
		return;

	int flashType = static_cast<int>(BZDB.eval("spawnEffect"));
	
	if (flashType == 0)
		return;

	BasicEffect	*effect = NULL;
	switch(flashType)
	{
		case 1:
			effect = new StdSpawnEffect;
			break;

		case 2:
			effect = new ConeSpawnEffect;
			break;

		case 3:
			effect = new RingSpawnEffect;
			break;
	}

	if (effect)
	{
		effect->setPos(pos,NULL);
		effect->setStartTime((float)TimeKeeper::getCurrent().getSeconds());
		effect->setTeam(team);
		effectsList.push_back(effect);
	}
}

std::vector<std::string> EffectsRenderer::getSpawnEffectTypes ( void )
{
	std::vector<std::string> ret;
	ret.push_back(std::string("None"));
	ret.push_back(std::string("Energy Fountain"));
	ret.push_back(std::string("Cone"));
	ret.push_back(std::string("Rings"));

	return ret;
}

void EffectsRenderer::addShotEffect ( int team, const float* pos, float rot, const float *vel )
{
	if (!BZDB.isTrue("useFancyEffects"))
		return;

	int flashType = static_cast<int>(BZDB.eval("shotEffect"));
	
	if (flashType == 0)
		return;

	float rots[3] = {0};
	rots[2] = rot;

	BasicEffect	*effect = NULL;
	switch(flashType)
	{
		case 1:
			effect = new StdShotEffect;
			break;
		case 2:
			effect = new FlashShotEffect;
			break;
		case 3:
			// composite effect
			BZDB.set("shotEffect", "1", StateDatabase::User);
			addShotEffect(team, pos, rot, vel);
			BZDB.set("shotEffect", "2", StateDatabase::User);
			addShotEffect(team, pos, rot, vel);
			BZDB.set("shotEffect", "3", StateDatabase::User);
			break;
	}

	if (effect)
	{
		effect->setPos(pos,rots);
		effect->setStartTime((float)TimeKeeper::getCurrent().getSeconds());
		if (BZDB.isTrue("useVelOnShotEffects"))
			effect->setVel(vel);
		effect->setTeam(team);

		effectsList.push_back(effect);
	}
}

std::vector<std::string> EffectsRenderer::getShotEffectTypes ( void )
{
	std::vector<std::string> ret;
	ret.push_back(std::string("None"));
	ret.push_back(std::string("Smoke Rings"));
	ret.push_back(std::string("Muzzle Flash"));
	ret.push_back(std::string("Smoke and Flash"));

	return ret;
}

void EffectsRenderer::addDeathEffect ( int team, const float* pos, float rot )
{
	if (!BZDB.isTrue("useFancyEffects"))
		return;

	int effectType = static_cast<int>(BZDB.eval("deathEffect"));

	if (effectType == 0)
		return;

	BasicEffect	*effect = NULL;

	float rots[3] = {0};
	rots[2] = rot;

	switch(effectType)
	{
		case 1:
			effect = new StdDeathEffect;
			break;
	}

	if (effect)
	{
		effect->setPos(pos,rots);
		effect->setStartTime((float)TimeKeeper::getCurrent().getSeconds());
		effect->setTeam(team);
		effectsList.push_back(effect);
	}
}

std::vector<std::string> EffectsRenderer::getDeathEffectTypes ( void )
{
	std::vector<std::string> ret;
	ret.push_back(std::string("None"));
	ret.push_back(std::string("We Got Death Star"));

	return ret;
}

//****************** effects base class*******************************
BasicEffect::BasicEffect()
{
	position[0] = position[1] = position[2] = 0.0f;
	rotation[0] = rotation[1] = rotation[2] = 0.0f;
	velocity[0] = velocity[1] = velocity[2] = 0.0f;
	teamColor = -1;
	startTime = (float)TimeKeeper::getCurrent().getSeconds();

	lifetime = 0;
	lastTime = startTime;
	deltaTime = 0;
}

void BasicEffect::setPos ( const float *pos, const float *rot )
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

void BasicEffect::setVel ( const float *vel )
{
	if (vel)
	{
		velocity[0] = vel[0];
		velocity[1] = vel[1];
		velocity[2] = vel[2];
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

//******************StdSpawnEffect****************
StdSpawnEffect::StdSpawnEffect() : BasicEffect()
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

StdSpawnEffect::~StdSpawnEffect()
{
}

bool StdSpawnEffect::update ( float time )
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

void StdSpawnEffect::draw(const SceneRenderer &)
{
	glPushMatrix();

	glTranslatef(position[0],position[1],position[2]+0.1f);

	ringState.setState();

	float color[3] = {0};

	getSpawnTeamColor(teamColor,color);
	
	float ageParam = age/lifetime;

	glColor4f(color[0],color[1],color[2],1.0f-(age/lifetime));
	glDepthMask(0);

	drawRingXY(radius*0.1f,2.5f+(age*2));
	drawRingXY(radius*0.5f,1.5f + (ageParam/1.0f * 2),0.5f,0.5f);
	drawRingXY(radius,2);

	glColor4f(1,1,1,1);
	glDepthMask(1);
	glPopMatrix();
}

//******************ConeSpawnEffect****************
bool ConeSpawnEffect::update ( float time )
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

void ConeSpawnEffect::draw(const SceneRenderer &)
{
	glPushMatrix();

	glTranslatef(position[0],position[1],position[2]+0.1f);

	ringState.setState();

	float color[3] = {0};

	getSpawnTeamColor(teamColor,color);

	glColor4f(color[0],color[1],color[2],1.0f-(age/lifetime));
	glDepthMask(0);

	drawRingXY(radius*0.5f,1.25f);
	
	glTranslatef(0,0,2);
	drawRingXY(radius*0.6f,1.5f);

	glTranslatef(0,0,2);
	drawRingXY(radius*0.75f,1.75f);

	glTranslatef(0,0,2);
	drawRingXY(radius*0.85f,1.89f);

	glTranslatef(0,0,2);
	drawRingXY(radius,2.0f);

	glColor4f(1,1,1,1);
	glDepthMask(1);
	glPopMatrix();
}


//******************RingSpawnEffect****************
bool RingSpawnEffect::update ( float time )
{
	// see if it's time to die
	// if not update all those fun times
	if ( BasicEffect::update(time))
		return true;

	// nope it's not.
	// we live another day
	// do stuff that maybe need to be done every time to animage

	radius  = 4.0f;
	return false;
}

void RingSpawnEffect::draw(const SceneRenderer &)
{
	glPushMatrix();

	glTranslatef(position[0],position[1],position[2]);

	ringState.setState();

	float color[3] = {0};

	getSpawnTeamColor(teamColor,color);

	glDepthMask(0);

	float range = lifetime/4.0f;	// first 3/4ths of the life are rings, last is fade
	range = (range*3)/4.0f; // of the ring section there are 4 ring segments
	float bigRange = range*3;

	float posZ = 0;
	float maxZ = 10;
	float alpha;

	float coreAlpha = 1;

	if ( age >= bigRange)
	{
		coreAlpha = 1.0f - ((age-bigRange)/(lifetime-bigRange));
	}
	// range is now how long one ring segement is
	if (age > 0 )	// the first ring
	{
		if ( age < range ) // the ring is still coming in
		{ 
			posZ = maxZ - ((age/range)*maxZ);
			alpha = age/range;
		}
		else
		{
			posZ = 0;
			alpha = coreAlpha;
		}

		glPushMatrix();
		glTranslatef(0,0,posZ);
		glColor4f(color[0],color[1],color[2],alpha);
		drawRingXY(radius,2.5f);
		glPopMatrix();
	}

	if (age > range )	// second ring in?
	{
		if ( age < range*2 ) // the ring is still coming in
		{ 
			posZ = maxZ - ((age-range)/range)*(maxZ-2.5f);
			alpha = (age-range)/(range*2);
		}
		else
		{
			posZ = 2.5f;
			alpha = coreAlpha;
		}

		glPushMatrix();
		glTranslatef(0,0,posZ);
		glColor4f(color[0],color[1],color[2],alpha);
		drawRingXY(radius,2.5f);
		glPopMatrix();
	}

	if (age > (range*2) )	// third ring in?
	{
		if ( age < range*3 ) // the ring is still coming in
		{ 
			posZ = maxZ - ((age-range*2)/range)*(maxZ-5.0f);
			alpha = (age-range)/(range*3);
		}
		else
		{
			posZ = 5.0f;
			alpha = coreAlpha;
		}

		glPushMatrix();
		glTranslatef(0,0,posZ);
		glColor4f(color[0],color[1],color[2],alpha);
		drawRingXY(radius,2.5f);
		glPopMatrix();
	}
	
	if (age > (range*3) )	// third ring in?
	{
		if ( age < range*4 ) // the ring is still coming in
		{ 
			posZ = maxZ - ((age-range*3)/range)*(maxZ-7.5f);
			alpha = (age-range)/(range*43);
		}
		else
		{
			posZ = 7.5f;
			alpha = coreAlpha;
		}

		glPushMatrix();
		glTranslatef(0,0,posZ);
		glColor4f(color[0],color[1],color[2],alpha);
		drawRingXY(radius,2.5f);
		glPopMatrix();
	} 

	glColor4f(1,1,1,1);
	glDepthMask(1);
	glPopMatrix();
}

//******************StdShotEffect****************
StdShotEffect::StdShotEffect() : BasicEffect()
{
	texture = TextureManager::instance().getTextureID("blend_flash",false);
	lifetime = 1.5f;
	radius = 0.125f;


	OpenGLGStateBuilder gstate;
	gstate.reset();
	gstate.setShading();
	gstate.setBlending((GLenum) GL_SRC_ALPHA,(GLenum) GL_ONE_MINUS_SRC_ALPHA);
	gstate.setAlphaFunc();

	if (texture >-1)
		gstate.setTexture(texture);

	ringState = gstate.getState();
}

StdShotEffect::~StdShotEffect()
{
}

bool StdShotEffect::update ( float time )
{
	// see if it's time to die
	// if not update all those fun times
	if ( BasicEffect::update(time))
		return true;

	// nope it's not.
	// we live another day
	// do stuff that maybe need to be done every time to animage

	radius += deltaTime*6;
	return false;
}

void StdShotEffect::draw(const SceneRenderer &)
{
	glPushMatrix();

	float pos[3];

	pos[0] = position[0] + velocity[0] * age;
	pos[1] = position[1] + velocity[1] * age;
	pos[2] = position[2] + velocity[2] * age;

	glTranslatef(pos[0],pos[1],pos[2]);
	glRotatef(180+rotation[2]/deg2Rad,0,0,1);

	ringState.setState();

	float color[3] = {0};
	color[0] = color[1] = color[2] = 1;

	float alpha = 0.5f-(age/lifetime);
	if (alpha < 0.001f)
		alpha = 0.001f;

	glColor4f(color[0],color[1],color[2],alpha);
	glDepthMask(0);

	drawRingYZ(radius,0.5f /*+ (age * 0.125f)*/,1.0f+age*5,0.65f,pos[2]);

	glColor4f(1,1,1,1);
	glDepthMask(1);
	glPopMatrix();
}

//******************FlashShotEffect****************
FlashShotEffect::FlashShotEffect() : StdShotEffect()
{
	// we use the jump jet texture upside-down to get a decent muzzle flare effect
	texture = TextureManager::instance().getTextureID("jumpjets",false);
	lifetime = 0.75f;
	radius = 0.75f;

	OpenGLGStateBuilder gstate;
	gstate.reset();
	gstate.setShading();
	gstate.setBlending((GLenum) GL_SRC_ALPHA,(GLenum) GL_ONE_MINUS_SRC_ALPHA);
	gstate.setAlphaFunc();

	if (texture >-1)
		gstate.setTexture(texture);

	ringState = gstate.getState();
}

bool FlashShotEffect::update ( float time )
{
	// see if it's time to die
	// if not update all those fun times
	if (BasicEffect::update(time))
		return true;

	// nope it's not.
	// we live another day
	// do stuff that maybe need to be done every time to animage
	if (age < lifetime / 2)
	  length = 8 * (age / lifetime);
	else
	  length = 8 * (1 - (age / lifetime));

	return false;
}

void FlashShotEffect::draw(const SceneRenderer &)
{
	glPushMatrix();

	float pos[3];

	pos[0] = position[0] + velocity[0] * age;
	pos[1] = position[1] + velocity[1] * age;
	pos[2] = position[2] + velocity[2] * age;

	glTranslatef(pos[0],pos[1],pos[2]);
	glRotatef(270+rotation[2]/deg2Rad,0,0,1);

	ringState.setState();

	float color[3] = {0};
	color[0] = color[1] = color[2] = 1;

	float alpha = 0.8f-(age/lifetime);
	if (alpha < 0.001f)
		alpha = 0.001f;

	glColor4f(color[0],color[1],color[2],alpha);
	glDepthMask(0);

	// draw me here
	glBegin(GL_QUADS);

		// side 1
		glTexCoord2f(0,1);
		glVertex3f(0,0,radius);

		glTexCoord2f(0,0);
		glVertex3f(0,length,radius);

		glTexCoord2f(1,0);
		glVertex3f(0,length,-radius);

		glTexCoord2f(1,1);
		glVertex3f(0,0,-radius);

		// side 2
		glTexCoord2f(1,1);
		glVertex3f(0,0,-radius);

		glTexCoord2f(1,0);
		glVertex3f(0,length,-radius);

		glTexCoord2f(0,0);
		glVertex3f(0,length,radius);

		glTexCoord2f(0,1);
		glVertex3f(0,0,radius);

	glEnd();

	glColor4f(1,1,1,1);
	glDepthMask(1);
	glPopMatrix();
}

//******************StdDeathEffect****************
StdDeathEffect::StdDeathEffect() : BasicEffect()
{
	texture = TextureManager::instance().getTextureID("blend_flash",false);
	lifetime = 1.5f;
	radius = 2.0f;


	OpenGLGStateBuilder gstate;
	gstate.reset();
	gstate.setShading();
	gstate.setBlending((GLenum) GL_SRC_ALPHA,(GLenum) GL_ONE_MINUS_SRC_ALPHA);
	gstate.setAlphaFunc();

	if (texture >-1)
		gstate.setTexture(texture);

	ringState = gstate.getState();
}

StdDeathEffect::~StdDeathEffect()
{
}

bool StdDeathEffect::update ( float time )
{
	// see if it's time to die
	// if not update all those fun times
	if ( BasicEffect::update(time))
		return true;

	// nope it's not.
	// we live another day
	// do stuff that maybe need to be done every time to animage

	radius += deltaTime*20;
	return false;
}

void StdDeathEffect::draw(const SceneRenderer &)
{
	glPushMatrix();

	glTranslatef(position[0],position[1],position[2]);
	glRotatef(180+rotation[2]/deg2Rad,0,0,1);

	ringState.setState();

	float color[3] = {0};
	color[0] = 108.0f/256.0f;
	color[1] = 16.0f/256.0f;
	color[2] = 16.0f/256.0f;

	float deltas[3];

	deltas[0] = 1.0f - color[0];
	deltas[1] = 1.0f - color[1];
	deltas[2] = 1.0f - color[2];


	float ageParam = age/lifetime;

	float alpha = 1.0f-(ageParam*0.5f);
	if (alpha < 0.005f)
		alpha = 0.005f;

	color[0] += deltas[0] *ageParam;
	color[1] += deltas[1] *ageParam;
	color[2] += deltas[2] *ageParam;

	glColor4f(color[0],color[1],color[2],alpha);
	glDepthMask(0);

	glPushMatrix();
	glTranslatef(0,0,0.5f);
	drawRingXY(radius*0.75f,1.5f + (ageParam/1.0f * 10),0.5f*age,0.5f);
	drawRingXY(radius,-0.5f,0.5f+ age,0.5f);

	glRotatef(90,0,0,1);
	drawRingYZ(radius,3,0,0,position[2]+0.5f);
	glPopMatrix();

	glColor4f(1,1,1,1);
	glDepthMask(1);
	glPopMatrix();
}

//******************************** geo utiliys********************************

void RadialToCartesian ( float angle, float rad, float *pos )
{
	pos[0] = sin(angle*deg2Rad)*rad;
	pos[1] = cos(angle*deg2Rad)*rad;
}

void drawRingXY ( float rad, float z, float topsideOffset, float bottomUV )
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

float clampedZ ( float z, float offset )
{
	if ( z +offset > 0.0f)
		return z;
	return -offset;
}

void drawRingYZ ( float rad, float z, float topsideOffset, float bottomUV, float ZOffset )
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
		glVertex3f(0,thispos[1],clampedZ(thispos[0],ZOffset));

		glNormal3f(-nextNormal[0],-nextNormal[1],-nextNormal[2]);
		glTexCoord2f(1,bottomUV);
		glVertex3f(0,nextPos[1],clampedZ(nextPos[0],ZOffset));

		glNormal3f(-nextNormal[0],-nextNormal[1],-nextNormal[2]);
		glTexCoord2f(1,1);
		glVertex3f(z,nextPos2[1],clampedZ(nextPos2[0],ZOffset));

		glNormal3f(-thisNormal[0],-thisNormal[1],-thisNormal[2]);
		glTexCoord2f(0,1);
		glVertex3f(z,thispos2[1],clampedZ(thispos2[0],ZOffset));

		// the "outside"

		glNormal3f(thisNormal[0],thisNormal[1],thisNormal[2]);
		glTexCoord2f(0,1);
		glVertex3f(z,thispos2[1],clampedZ(thispos2[0],ZOffset));

		glNormal3f(nextNormal[0],nextNormal[1],nextNormal[2]);
		glTexCoord2f(1,1);
		glVertex3f(z,nextPos2[1],clampedZ(nextPos2[0],ZOffset));

		glNormal3f(nextNormal[0],nextNormal[1],nextNormal[2]);
		glTexCoord2f(1,bottomUV);
		glVertex3f(0,nextPos[1],clampedZ(nextPos[0],ZOffset));

		glNormal3f(thisNormal[0],thisNormal[1],thisNormal[2]);
		glTexCoord2f(0,bottomUV);
		glVertex3f(0,thispos[1],clampedZ(thispos[0],ZOffset));

		glEnd();
	}
}

void getSpawnTeamColor ( int teamColor, float *color )
{
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
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

