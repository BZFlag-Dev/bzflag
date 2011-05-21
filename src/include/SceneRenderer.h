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

/* SceneRenderer:
 *  Encapsulates information about rendering a scene.
 */

#ifndef BZF_SCENE_RENDERER_H
#define BZF_SCENE_RENDERER_H

/* the common header */
#include "common.h"

/* interface header */
#include "Singleton.h"

/* system headers */
#include <vector>

/* common headers */
#include "OpenGLLight.h"
#include "ViewFrustum.h"
#include "RenderNode.h"
#include "Extents.h"
#include "vectors.h"


#define RENDERER (SceneRenderer::instance())


class SceneDatabase;
class SceneIterator;
class SceneNode;
class BackgroundRenderer;
class HUDRenderer;
class MainWindow;
class Extents;


#define _LOW_QUALITY 0
#define _MEDIUM_QUALITY 1
#define _HIGH_QUALITY 2
#define _EXPERIMENTAL_QUALITY 3


class SceneRenderer : public Singleton<SceneRenderer> {
  public:
    enum ViewType {
      Normal,   // one view
      Stereo,   // binocular stereo
      Stacked,    // top-bottom stereo view
      ThreeChannel, // one wide view
      Anaglyph,   // red-blue stereo
      Interlaced    // right/left interlaced stereo
    };

    enum SpecialMode {
      NoSpecial,
      WireFrame,
      HiddenLine,
      DepthComplexity,
      SpecialModeCount
    };

    enum ShadowMode {
      NoShadows      = 0,
      StippleShadows = 1,
      StencilShadows = 2,
      MappedShadows  = 3
    };

    enum RadarStyle {
      NormalRadar     = 0,
      EnhancedRadar   = 1,
      FastRadar       = 2,
      FastSortedRadar = 3
    };

    void    setWindow(MainWindow* _window);
    MainWindow& getWindow() const;

    SpecialMode getSpecialMode() const;

    bool    useStencil() const;
    int   useQuality() const;
    float   getPanelOpacity() const;
    int   getRadarSize() const;
    int   getMaxMotionFactor() const;
    bool    isLastFrame() const;
    bool    isSameFrame() const;
    ViewType  getViewType() const;

    void    setQuality(int value);
    void    setSpecialMode(SpecialMode mode);

    void    setSmoothing(bool on);
    void    setZBuffer(bool on);
    void    setPanelOpacity(float opacity);
    void    setRadarSize(int size);
    void    setMaxMotionFactor(int size);
    void    setDim(bool on);
    void    setViewType(ViewType);
    void    setRebuildTanks();


    void    setExposed();

    void    clearRadar(float opacity);

    void    getGroundUV(const fvec2& p, fvec2& uv) const;

    bool    getBlank() const;
    bool    getInvert() const;
    void    setBlank(bool blank = true);
    void    setInvert(bool invert = true);

    inline bool getDrawingMirror() const { return drawingMirror; }

    const ViewFrustum&  getViewFrustum() const;
    ViewFrustum&    getViewFrustum();

    int     getNumLights() const;
    int     getNumAllLights() const;
    const OpenGLLight&  getLight(int index) const;
    void      enableLight(int index, bool = true);
    void      clearLights();
    void      addLight(OpenGLLight&);

    // temporarily turn off non-applicable lights for big meshes
    void    disableLights(const Extents& extents);
    void    reenableLights();

    void    setupSun(); // setup sun lighting params
    void    enableSun(bool = true); // toggle light state

    void    setTimeOfDay(double julianDay);

    const fvec4&    getSunColor() const;
    const fvec4&    getSunScaledColor() const;
    float     getSunBrightness() const;
    const fvec3*    getSunDirection() const;
    const fvec4&    getAmbientColor() const;
    const float*    getCelestialTransform() const;

    SceneDatabase*  getSceneDatabase() const;
    void      setSceneDatabase(SceneDatabase*);

    const Extents*  getVisualExtents() const;
    float     getLengthPerPixel() const;

    int     getFrameTriangleCount() const;

    BackgroundRenderer* getBackground();
    void      setBackground(BackgroundRenderer*);

    const RenderNodeList& getShadowList() const;

    void      setupFog();
    inline bool   isFogActive() const { return fogActive; }
    inline const fvec4& getFogColor() const { return fogColor; }

    void    render(bool lastFrame = true,
                   bool sameFrame = false,
                   bool fullWindow = false);
    void    renderScene();

    void    notifyStyleChange();
    void    updateNodeStyles();
    void    addRenderNode(RenderNode* node, const OpenGLGState*);
    void    addShadowNode(RenderNode* node);

    int     getShadowPlanes(const fvec4** planes) const;

  protected:
    friend class Singleton<SceneRenderer>;

  private:
    // disallowed -- don't want to deal with potential state problems
    SceneRenderer();
    ~SceneRenderer();

    SceneRenderer(const SceneRenderer&);
    SceneRenderer&  operator=(const SceneRenderer&);

    void    setupBackgroundMaterials();

    void    getLights();
    void    getRenderNodes();

    void    drawMirror();
    void    doRender();
    void    renderDepthComplexity();
    void    renderPostDimming();

    void    setupShadowPlanes();

  private:
    MainWindow*   window;
    SpecialMode   specialMode;
    bool      blank;
    bool      invert;
    bool      mirror;
    bool      drawingMirror;
    bool      drawGround;
    bool      clearZbuffer;
    bool      mapFog;
    ViewFrustum   frustum;
    float     lengthPerPixel;
    int     maxLights;
    int     reservedLights;
    int     dynamicLights;
    int     lightsSize;
    int     lightsCount;
    OpenGLLight**   lights;
    OpenGLLight   theSun;
    bool      sunOrMoonUp;
    fvec3     sunDirection; // or moon
    fvec4     sunColor;
    fvec4     sunScaledColor;
    float     celestialTransform[4][4];
    float     sunBrightness;
    fvec4     ambientColor;
    SceneDatabase*  scene;
    BackgroundRenderer* background;
    int     triangleCount;
    static const int  SunLight;

    static const float dimDensity;
    static const fvec4 dimnessColor;
    static const fvec4 blindnessColor;
    float teleporterProximity;

    int   useQualityValue;
    float   panelOpacity;
    int   radarSize;
    int   maxMotionFactor;
    bool    useStencilOn;
    ViewType  viewType;
    bool    inOrder;
    bool    useDimming;
    bool    canUseHiddenLine;
    bool    exposed;
    bool    lastFrame;
    bool    sameFrame;
    bool    fullWindow;
    bool    needStyleUpdate;
    bool    rebuildTanks;

    bool    fogActive;
    fvec4   fogColor;

    fvec4   shadowPlanes[4];
    int   shadowPlaneCount;

    RenderNodeList  shadowList;
    RenderNodeGStateList  orderedList;
};


//
// SceneRenderer
//

inline MainWindow&    SceneRenderer::getWindow() const {
  return *window;
}

inline bool     SceneRenderer::getBlank() const {
  return blank;
}

inline void     SceneRenderer::setBlank(bool _blank) {
  blank = _blank;
}

inline bool     SceneRenderer::getInvert() const {
  return invert;
}

inline void     SceneRenderer::setInvert(bool _invert) {
  invert = _invert;
}

inline const ViewFrustum& SceneRenderer::getViewFrustum() const {
  return frustum;
}

inline ViewFrustum&   SceneRenderer::getViewFrustum() {
  return frustum;
}

inline float      SceneRenderer::getLengthPerPixel() const {
  return lengthPerPixel;
}

inline const OpenGLLight& SceneRenderer::getLight(int index) const {
  return *(lights[index]);
}

inline const fvec4&   SceneRenderer::getSunColor() const {
  return sunColor;
}

inline const fvec4&   SceneRenderer::getSunScaledColor() const {
  return sunScaledColor;
}

inline float      SceneRenderer::getSunBrightness() const {
  return sunBrightness;
}

inline const fvec4&     SceneRenderer::getAmbientColor() const {
  return ambientColor;
}

inline const float*   SceneRenderer::getCelestialTransform() const {
  return &celestialTransform[0][0];
}

inline SceneDatabase*   SceneRenderer::getSceneDatabase() const {
  return scene;
}

inline BackgroundRenderer*  SceneRenderer::getBackground() {
  return background;
}

inline bool     SceneRenderer::isLastFrame() const {
  return lastFrame;
}

inline bool     SceneRenderer::isSameFrame() const {
  return sameFrame;
}

inline int      SceneRenderer::useQuality() const {
  return useQualityValue;
}

inline void SceneRenderer::addRenderNode(RenderNode* node,
                                         const OpenGLGState* gstate) {
  if (inOrder || gstate->getNeedsSorting()) {
    // nodes will be drawn in the same order received
    orderedList.append(node, gstate);
  }
  else {
    // store node in gstate bucket
    gstate->addRenderNode(node);
  }
}

inline void SceneRenderer::addShadowNode(RenderNode* node) {
  shadowList.append(node);
}


#endif // BZF_SCENE_RENDERER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
