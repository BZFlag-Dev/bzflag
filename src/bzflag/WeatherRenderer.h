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

/*
* WeatherRenderer:
*	Encapsulates rendering of weather stuff ( rain and clouds )
*
*/

#ifndef	BZF_WEATHER_RENDERER_H
#define	BZF_WEATHER_RENDERER_H

#include "common.h"

/* system headers */
#include <string>

/* common interface headers */
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "OpenGLDisplayList.h"
#include "SceneRenderer.h"

class WeatherRenderer {
public:
	WeatherRenderer();
	~WeatherRenderer();

	// called once to setup the rain state, load lists and materials and stuff
	void init ( void );

	// called each time the rain state needs to change, i.e. when the bzdb stuff changes
	void set ( void );

	// called to update the rain simulation state.
	void update ( void );

	// called to draw the rain for the current frame
	void draw ( const SceneRenderer& sr );

	// called when the GL lists need to be remade
	void rebuildContext ( void );
protected:
	OpenGLGState				rainGState;
	OpenGLGState				texturedRainState;
	OpenGLGState				puddleState;
	std::string					rainSkin;
	std::vector<std::string>	rainTextures;
	float						rainColor[2][4];
	float						rainSize[2];
	int							rainDensity;
	float						rainSpeed;
	float						rainSpeedMod;
	float						rainSpread;
	bool						doPuddles;
	bool						doLineRain;
	bool						doBillBoards;
	float						rainStartZ;
	float						rainEndZ;
	float						maxPuddleTime;
	float						puddleSpeed;
	float						puddleColor[4];
	OpenGLDisplayList			dropList;
	OpenGLDisplayList			puddleList;


	typedef struct {
		float    pos[3];
		float	 speed;
		int		 texture;
	}rain;
	std::vector<rain>			raindrops;

	typedef struct {
		float			pos[3];
		float			time;
		int				texture;
	}puddle;
	std::vector<puddle>			puddles;

	float						lastRainTime;

	void buildDropList ( void );
	void buildPuddleList ( void );

	bool updateDrop ( std::vector<rain>::iterator &drop, float frameTime );
	bool updatePuddle ( std::vector<puddle>::iterator &splash, float frameTime );
};

#endif // BZF_WEATHER_RENDERER_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

