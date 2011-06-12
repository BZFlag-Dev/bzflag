/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
 *  Encapsulates rendering background stuff
 *
 * FIXME -- should be abstract base for background rendering
 */

#ifndef BZF_BACKGROUND_RENDERER_H
#define BZF_BACKGROUND_RENDERER_H

#include "common.h"

/* system headers */
#include <string>

/* common interface headers */
#include "bzfgl.h"
#include "ogl/OpenGLGState.h"
#include "bzflag/SceneRenderer.h"
#include "WeatherRenderer.h"

#include "ogl/OpenGLUtils.h"

class BackgroundRenderer : public GLDisplayListCreator {
  public:
    BackgroundRenderer(const SceneRenderer&);
    virtual ~BackgroundRenderer();

    virtual void buildGeometry(GLDisplayList displayList);

    void    setupGroundMaterials();
    void    setupSkybox();

    void    renderSky(SceneRenderer&, bool fullWindow, bool mirror);
    void    renderGround(SceneRenderer&, bool fullWindow);
    void    renderGroundEffects(SceneRenderer&, bool drawingMirror);
    void    renderEnvironment(SceneRenderer&, bool update);

    void    resize();

    const fvec3*  getSunDirection() const;
    void    setBlank(bool blank = true);
    void    setInvert(bool invert = true);
    void    setSimpleGround(bool simple = true);
    void    setCelestial(const SceneRenderer&,
                         const float sunDirection[3],
                         const float moonDirection[3]);
    void    addCloudDrift(float uDrift, float vDrift);
    void    notifyStyleChange();

    int     getTriangleCount() const;
    void    resetTriangleCount();

    void    multShadowMatrix() const;

  protected:
    void    drawSky(SceneRenderer&, bool mirror);
    void    drawSkybox();
    void    drawGround(void);
    void    drawGroundCentered(void);
    void    drawGroundGrid(SceneRenderer&);
    void    drawGroundShadows(SceneRenderer&);
    void    drawGroundReceivers(SceneRenderer&);
    void    drawAdvancedGroundReceivers(SceneRenderer&);
    void    drawMountains(void);


  private:
    BackgroundRenderer(const BackgroundRenderer&);
    BackgroundRenderer& operator=(const BackgroundRenderer&);

    void    resizeSky();

    void    setSkyColors();
    void    resetCloudList();
    void    makeCelestialLists(const SceneRenderer&);
    static void   bzdbCallback(const std::string&, void*);

  private:
    // rendering state
    bool    blank;
    bool    invert;
    bool    simpleGround;
    int     style;
    int     styleIndex;

    // stuff for ground
    OpenGLGState  groundGState[4];
    OpenGLGState  invGroundGState[4];
    int     groundTextureID;
    const float*  groundTextureMatrix;

    // stuff for grid
    float   gridSpacing;
    float   gridCount;
    OpenGLGState  gridGState;

    // stuff for ground receivers
    OpenGLGState  receiverGState;

    // stuff for mountains
    bool    mountainsAvailable;
    bool    mountainsVisible;
    int     numMountainTextures;
    int     mountainsMinWidth;
    OpenGLGState* mountainsGState;

    // stuff for clouds
    float   cloudDriftU, cloudDriftV;
    bool    cloudsAvailable;
    bool    cloudsVisible;
    OpenGLGState  cloudsGState;

    // weather
    WeatherRenderer weather;

    // stuff for sun shadows
    bool    doShadows;
    bool    shadowsVisible;
    OpenGLGState  sunShadowsGState;

    // celestial stuff
    bool    haveSkybox;
    GLenum    skyboxWrapMode;
    int     skyboxTexID[6];
    fvec4   skyboxColor[8];
    bool    doStars;
    bool    doSunset;
    fvec4   skyZenithColor;
    fvec4   skySunDirColor;
    fvec4   skyAntiSunDirColor;
    fvec4   skyCrossSunDirColor;
    fvec3   sunDirection;
    fvec3   moonDirection;
    float   sunAzimuth;
    float   sunsetTop;
    int     starGStateIndex;
    OpenGLGState  skyGState;
    OpenGLGState  sunGState;
    OpenGLGState  moonGState[2];
    OpenGLGState  starGState[2];

    // display lists
    GLDisplayList sunXFormList;
    GLDisplayList moonList;
    GLDisplayList starXFormList;
    GLDisplayList lowGroundList;
    GLDisplayList mediumGroundList;
    GLDisplayList cloudsList;
    std::vector<GLDisplayList> mountanLists;


    static fvec3    skyPyramid[5];
    static const float  cloudRepeats;

    static fvec4  rcvrGroundColor[4];
    static fvec4  rcvrGroundInvColor[4];
    static fvec4  groundColor[4];
    static fvec4  groundColorInv[4];
    static const fvec4  defaultGroundColor[4];
    static const fvec4  defaultGroundColorInv[4];
    static const fvec4  receiverColor;
    static const fvec4  receiverColorInv;

    int     triangleCount;

    SceneRenderer*       lastRenderer;

    void buildMountain(unsigned int index);
};

//
// BackgroundRenderer
//

inline void   BackgroundRenderer::setBlank(bool _blank) {
  blank = _blank;
}

inline void   BackgroundRenderer::setInvert(bool _invert) {
  invert = _invert;
}

inline void   BackgroundRenderer::setSimpleGround(bool _simple) {
  simpleGround = _simple;
}

inline int    BackgroundRenderer::getTriangleCount() const {
  return triangleCount;
}

inline void   BackgroundRenderer::resetTriangleCount() {
  triangleCount = 0;
}


#endif // BZF_BACKGROUND_RENDERER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
