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

// bzflag global common header
#include "common.h"
#include "global.h"

// interface header
#include "WeatherRenderer.h"

// system headers
#include <string>
#include <vector>

// common impl headers
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "OpenGLDisplayList.h"
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

// for debug
#define _CULLING_RAIN false

void bzdbCallBack(const std::string& /*name*/, void* userData)
{
	((WeatherRenderer*)userData)->set();
}

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
	spinRain = false;

	rainStartZ = -1;
	rainEndZ = 0;

	lastRainTime = 0;

	maxPuddleTime = 5;
	puddleSpeed = 1.0f;

	puddleColor[0] = puddleColor[1] = puddleColor[2] = puddleColor[3] = 1.0f;

	gridSize = 200;

	keyFactor = 1.0f/gridSize;

	rainCount = 0;
	cellCount = 0;

	// install callbacks
	BZDB.addCallback("_rainType",bzdbCallBack,this);
	BZDB.addCallback("_rainDensity",bzdbCallBack,this);
	BZDB.addCallback("_rainSpread",bzdbCallBack,this);
	BZDB.addCallback("_rainSpeedMod",bzdbCallBack,this);
	BZDB.addCallback("_rainStartZ",bzdbCallBack,this);
	BZDB.addCallback("_rainEndZ",bzdbCallBack,this);
	BZDB.addCallback("_rainBaseColor",bzdbCallBack,this);
	BZDB.addCallback("_useRainPuddles",bzdbCallBack,this);
	BZDB.addCallback("_useLineRain",bzdbCallBack,this);
	BZDB.addCallback("_useRainBillboards",bzdbCallBack,this);
	BZDB.addCallback("_rainPuddleSpeed",bzdbCallBack,this);
	BZDB.addCallback("_rainMaxPuddleTime",bzdbCallBack,this);
	BZDB.addCallback("_rainPuddleColor",bzdbCallBack,this);
	BZDB.addCallback("_rainPuddleTexture",bzdbCallBack,this);
	BZDB.addCallback("_rainTexture",bzdbCallBack,this);
	BZDB.addCallback("_rainSpins",bzdbCallBack,this);
}

WeatherRenderer::~WeatherRenderer()
{
	BZDB.removeCallback("_rainType",bzdbCallBack,this);
	BZDB.removeCallback("_rainDensity",bzdbCallBack,this);
	BZDB.removeCallback("_rainSpread",bzdbCallBack,this);
	BZDB.removeCallback("_rainSpeedMod",bzdbCallBack,this);
	BZDB.removeCallback("_rainStartZ",bzdbCallBack,this);
	BZDB.removeCallback("_rainEndZ",bzdbCallBack,this);
	BZDB.removeCallback("_rainBaseColor",bzdbCallBack,this);
	BZDB.removeCallback("_useRainPuddles",bzdbCallBack,this);
	BZDB.removeCallback("_useLineRain",bzdbCallBack,this);
	BZDB.removeCallback("_useRainBillboards",bzdbCallBack,this);
	BZDB.removeCallback("_rainPuddleSpeed",bzdbCallBack,this);
	BZDB.removeCallback("_rainMaxPuddleTime",bzdbCallBack,this);
	BZDB.removeCallback("_rainPuddleColor",bzdbCallBack,this);
	BZDB.removeCallback("_rainPuddleTexture",bzdbCallBack,this);
	BZDB.removeCallback("_rainTexture",bzdbCallBack,this);
	BZDB.removeCallback("_rainSpins",bzdbCallBack,this);
}

void WeatherRenderer::init ( void )
{
	OpenGLGStateBuilder gstate;

//	static const GLfloat	white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
//	OpenGLMaterial rainMaterial(white, white, 0.0f);

	TextureManager &tm = TextureManager::instance();

	gstate.reset();
	gstate.setShading();
	gstate.setBlending((GLenum)GL_SRC_ALPHA, (GLenum)GL_ONE_MINUS_SRC_ALPHA);
//	gstate.setMaterial(rainMaterial);
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

	if (dbItemSet("_rainType") || dbItemSet("_rainDensity"))
	{
		// default rain desnity
		rainDensity = 1000;

		// some defaults
		doLineRain = false;
		rainSize[0] = 1.0f; rainSize[1] = 1.0f;
		rainSpeed = -100.0f;
		rainSpeedMod = 50.0f;
		doPuddles = true;
		rainColor[0][0] = 0.75f;   rainColor[0][1] = 0.75f;   rainColor[0][2] = 0.85f;   rainColor[0][3] = 0.75f; 
		rainColor[1][0] = 0.0f;   rainColor[1][1] = 0.0f;   rainColor[1][2] = 0.0f;   rainColor[1][3] = 0.0f; 
		rainSize[0] = rainSize[1] = 1.0f;
		maxPuddleTime = 1.5F;
		puddleSpeed = 1.0f;
		puddleColor[0] = puddleColor[1] = puddleColor[2] = puddleColor[3] = 1.0f;
		spinRain = true;

		rainSpread  = 500.0f;

		TextureManager &tm = TextureManager::instance();

//		static const GLfloat	white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
//		OpenGLMaterial rainMaterial(white, white, 0.0f);

		OpenGLGStateBuilder gstate;
		gstate.reset();
		gstate.setShading();
		gstate.setBlending((GLenum)GL_SRC_ALPHA, (GLenum)GL_ONE_MINUS_SRC_ALPHA);
//		gstate.setMaterial(rainMaterial);
		gstate.setAlphaFunc();

		OpenGLGStateBuilder puddleGStateBuilder(puddleState);

		puddleGStateBuilder.setTexture(tm.getTextureID("puddle"));

		if (dbItemSet("_rainType"))
		{
			std::string rainType = TextUtils::tolower(BZDB.get("_rainType"));

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
				spinRain = false;
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
				puddleColor[0] = 1.0f;
				puddleColor[1] = puddleColor[2] = 1.0f;
				doLineRain = true;
				rainSize[0] = 0; rainSize[1] = 0.75f;
				puddleGStateBuilder.setTexture(tm.getTextureID("puddle"));
			}
		}

		if (dbItemSet("_rainPuddleTexture"))
			puddleGStateBuilder.setTexture(tm.getTextureID(BZDB.get("_rainPuddleTexture").c_str()));

		// see if the texture is specificly overiden
		if (dbItemSet("_rainTexture"))
			gstate.setTexture(tm.getTextureID(BZDB.get("_rainTexture").c_str()));

		texturedRainState = gstate.getState();

		// if there is a specific overides for stuff
		if (dbItemSet("_rainSpread"))
			rainSpread = BZDB.eval("_rainSpread");
	
		if (dbItemSet("_rainDensity"))
			rainDensity = (int)BZDB.eval("_rainDensity");

		if (dbItemSet("_rainSpeed"))
			rainSpeed = BZDB.eval("_rainSpeed");

		if (dbItemSet("_rainSpeedMod"))
			rainSpeedMod = BZDB.eval("_rainSpeedMod");

		if (dbItemSet("_rainSize"))
			BZDB.evalPair("_rainSize",rainSize);

		if (dbItemSet("_rainStartZ"))
			rainStartZ = BZDB.eval("_rainStartZ");

		if (dbItemSet("_rainEndZ"))
			rainEndZ = BZDB.eval("_rainEndZ");

		if (dbItemSet("_rainBaseColor"))
			parseColorString(BZDB.get("_rainBaseColor"), rainColor[0]);

		if (dbItemSet("_rainTopColor"))
			parseColorString(BZDB.get("_rainTopColor"), rainColor[1]);

		if (dbItemSet("_useRainPuddles"))
			doPuddles = BZDB.evalInt("_useRainPuddles") == 1;

		if (dbItemSet("_useLineRain"))
			doLineRain = BZDB.evalInt("_useLineRain") == 1;

		if (dbItemSet("_useRainBillboards"))
			doBillBoards = BZDB.evalInt("_useRainBillboards") == 1;

		if (dbItemSet("_rainPuddleColor"))
			parseColorString(BZDB.get("_rainPuddleColor"), puddleColor);

		if (dbItemSet("_rainPuddleSpeed"))
			puddleSpeed = BZDB.eval("rainPuddleSpeed");

		if (dbItemSet("_rainMaxPuddleTime"))
			maxPuddleTime = BZDB.eval("_rainMaxPuddleTime");

		if (dbItemSet("_rainSpins"))
			spinRain = BZDB.evalInt("_rainSpins") == 1;

		// update the actual puddle material
//		OpenGLMaterial puddleMaterial(puddleColor, puddleColor, 0.0f);
//		puddleGStateBuilder.setMaterial(puddleMaterial);
		puddleState = puddleGStateBuilder.getState();


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

		int totalRain = rainCount;
		if (!_CULLING_RAIN)
			totalRain = raindrops.size();

		if (totalRain < rainDensity)
		{
			for ( int drops = totalRain; drops< rainDensity; drops++)
			{
				rain drop;
				drop.speed = rainSpeed + (((float)bzfrand()*2.0f -1.0f)*rainSpeedMod);
				drop.pos[0] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);
				drop.pos[1] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);
				drop.pos[2] = rainStartZ+ (((float)bzfrand())*rainHeightDelta);

				if (_CULLING_RAIN)
					addDrop(drop);
				else
					raindrops.push_back(drop);
			}
			lastRainTime = TimeKeeper::getCurrent().getSeconds();
		}
		// recompute the drops based on the posible new size
		buildDropList();

		if (_CULLING_RAIN)	// need to update the bbox depths on all the chunks
		{
			std::map<int,visibleChunk>::iterator itr = chunkMap.begin();

			while (itr != chunkMap.end())
			{
				itr->second.bboxMin[2] = rainStartZ > rainEndZ ?  rainEndZ : rainStartZ;
				itr->second.bboxMax[2] = rainStartZ > rainEndZ ?  rainStartZ : rainEndZ;
				itr++;
			}
		}
	}
	else
	{
		raindrops.clear();
		chunkMap.clear();
		rainCount = 0;
	}
}

void WeatherRenderer::update ( void )
{
	// update the time
	float frameTime = TimeKeeper::getCurrent().getSeconds()-lastRainTime;
	lastRainTime = TimeKeeper::getCurrent().getSeconds();

	std::vector<rain> dropsToAdd;

	// clamp the update time
	// its not an important sim so just keep it smooth
	if (frameTime > 0.06f)
		frameTime = 0.06f;

	if (!_CULLING_RAIN)
	{
		// update all the drops in the world
		std::vector<rain>::iterator itr = raindrops.begin();
		while (itr != raindrops.end())
		{
			if (updateDrop(itr,frameTime,dropsToAdd))
				itr++;
		}
	}
	else
	{
		std::map<int,visibleChunk>::iterator itr = chunkMap.begin();
		
		while ( itr!= chunkMap.end() )
		{
			if ( !itr->second.drops.size() )	// kill any empty chunks
			{
				//cellCount--;
				//itr == chunkMap.erase(itr);
			}
			else
			{
				std::vector<rain>::iterator dropItr = itr->second.drops.begin();
				while (dropItr != itr->second.drops.end())
				{
					if (updateDrop(dropItr,frameTime,dropsToAdd))
						dropItr++;
					else
					{
						dropItr = itr->second.drops.erase(dropItr);
						rainCount--;
					}
				}
				itr++;
			}
		}

		// add in any new drops
		std::vector<rain>::iterator dropItr = dropsToAdd.begin();
		while (dropItr != dropsToAdd.end())
		{
			addDrop(*dropItr);
			dropItr++;
		}
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
	if (!_CULLING_RAIN)	
	{
		if (!raindrops.size())
			return;
	}
	else
	{
		if (!rainCount)
			return;
	}

	int visibleChunks = 0;

	glDisable(GL_CULL_FACE);
	glMatrixMode(GL_MODELVIEW);
	glColor4f(1,1,1,1.0f);
	glDepthMask(GL_FALSE);
//	glDisable(GL_LIGHTING);

	if (doLineRain)	// we are doing line rain
	{
//		glEnable(GL_COLOR_MATERIAL);
		rainGState.setState();

		glPushMatrix();	
		glBegin(GL_LINES);
	}
	else
		texturedRainState.setState();

	if (!_CULLING_RAIN)	// draw ALL the rain
	{
		std::vector<rain>::iterator itr = raindrops.begin();
		while (itr != raindrops.end())
		{
			drawDrop(*itr,sr);
			itr++;
		}
	}
	else // do the smart thing and just draw the rain that is VISIBLE
	{
		std::map<int,visibleChunk>::iterator itr = chunkMap.begin();

		const Frustum*	frustum = &sr.getViewFrustum();
		while ( itr!= chunkMap.end() )
		{
			if ( itr->second.drops.size() )	// skip any empty chunks
			{
				if (testAxisBoxInFrustum(itr->second.bboxMin,itr->second.bboxMax,frustum) != Outside)	// see if the chunk is visible
				{
					visibleChunks++;
					std::vector<rain>::iterator dropItr = itr->second.drops.begin();
					while (dropItr != itr->second.drops.end())
					{
						drawDrop(*dropItr,sr);
						dropItr++;
					}
				}
			}	
			itr++;
		}
	}

	if (doLineRain)
	{
		glEnd();
		glPopMatrix();
//		glDisable(GL_COLOR_MATERIAL);
	}
	
	if (doPuddles)
	{
		std::vector<puddle>::iterator puddleItr = puddles.begin();

		puddleState.setState();
		while(puddleItr != puddles.end())
		{
			drawPuddle(*puddleItr);
			puddleItr++;
		}
	}

//	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glColor4f(1,1,1,1);
	glDepthMask(GL_TRUE);
}

void WeatherRenderer::rebuildContext ( void )
{
	buildPuddleList();
	buildDropList();
}

void WeatherRenderer::buildDropList ( bool draw )
{
	if (!draw)
		dropList.begin();

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
		glPushMatrix();
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
		glPopMatrix();
	}
	if (!draw)
		dropList.end();
}

void WeatherRenderer::buildPuddleList ( bool draw )
{
	float scale = 1;
	if (!draw)
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

	if (!draw)
		puddleList.end();
}

bool WeatherRenderer::updateDrop ( std::vector<rain>::iterator &drop, float frameTime, std::vector<rain> &toAdd )
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

		if (!_CULLING_RAIN)
		{
			if ( (int)(raindrops.size()) <= rainDensity)
			{
				// reset this drop
				drop->pos[2] = rainStartZ;
				drop->speed = rainSpeed + ((float)(bzfrand()*2.0f -1.0f)*rainSpeedMod);
				drop->pos[0] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);
				drop->pos[1] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);
				return true;
			}
			else	// we need less rain, so don't do this one
			{
				drop = raindrops.erase(drop);
				return false;
			}
		}
		else
		{
			if (rainCount <= rainDensity)
			{
				rain newDrop;
				newDrop.pos[2] = rainStartZ;
				newDrop.speed = rainSpeed + ((float)(bzfrand()*2.0f -1.0f)*rainSpeedMod);
				newDrop.pos[0] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);
				newDrop.pos[1] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);

				toAdd.push_back(newDrop);
			}
				// kill the drop
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

void WeatherRenderer::drawDrop ( rain &drop, const SceneRenderer& sr )
{
	if (doLineRain)
	{
		float alphaMod = 0;

		if ( drop.pos[2] < 5.0f)
			alphaMod = 1.0f - (5.0f/drop.pos[2]);

		float alphaVal = rainColor[0][3]-alphaMod;
		if (alphaVal < 0)
			alphaVal = 0;

		glColor4f(rainColor[0][0],rainColor[0][1],rainColor[0][2],alphaVal);
		glVertex3fv(drop.pos);

		alphaVal = rainColor[1][3]-alphaMod;
		if (alphaVal < 0)
			alphaVal = 0;

		glColor4f(rainColor[1][0],rainColor[1][1],rainColor[1][2],alphaVal);
		glVertex3f(drop.pos[0],drop.pos[1],drop.pos[2]+ (rainSize[1] - (drop.speed * 0.15f)));
	}
	else
	{
		float alphaMod = 0;

		if ( drop.pos[2] < 2.0f)
			alphaMod = (2.0f - drop.pos[2])*0.5f;

		glColor4f(1,1,1,1.0f - alphaMod);
		glPushMatrix();
		glTranslatef(drop.pos[0],drop.pos[1],drop.pos[2]);
		if (doBillBoards)
			sr.getViewFrustum().executeBillboard();
		else if (spinRain)
			glRotatef(lastRainTime*10.0f * rainSpeed*0.85f,0,1,0);

		if (spinRain)
			glRotatef(lastRainTime*10.0f * rainSpeed,0,0,1);

		if (1)
			dropList.execute();
		else
			buildDropList(true);
		glPopMatrix();
	}
}

void WeatherRenderer::drawPuddle ( puddle	&splash )
{
	glPushMatrix();
	glTranslatef(splash.pos[0],splash.pos[1],splash.pos[2]);

	float scale = fabs(splash.time * rainSpeed*0.035f*puddleSpeed);
	float lifeTime = splash.time/maxPuddleTime;

        glColor4f(puddleColor[0], puddleColor[1], puddleColor[2], 1.0f - lifeTime);

	glScalef(scale,scale,scale);
	if (1)
		puddleList.execute();
	else
		buildPuddleList(true);

	glPopMatrix();
}

void WeatherRenderer::addDrop ( rain & drop )
{
	int key = keyFromPos(drop.pos[0],drop.pos[1]);

	std::map<int,visibleChunk>::iterator itr = chunkMap.find(key);
	rainCount++;
	
	if (itr != chunkMap.end())
	{
		itr->second.drops.push_back(drop);
		return;
	}
	
	visibleChunk	chunk;

	setChunkFromDrop(chunk,drop);

	chunk.drops.push_back(drop);
	chunkMap[key] = chunk;
	cellCount++;
}

int WeatherRenderer::keyFromPos ( float x, float y )
{
	// todo, optimze this
	int keyX = (int)(x*keyFactor);
	int keyY = (int)(y*keyFactor);

	short	temp[2];

	temp[0] = (short)keyX;
	temp[1] = (short)keyY;

	return  *((int*)temp);
}

void WeatherRenderer::setChunkFromDrop ( visibleChunk &chunk, rain & drop )
{
	int keyX = (int)(drop.pos[0]*keyFactor);
	int keyY = (int)(drop.pos[1]*keyFactor);

	chunk.bboxMin[0] = keyX * gridSize;
	chunk.bboxMin[1] = keyY * gridSize;
	chunk.bboxMin[2] = rainStartZ > rainEndZ ?  rainEndZ : rainStartZ;
	chunk.bboxMax[0] = keyX * gridSize + gridSize;
	chunk.bboxMax[1] = keyY * gridSize + gridSize;
	chunk.bboxMax[2] = rainStartZ > rainEndZ ?  rainStartZ : rainEndZ;
}

bool WeatherRenderer::dbItemSet ( const char* name )
{
	if (!BZDB.isSet(name))
		return false;

	if (TextUtils::tolower(BZDB.get(name)) == "none")
		return false;

	return true;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

