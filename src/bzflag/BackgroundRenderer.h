/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include "SceneRenderer.h"
#include "WeatherRenderer.h"

#include "OpenGLUtils.h"

class BackgroundRenderer : public GLDisplayListCreator
{
  public:
			BackgroundRenderer(const SceneRenderer&);
			virtual ~BackgroundRenderer();

    virtual void buildGeometry ( GLDisplayList displayList );

    void		setupGroundMaterials();
    void		setupSkybox();

    void		renderSky(SceneRenderer&, bool fullWindow, bool mirror);
    void		renderGround(SceneRenderer&, bool fullWindow);
    void		renderGroundEffects(SceneRenderer&, bool drawingMirror);
    void		renderEnvironment(SceneRenderer&, bool update);

    void		resize();

    const GLfloat*	getSunDirection() const;
    void		setBlank(bool blank = true);
    void		setInvert(bool invert = true);
    void		setSimpleGround(bool simple = true);
    void		setCelestial(const SceneRenderer&,
				     const float sunDirection[3],
				     const float moonDirection[3]);
    void		addCloudDrift(GLfloat uDrift, GLfloat vDrift);
    void		notifyStyleChange();

    int			getTriangleCount() const;
    void		resetTriangleCount();
  protected:
    void		drawSky(SceneRenderer&, bool mirror);
    void		drawSkybox();
    void		drawGround(void);
    void		drawGroundCentered(void);
    void		drawGroundGrid(SceneRenderer&);
    void		drawGroundShadows(SceneRenderer&);
    void		drawGroundReceivers(SceneRenderer&);
    void		drawAdvancedGroundReceivers(SceneRenderer&);
    void		drawMountains(void);


  private:
			BackgroundRenderer(const BackgroundRenderer&);
    BackgroundRenderer&	operator=(const BackgroundRenderer&);

    void		resizeSky();

    void		setSkyColors();
    void		resetCloudList();
    void		makeCelestialLists(const SceneRenderer&);
    static void		bzdbCallback(const std::string&, void*);

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
    int			groundTextureID;
    const GLfloat*	groundTextureMatrix;

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

    // stuff for clouds
    GLfloat		cloudDriftU, cloudDriftV;
    bool		cloudsAvailable;
    bool		cloudsVisible;
    OpenGLGState	cloudsGState;

    // weather
    WeatherRenderer	weather;

    // stuff for sun shadows
    bool		doShadows;
    bool		shadowsVisible;
    OpenGLGState	sunShadowsGState;

    // celestial stuff
    bool		haveSkybox;
    GLenum		skyboxWrapMode;
    int			skyboxTexID[6];
    GLfloat		skyboxColor[8][4];
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

    // display lists
    GLDisplayList	sunXFormList;
    GLDisplayList	moonList;
    GLDisplayList	starXFormList;
    GLDisplayList	lowGroundList;
    GLDisplayList	mediumGroundList;
    GLDisplayList	cloudsList;
    std::vector<GLDisplayList> mountanLists;


    static GLfloat		skyPyramid[5][3];
    static const GLfloat	cloudRepeats;

    static GLfloat		rcvrGroundColor[4][4];
    static GLfloat		rcvrGroundInvColor[4][4];
    static GLfloat		groundColor[4][4];
    static GLfloat		groundColorInv[4][4];
    static const GLfloat	defaultGroundColor[4][4];
    static const GLfloat	defaultGroundColorInv[4][4];
    static const GLfloat	receiverColor[3];
    static const GLfloat	receiverColorInv[3];

    int			triangleCount;

    SceneRenderer	      *lastRenderer;

    void buildMountan ( unsigned int index );
};

//
// BackgroundRenderer
//

inline void		BackgroundRenderer::setBlank(bool _blank)
{
  blank = _blank;
}

inline void		BackgroundRenderer::setInvert(bool _invert)
{
  invert = _invert;
}

inline void		BackgroundRenderer::setSimpleGround(bool _simple)
{
  simpleGround = _simple;
}

inline int		BackgroundRenderer::getTriangleCount() const
{
  return triangleCount;
}

inline void		BackgroundRenderer::resetTriangleCount()
{
  triangleCount = 0;
}


#endif // BZF_BACKGROUND_RENDERER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
