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

/*
 * BackgroundRenderer:
 *	Encapsulates rendering background stuff
 *
 * FIXME -- should be abstract base for background rendering
 */

#ifndef	BZF_BACKGROUND_RENDERER_H
#define	BZF_BACKGROUND_RENDERER_H

#include "bzfgl.h"
#include "common.h"
#include "OpenGLGState.h"
#include "OpenGLDisplayList.h"

class SceneRenderer;

class BackgroundRenderer {
  public:
			BackgroundRenderer(const SceneRenderer&);
			~BackgroundRenderer();

    void		renderSkyAndGround(SceneRenderer&, boolean fullWindow);
    void		render(SceneRenderer&);

    boolean		getBlank() const;
    boolean		getInvert() const;
    boolean		getSimpleGround() const;
    void		setBlank(boolean blank = True);
    void		setInvert(boolean invert = True);
    void		setSimpleGround(boolean simple = True);
    void		setCelestial(const SceneRenderer&,
					const float sunDirection[3],
					const float moonDirection[3]);
    void		addCloudDrift(GLfloat uDrift, GLfloat vDrift);

  protected:
    void		drawSky(SceneRenderer&);
    void		drawGround(SceneRenderer&);
    void		drawGroundGrid(SceneRenderer&);
    void		drawTeamBases(SceneRenderer&);
    void		drawGroundShadows(SceneRenderer&);
    void		drawGroundReceivers(SceneRenderer&);
    void		drawMountains(SceneRenderer&);

    void		notifyStyleChange(SceneRenderer&);

  private:
			BackgroundRenderer(const BackgroundRenderer&);
    BackgroundRenderer&	operator=(const BackgroundRenderer&);

    void		doInitDisplayLists();
    static void		initDisplayLists(void*);

  private:
    // rendering state
    boolean		blank;
    boolean		invert;
    boolean		simpleGround;
    int			style;
    int			styleIndex;

    // stuff for ground
    OpenGLGState	groundGState[4];
    OpenGLDisplayList	groundList[4];
    OpenGLDisplayList	simpleGroundList[4];

    // stuff for grid
    GLfloat		gridSpacing;
    GLfloat		gridCount;
    OpenGLGState	gridGState;

    // stuff for team bases
    boolean		doTeamBases;
    OpenGLGState	teamBasesGState;
    OpenGLDisplayList	teamBasesList;

    // stuff for ground receivers
    OpenGLGState	receiverGState;

    // stuff for mountains
    boolean		mountainsAvailable;
    boolean		mountainsVisible;
    int			numMountainTextures;
    int			mountainsMinWidth;
    OpenGLGState*	mountainsGState;
    OpenGLDisplayList*	mountainsList;

    // stuff for clouds
    GLfloat		cloudDriftU, cloudDriftV;
    boolean		cloudsAvailable;
    boolean		cloudsVisible;
    OpenGLGState	cloudsGState;
    OpenGLDisplayList	cloudsList;

    // stuff for sun shadows
    boolean		doShadows;
    boolean		shadowsVisible;
    OpenGLGState	sunShadowsGState;

    // celestial stuff
    boolean		doStars;
    boolean		doSunset;
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

inline boolean		BackgroundRenderer::getBlank() const
{
  return blank;
}

inline void		BackgroundRenderer::setBlank(boolean _blank)
{
  blank = _blank;
}

inline boolean		BackgroundRenderer::getInvert() const
{
  return invert;
}

inline void		BackgroundRenderer::setInvert(boolean _invert)
{
  invert = _invert;
}

inline boolean		BackgroundRenderer::getSimpleGround() const
{
  return simpleGround;
}

inline void		BackgroundRenderer::setSimpleGround(boolean _simple)
{
  simpleGround = _simple;
}

#endif // BZF_BACKGROUND_RENDERER_H
// ex: shiftwidth=2 tabstop=8
