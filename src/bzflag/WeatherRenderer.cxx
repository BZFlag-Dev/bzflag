/* bzflag
* Copyright (c) 1993 - 2004 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "common.h"
#include "global.h"
#include "OpenGLMaterial.h"
#include "OpenGLTexture.h"
#include "WeatherRenderer.h"
#include "stars.h"
#include "TextureManager.h"
#include "StateDatabase.h"
#include "SceneRenderer.h"
#include "BZDBCache.h"
#include "daylight.h"
#include "MainWindow.h"
#include "SceneNode.h"
#include "TimeKeeper.h"
#include "TextUtils.h"

WeatherRenderer::WeatherRenderer()
{
	rainColor[0][0] = 0.75f;   rainColor[0][1] = 0.75f;   rainColor[0][2] = 0.75f;   rainColor[0][3] = 0.75f; 
	rainColor[1][0] = 0.0f;   rainColor[1][1] = 0.0f;   rainColor[1][2] = 0.0f;   rainColor[1][3] = 0.0f; 

	rainSize[0] = rainSize[1] = 1.0f;

	rainDensity  = 1000;
	rainSpeed = -100;
	rainSpeedMod = 10;
	rainSpread = 500;

	doPuddles = true;
	doLineRain = true;

	rainStartZ = -1;
	rainEndZ = 0;

	lastRainTime = 0;

	maxPuddleTime = 5;
	puddleSpeed = 1.0f;

	puddleColor[0] = puddleColor[1] = puddleColor[2] = puddleColor[3] = 1.0f;
}

WeatherRenderer::~WeatherRenderer()
{
}

void WeatherRenderer::init ( void )
{
	OpenGLGStateBuilder gstate;

	static const GLfloat	white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	OpenGLMaterial rainMaterial(white, white, 0.0f);

	TextureManager &tm = TextureManager::instance();

	gstate.reset();
	gstate.setShading();
	gstate.setBlending((GLenum)GL_SRC_ALPHA, (GLenum)GL_ONE_MINUS_SRC_ALPHA);
	gstate.setMaterial(rainMaterial);
	gstate.setAlphaFunc();
	rainGState = gstate.getState();

	// just set up some default textures
	gstate.setTexture(tm.getTextureID("snowflake"));
	texturedRainState = gstate.getState();

	gstate.setTexture(tm.getTextureID("puddle"));
	puddleState = gstate.getState();

	buildPuddleList();
}

void WeatherRenderer::set ( void )
{
	//TextureManager &tm = TextureManager::instance();

	// check the bzdb and see if we need to change any rain stuff
	rainDensity = 0;

	if (BZDB.isSet("rainType") || BZDB.isSet("rainDensity"))
	{
		// default rain desnity
		rainDensity = 1000;

		// some defaults
		doLineRain = false;
		rainSize[0] = 1.0f; rainSize[1] = 1.0f;
		rainSpeed = -100.0f;
		rainSpeedMod = 50.0f;
		doPuddles = true;
		rainColor[0][0] = 0.75f;   rainColor[0][1] = 0.75f;   rainColor[0][2] = 0.75f;   rainColor[0][3] = 0.75f; 
		rainColor[1][0] = 0.0f;   rainColor[1][1] = 0.0f;   rainColor[1][2] = 0.0f;   rainColor[1][3] = 0.0f; 
		rainSize[0] = rainSize[1] = 1.0f;
		maxPuddleTime = 1.5F;
		puddleSpeed = 1.0f;
		puddleColor[0] = puddleColor[1] = puddleColor[2] = puddleColor[3] = 1.0f;

		rainSpread  = 500.0f;
		if (BZDB.isSet("rainSpread"))
			rainSpread = BZDB.eval("rainSpread");

		TextureManager &tm = TextureManager::instance();

		static const GLfloat	white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		OpenGLMaterial rainMaterial(white, white, 0.0f);

		OpenGLGStateBuilder gstate;
		gstate.reset();
		gstate.setShading();
		gstate.setBlending((GLenum)GL_SRC_ALPHA, (GLenum)GL_ONE_MINUS_SRC_ALPHA);
		gstate.setMaterial(rainMaterial);
		gstate.setAlphaFunc();

		OpenGLGStateBuilder puddleGStateBuilder(puddleState);

		puddleGStateBuilder.setTexture(tm.getTextureID("puddle"));

		if (BZDB.isSet("rainType"))
		{
			std::string rainType = TextUtils::tolower(BZDB.get("rainType"));

			if (rainType == "snow")
			{
				doBillBoards = false;
				gstate.setTexture(tm.getTextureID("snowflake"));
				rainSpeed = -20.0f;
				rainSpeedMod = 5.0f;
				doPuddles = false;
			}
			else if (rainType == "fatrain")
			{
				doBillBoards = false;
				rainSize[0] = 0.5f; rainSize[1] = 0.75f;
				gstate.setTexture(tm.getTextureID("raindrop"));
				rainSpeed = -50.0f;
				rainSpeedMod = 25.0f;
			}
			else if (rainType == "frog")
			{
				gstate.setTexture(tm.getTextureID("frog"));
				rainSpeed = -100.0f;
				rainSpeedMod = 5.0f;
				doPuddles = true;
				doBillBoards = true;
				// ewww
				puddleSpeed = 3.0f;
				puddleColor[0] = 0.75f;
				puddleColor[1] = puddleColor[2] = 0.0f;
				rainSize[0] = 2.0; rainSize[1] = 2.0f;
			}
			else if (rainType == "particle")
			{
				rainDensity = 500;
				gstate.setTexture(tm.getTextureID("red_super_bolt"));
				rainSpeed = -20.0f;
				rainSpeedMod = 5.0f;
				doPuddles = true;
				doBillBoards = true;
				puddleSpeed = 10.0f;
				// ewww
				puddleColor[0] = 1.0f;
				puddleColor[1] = puddleColor[2] = 0.0f;
				rainSize[0] = 1.0; rainSize[1] = 1.0f;
			}
			else if (rainType == "bubble")
			{
				gstate.setTexture(tm.getTextureID("bubble"));
				rainSpeed = 20.0f;
				rainSpeedMod = 1.0f;
				doBillBoards = true;
				doPuddles = false;
			}
			else if (rainType == "rain")
			{
				doLineRain = true;
				rainSize[0] = 0; rainSize[1] = 0.75f;
			}
		}

		if (BZDB.isSet("rainPuddleTexture"))
			puddleGStateBuilder.setTexture(tm.getTextureID(BZDB.get("rainPuddleTexture").c_str()));

		OpenGLMaterial puddleMaterial(puddleColor, puddleColor, 0.0f);
		puddleGStateBuilder.setMaterial(puddleMaterial);
		puddleState = puddleGStateBuilder.getState();

		// see if the texture is specificly overiden
		if (BZDB.isSet("rainTexture"))
			gstate.setTexture(tm.getTextureID(BZDB.get("rainTexture").c_str()));

		texturedRainState = gstate.getState();

		// if there is a specific overides for stuff
		if (BZDB.isSet("rainDensity"))
			rainDensity = (int)BZDB.eval("rainDensity");

		if (BZDB.isSet("rainSpeed"))
			rainSpeed = BZDB.eval("rainSpeed");

		if (BZDB.isSet("rainSpeedMod"))
			rainSpeedMod = BZDB.eval("rainSpeedMod");

		if (BZDB.isSet("rainSize"))
			BZDB.evalPair("rainSize",rainSize);

		if (BZDB.isSet("rainStartZ"))
			rainStartZ = BZDB.eval("rainStartZ");

		if (BZDB.isSet("rainEndZ"))
			rainEndZ = BZDB.eval("rainEndZ");

		if (BZDB.isSet("rainBaseColor"))
			BZDB.evalTriplet("rainBaseColor",rainColor[0]);

		if (BZDB.isSet("rainTopColor"))
			BZDB.evalTriplet("rainTopColor",rainColor[1]);

		if (BZDB.isSet("rainPuddleColor"))
			BZDB.evalTriplet("rainPuddleColor",puddleColor);

		if (BZDB.isSet("rainPuddleSpeed"))
			puddleSpeed = BZDB.eval("rainPuddleSpeed");

		if (BZDB.isSet("rainMaxPuddleTime"))
			maxPuddleTime = BZDB.eval("rainMaxPuddleTime");

		// make sure we know where to start and stop the rain
		// we want to compute the heights for us
		if (rainStartZ == -1 && rainEndZ == 0)
		{
			// check the dir
			if ( rainSpeed < 0)	// rain going down
			{	
				rainStartZ = 120.0f * BZDBCache::tankHeight;	// same as the clouds
				rainEndZ = 0;
			}
			else				// rain going up ( tiny bubbles
			{
				if ( rainSpeed == 0)
					rainSpeed = 0.1f;

				rainEndZ = 120.0f * BZDBCache::tankHeight;	// same as the clouds
				rainStartZ = 0;
			}
		}
		else // the specified rain start and end values, make sure they make sense with the directon
		{
			if (rainSpeed < 0)	// rain going down
			{
				if ( rainEndZ > rainStartZ)
				{
					float temp = rainStartZ;
					rainStartZ = rainEndZ;
					rainEndZ = temp;
				}
			}
			else // rain going up
			{
				if ( rainEndZ < rainStartZ)
				{
					float temp = rainStartZ;
					rainStartZ = rainEndZ;
					rainEndZ = temp;
				}
			}
		}

		float rainHeightDelta = rainEndZ-rainStartZ;

		if (raindrops.size() == 0)
		{
			for ( int drops = 0; drops< rainDensity; drops++)
			{
				rain drop;
				drop.speed = rainSpeed + (((float)bzfrand()*2.0f -1.0f)*rainSpeedMod);
				drop.pos[0] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);
				drop.pos[1] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);
				drop.pos[2] = rainStartZ+ (((float)bzfrand())*rainHeightDelta);
				raindrops.push_back(drop);
			}
			lastRainTime = TimeKeeper::getCurrent().getSeconds();
		}
	}

	// recompute the drops based on the posible new size
	buildDropList();
}

void WeatherRenderer::update ( void )
{
	// update the time
	float frameTime = TimeKeeper::getCurrent().getSeconds()-lastRainTime;
	lastRainTime = TimeKeeper::getCurrent().getSeconds();

	if (frameTime > 1.0f)
		frameTime = 1.0f;

	// update all the drops in the world
	std::vector<rain>::iterator itr = raindrops.begin();
	while (itr != raindrops.end())
	{
		if (updateDrop(itr,frameTime))
			itr++;
	}

	// update all the puddles
	std::vector<puddle>::iterator puddleItr = puddles.begin();

	while(puddleItr != puddles.end())
	{
		if ( updatePuddle(puddleItr,frameTime))
			puddleItr++;
	}
}

void WeatherRenderer::draw ( const SceneRenderer& sr )
{
	glDepthMask(0);
	if (doLineRain)	// we are doing line rain
	{
		rainGState.setState();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();	
		glBegin(GL_LINES);

		std::vector<rain>::iterator itr = raindrops.begin();
		while (itr != raindrops.end())
		{
			float alphaMod = 0;

			if ( itr->pos[2] < 5.0f)
				alphaMod = 1.0f - (5.0f/itr->pos[2]);

			float alphaVal = rainColor[0][3]-alphaMod;
			if (alphaVal < 0)
				alphaVal = 0;

			glColor4f(rainColor[0][0],rainColor[0][1],rainColor[0][2],alphaVal);
			glVertex3fv(itr->pos);

			alphaVal = rainColor[1][3]-alphaMod;
			if (alphaVal < 0)
				alphaVal = 0;

			glColor4f(rainColor[1][0],rainColor[1][1],rainColor[1][2],alphaVal);
			glVertex3f(itr->pos[0],itr->pos[1],itr->pos[2]+ (rainSize[1] - (itr->speed * 0.15f)));
			itr++;
		}
		glEnd();
	}
	else // 3d rain
	{
		texturedRainState.setState();
		glDisable(GL_CULL_FACE);
		glMatrixMode(GL_MODELVIEW);

		std::vector<rain>::iterator itr = raindrops.begin();
		while (itr != raindrops.end())
		{
			float alphaMod = 0;

			if ( itr->pos[2] < 2.0f)
				alphaMod = (2.0f - itr->pos[2])*0.5f;

			glColor4f(1,1,1,1.0f - alphaMod);
			glPushMatrix();
			glTranslatef(itr->pos[0],itr->pos[1],itr->pos[2]);
			if (doBillBoards)
				sr.getViewFrustum().executeBillboard();

			glRotatef(lastRainTime*10.0f * rainSpeed,0,0,1);

			dropList.execute();

			glPopMatrix();
			itr++;
		}
		glEnable(GL_CULL_FACE);
	}
	if (doPuddles)
	{
		std::vector<puddle>::iterator puddleItr = puddles.begin();

		puddleState.setState();
		glDisable(GL_CULL_FACE);
		glMatrixMode(GL_MODELVIEW);
		glColor4f(1,1,1,1.0f);

		while(puddleItr != puddles.end())
		{
			glPushMatrix();
			glTranslatef(puddleItr->pos[0],puddleItr->pos[1],puddleItr->pos[2]);

			float scale = puddleItr->time * rainSpeed*0.035f*puddleSpeed;
			float lifeTime = puddleItr->time/maxPuddleTime;

			glColor4f(1,1,1,1.0f - lifeTime);

			glScalef(scale,scale,scale);
			puddleList.execute();

			glPopMatrix();

			puddleItr++;
		}
	}
	glEnable(GL_CULL_FACE);
	glColor4f(1,1,1,1);
	glPopMatrix();
	glDepthMask(1);
}

void WeatherRenderer::rebuildContext ( void )
{
	buildPuddleList();
	buildDropList();
}

void WeatherRenderer::buildDropList ( void )
{
	dropList.begin();
	glPushMatrix();

	if (doBillBoards)
	{
		glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex3f(-rainSize[0],-rainSize[1],0);

		glTexCoord2f(1,0);
		glVertex3f(rainSize[0],-rainSize[1],0);

		glTexCoord2f(1,1);
		glVertex3f(rainSize[0],rainSize[1],0);

		glTexCoord2f(0,1);
		glVertex3f(-rainSize[0],rainSize[1],0);
		glEnd();
	}
	else
	{
		glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex3f(-rainSize[0],0,-rainSize[1]);

		glTexCoord2f(1,0);
		glVertex3f(rainSize[0],0,-rainSize[1]);

		glTexCoord2f(1,1);
		glVertex3f(rainSize[0],0,rainSize[1]);

		glTexCoord2f(0,1);
		glVertex3f(-rainSize[0],0,rainSize[1]);
		glEnd();

		glRotatef(120,0,0,1);

		glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex3f(-rainSize[0],0,-rainSize[1]);

		glTexCoord2f(1,0);
		glVertex3f(rainSize[0],0,-rainSize[1]);

		glTexCoord2f(1,1);
		glVertex3f(rainSize[0],0,rainSize[1]);

		glTexCoord2f(0,1);
		glVertex3f(-rainSize[0],0,rainSize[1]);
		glEnd();

		glRotatef(120,0,0,1);

		glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex3f(-rainSize[0],0,-rainSize[1]);

		glTexCoord2f(1,0);
		glVertex3f(rainSize[0],0,-rainSize[1]);

		glTexCoord2f(1,1);
		glVertex3f(rainSize[0],0,rainSize[1]);

		glTexCoord2f(0,1);
		glVertex3f(-rainSize[0],0,rainSize[1]);
		glEnd();
	}
	glPopMatrix();
	dropList.end();
}

void WeatherRenderer::buildPuddleList ( void )
{
	float scale = 1;
	puddleList.begin();

	glBegin(GL_QUADS);
	glTexCoord2f(0,0);
	glVertex3f(-scale,-scale,0);

	glTexCoord2f(1,0);
	glVertex3f(scale,-scale,0);

	glTexCoord2f(1,1);
	glVertex3f(scale,scale,0);

	glTexCoord2f(0,1);
	glVertex3f(-scale,scale,0);
	glEnd();

	puddleList.end();
}

bool WeatherRenderer::updateDrop ( std::vector<rain>::iterator &drop, float frameTime )
{
	drop->pos[2] += drop->speed * frameTime;

	bool killDrop = false;

	if ( drop->speed < 0)
		killDrop = drop->pos[2] < rainEndZ;
	else
		killDrop = drop->pos[2] > rainEndZ;

	if ( killDrop )
	{
		if (doPuddles)
		{	
			puddle	thePuddle;
			thePuddle.pos[0] = drop->pos[0];thePuddle.pos[1] = drop->pos[1];
			thePuddle.pos[2] = rainEndZ;;
			thePuddle.time = 0.001f;
			puddles.push_back(thePuddle);
		}

		if ( (int)(raindrops.size()) <= rainDensity)
		{
			// reset this drop
			drop->pos[2] = rainStartZ;
			drop->speed = rainSpeed + ((float)(bzfrand()*2.0f -1.0f)*rainSpeedMod);
			drop->pos[0] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);
			drop->pos[1] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);

			// we need more rain!!!
			if ((int)(raindrops.size()) < rainDensity)
			{
				rain newDrop;
				newDrop.pos[2] = rainStartZ;
				newDrop.speed = rainSpeed + ((float)(bzfrand()*2.0f -1.0f)*rainSpeedMod);
				newDrop.pos[0] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);
				newDrop.pos[1] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);
				raindrops.push_back(newDrop);
			}
			return true;
		}
		else	// we need less rain, so don't do this one
		{
			drop = raindrops.erase(drop);
			return false;
		}
	}
	return true;
}

bool WeatherRenderer::updatePuddle ( std::vector<puddle>::iterator &splash, float frameTime )
{
	if ( splash->time > maxPuddleTime )
	{
		splash = puddles.erase(splash);
		return false;
	}
	splash->time += frameTime;
	return true;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

