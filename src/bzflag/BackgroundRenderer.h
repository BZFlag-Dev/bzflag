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
 * BackgroundRenderer:
 *	Encapsulates rendering background stuff
 *
 * FIXME -- should be abstract base for background rendering
 */

#ifndef	BZF_BACKGROUND_RENDERER_H
#define	BZF_BACKGROUND_RENDERER_H

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
		float						puddleColor[3];
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


class BackgroundRenderer {
  public:
			BackgroundRenderer(const SceneRenderer&);
			~BackgroundRenderer();

    void		renderSky(SceneRenderer&, bool fullWindow);
    void		renderGround(SceneRenderer&, bool fullWindow);
    void		renderGroundEffects(SceneRenderer&);
    void		renderEnvironment(SceneRenderer&);

    void                resize();

    bool		getBlank() const;
    bool		getInvert() const;
    bool		getSimpleGround() const;
    const GLfloat*	getSunDirection() const;
    void		setBlank(bool blank = true);
    void		setInvert(bool invert = true);
    void		setSimpleGround(bool simple = true);
    void		setCelestial(const SceneRenderer&,
                                     const float sunDirection[3],
                                     const float moonDirection[3]);
    void		addCloudDrift(GLfloat uDrift, GLfloat vDrift);
    void		notifyStyleChange();

    std::string		userTextures[2];
  protected:
    void		drawSky(SceneRenderer&);
    void		drawGround(void);
    void		drawGroundGrid(SceneRenderer&);
    void		drawGroundShadows(SceneRenderer&);
    void		drawGroundReceivers(SceneRenderer&);
    void		drawMountains(void);


  private:
			BackgroundRenderer(const BackgroundRenderer&);
    BackgroundRenderer&	operator=(const BackgroundRenderer&);

    void                resizeSky();
    void		doInitDisplayLists();
    static void		initDisplayLists(void*);

  private:
    // rendering state
    bool		blank;
    bool		invert;
    bool		simpleGround;
    int			style;
    int			styleIndex;

    // stuff for ground
    OpenGLGState	groundGState[4];
    OpenGLGState	invGroundGState[4];
    OpenGLDisplayList	simpleGroundList[4];

    // stuff for grid
    GLfloat		gridSpacing;
    GLfloat		gridCount;
    OpenGLGState	gridGState;

    // stuff for ground receivers
    OpenGLGState	receiverGState;

    // stuff for mountains
    bool		mountainsAvailable;
    bool		mountainsVisible;
    int			numMountainTextures;
    int			mountainsMinWidth;
    OpenGLGState*	mountainsGState;
    OpenGLDisplayList*	mountainsList;

    // stuff for clouds
    GLfloat		cloudDriftU, cloudDriftV;
    bool		cloudsAvailable;
    bool		cloudsVisible;
    OpenGLGState	cloudsGState;
    OpenGLDisplayList	cloudsList;

	// stuff for rain
	OpenGLGState				rainGState;
	OpenGLGState				texturedRainState;
	OpenGLGState				puddleState;
	std::vector<std::string>	rainTextures;
	float						rainColor[4][2];
	float						rainSize[2];
	int							rainDensity;
	float						rainSpeed;
	float						rainSpeedMod;
	float						rainSpread;
	bool						doBillboards;
	typedef struct {
		float    pos[3];
		float	 speed;
		int		 texture;
	}rain;
	std::vector<rain>			raindrops;
	bool									doPuddles;
	typedef struct {
		float			pos[3];
		float			time;
		int				texture;
	}puddle;
	std::vector<puddle>			puddles;
	float									lastRainTime;

    // stuff for sun shadows
    bool		doShadows;
    bool		shadowsVisible;
    OpenGLGState	sunShadowsGState;

    // celestial stuff
    bool		doStars;
    bool		doSunset;
    GLfloat		skyZenithColor[3];
    GLfloat		skySunDirColor[3];
    GLfloat		skyAntiSunDirColor[3];
    GLfloat		skyCrossSunDirColor[3];
    float		sunDirection[3];
    float		moonDirection[3];
    float		sunAzimuth;
    float		sunsetTop;
    int			starGStateIndex;
    OpenGLGState	skyGState;
    OpenGLGState	sunGState;
    OpenGLGState	moonGState[2];
    OpenGLGState	starGState[2];
    OpenGLDisplayList	sunList;
    OpenGLDisplayList	sunXFormList;
    OpenGLDisplayList	moonList;
    OpenGLDisplayList	starList;
    OpenGLDisplayList	starXFormList;

    static GLfloat		skyPyramid[5][3];
    static const GLfloat	cloudRepeats;

    static const GLfloat	groundColor[4][3];
    static const GLfloat	groundColorInv[4][3];
    static const GLfloat	receiverColor[3];
    static const GLfloat	receiverColorInv[3];
};

//
// BackgroundRenderer
//

inline bool		BackgroundRenderer::getBlank() const
{
  return blank;
}

inline void		BackgroundRenderer::setBlank(bool _blank)
{
  blank = _blank;
}

inline bool		BackgroundRenderer::getInvert() const
{
  return invert;
}

inline void		BackgroundRenderer::setInvert(bool _invert)
{
  invert = _invert;
}

inline bool		BackgroundRenderer::getSimpleGround() const
{
  return simpleGround;
}

inline void		BackgroundRenderer::setSimpleGround(bool _simple)
{
  simpleGround = _simple;
}


#endif // BZF_BACKGROUND_RENDERER_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

