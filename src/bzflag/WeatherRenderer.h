/* bzflag
* Copyright (c) 1993 - 2007 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
* WeatherRenderer:
*	Encapsulates rendering of weather stuff (rain and clouds)
*
*/

#ifndef	BZF_WEATHER_RENDERER_H
#define	BZF_WEATHER_RENDERER_H

#include "common.h"

/* system headers */
#include <string>
#include <vector>
#include <map>

/* common interface headers */
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "SceneRenderer.h"

#include "OpenGLUtils.h"

class WeatherRenderer : public GLDisplayListCreator
{
public:
	WeatherRenderer();
	virtual ~WeatherRenderer();

	virtual void buildGeometry ( GLDisplayList displayList );

	// called once to setup the rain state, load lists and materials and stuff
	void init(void);

	// called each time the rain state needs to change, i.e. when the bzdb stuff changes
	void set(void);

	// called to update the rain simulation state.
	void update(void);

	// called to draw the rain for the current frame
	void draw(const SceneRenderer& sr);

protected:
	OpenGLGState				rainGState;
	OpenGLGState				texturedRainState;
	OpenGLGState				puddleState;
	std::string				rainSkin;
	std::vector<std::string>		rainTextures;
	float					rainColor[2][4];
	float					rainSize[2];
	int					rainDensity;
	float					rainSpeed;
	float					rainSpeedMod;
	float					rainSpread;
	bool					doPuddles;
	bool					doLineRain;
	bool					doBillBoards;
	bool					spinRain;
	bool					cullRoofTops;
	bool					roofPuddles;
	float					rainStartZ;
	float					rainEndZ;
	float					maxPuddleTime;
	float					puddleSpeed;
	float					puddleColor[4];

	GLDisplayList				dropList;
	GLDisplayList				puddleList;

public:
	typedef struct {
		float		pos[3];
		float		speed;
		float		roofTop;
		int		texture;
	} rain;

protected:
	std::vector<rain>	raindrops;

	typedef struct {
		float		pos[3];
		float		time;
		int		texture;
	} puddle;
	std::vector<puddle>	puddles;

	float			lastRainTime;

	bool updateDrop(std::vector<rain>::iterator &drop, float frameTime, std::vector<rain> &toAdd);
	bool updatePuddle(std::vector<puddle>::iterator &splash, float frameTime);

	void drawDrop(rain &drop, const SceneRenderer& sr);
	void drawPuddle(puddle &splash);

	// some kinda culling
	void addDrop(rain &drop);

	int keyFromPos(float x, float y);

	float			gridSize;
	float			keyFactor;

public:
	typedef struct {
	  float mins[3];
	  float maxs[3];
	} copyExtents;

protected:
	typedef struct  {
	  std::vector<rain>	drops;
	  copyExtents		bbox;
	} visibleChunk;

	std::map<int, visibleChunk>	chunkMap;

	void setChunkFromDrop(visibleChunk &chunk, rain &drop);

	bool dbItemSet(const char* name);

	int	rainCount;
	int	cellCount;
};

#endif // BZF_WEATHER_RENDERER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
