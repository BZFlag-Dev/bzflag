/* bzflag
 * Copyright (c) 1993-2012 Tim Riker
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
 *	Encapsulates information about rendering a scene.
 */

#ifndef	BZF_SCENE_RENDERER_H
#define	BZF_SCENE_RENDERER_H

#if defined(_MSC_VER)
#pragma warning(disable: 4786)
#endif

/* the common header */
#include "common.h"

/* interface headers */
#include "Singleton.h"

/* system interface headers */
#include <vector>

/* common interface headers */
#include "OpenGLLight.h"
#include "ViewFrustum.h"
#include "RenderNode.h"
#include "vectors.h"

#define RENDERER (SceneRenderer::instance())

class SceneDatabase;
class SceneIterator;
class SceneNode;
class BackgroundRenderer;
class HUDRenderer;
class MainWindow;
class Extents;

class FlareLight {
public:
  FlareLight(const float* pos, const float* color);
  ~FlareLight();

public:
  float		pos[3];
  float		color[3];
};


class SceneRenderer : public Singleton<SceneRenderer>
{
public:
  enum ViewType {
    Normal,		// one view
    Stereo,		// binocular stereo
    Stacked,		// top-bottom stereo view
    ThreeChannel,	// one wide view
    Anaglyph,		// red-blue stereo
    Interlaced		// right/left interlaced stereo
  };

  void		setWindow(MainWindow* _window);
  MainWindow&	getWindow() const;

  bool		useABGR() const;
  bool		useStencil() const;
  int		useQuality() const;
  bool		useDepthComplexity() const;
  bool		useWireframe() const;
  bool		useHiddenLine() const;
  float		getPanelOpacity() const;
  float         getRadarOpacity() const;
  bool          isPanelOpaque() const;
  int		getRadarSize() const;
  int		getMaxMotionFactor() const;
  bool		isLastFrame() const;
  bool		isSameFrame() const;
  ViewType	getViewType() const;

  void		setSmoothing(bool on);
  void		setZBuffer(bool on);
  void		setZBufferSplit(bool on);
  void		setQuality(int value);
  void		setDepthComplexity(bool on);
  void		setWireframe(bool on);
  void		setHiddenLine(bool on);
  void		setPanelOpacity(float opacity);
  void          setRadarOpacity(float opacity);
  void		setRadarSize(int size);
  void		setMaxMotionFactor(int size);
  void		setDim(bool on);
  void		setViewType(ViewType);
  void		setRebuildTanks();

  void		setExposed();

  void		clearRadar(float opacity);

  void		getGroundUV(const float p[2], float uv[2]) const;

  bool		getBlank() const;
  bool		getInvert() const;
  void		setBlank(bool blank = true);
  void		setInvert(bool invert = true);

  const ViewFrustum&	getViewFrustum() const;
  ViewFrustum&	getViewFrustum();

  int			getNumLights() const;
  int			getNumAllLights() const;
  const OpenGLLight&	getLight(int index) const;
  void		enableLight(int index, bool = true);
  void		clearLights();
  void		addLight(OpenGLLight&);
  void		addFlareLight(const float* pos, const float* color);

  // temporarily turn off non-applicable lights for big meshes
  void		disableLights(const float mins[3], const float maxs[3]);
  void		reenableLights();

  void		setupSun(); // setup sun lighting params
  void		enableSun(bool = true); // toggle light state

  void		setTimeOfDay(double julianDay);

  const GLfloat*	getSunColor() const;
  const GLfloat*	getSunScaledColor() const;
  GLfloat		getSunBrightness() const;
  const GLfloat*	getSunDirection() const;
  const GLfloat*	getAmbientColor() const;
  const GLfloat*	getCelestialTransform() const;

  SceneDatabase*	getSceneDatabase() const;
  void			setSceneDatabase(SceneDatabase*);

  const Extents*	getVisualExtents() const;
  float			getLengthPerPixel() const;

  int			getFrameTriangleCount() const;

  BackgroundRenderer*	getBackground();
  void			setBackground(BackgroundRenderer*);

  const RenderNodeList& getShadowList() const;

  bool		isFogActive() const { return fogActive; }
  void			setFogActive(bool b) { fogActive = b; }

  const fvec4&	getFogColor() const { return fogColor; }
  void	setFogColor( float *color) { fogColor = color; }


  void		render(bool lastFrame = true,
		       bool sameFrame = false,
		       bool fullWindow = false);
  void		renderScene(bool lastFrame = true,
			    bool sameFrame = false,
			    bool fullWindow = false);
  void		notifyStyleChange();
  void		addRenderNode(RenderNode* node, const OpenGLGState*);
  void		addShadowNode(RenderNode* node);

protected:
  friend class Singleton<SceneRenderer>;

private:
  // disallowed -- don't want to deal with potential state problems
  SceneRenderer();
  ~SceneRenderer();

  SceneRenderer(const SceneRenderer&);
  SceneRenderer&	operator=(const SceneRenderer&);

  void		setupBackgroundMaterials();

  void		getLights();
  void		getRenderNodes();

  void		doRender();
  void		renderDepthComplexity();
  void		renderDimming();

  MainWindow*		window;
  bool			blank;
  bool			invert;
  bool			mirror;
  bool			drawGround;
  bool			clearZbuffer;
  ViewFrustum		frustum;
  float			lengthPerPixel;
  GLint			maxLights;
  GLint			reservedLights;
  GLint			dynamicLights;
  int			lightsSize;
  int			lightsCount;
  OpenGLLight**		lights;
  OpenGLLight		theSun;
  bool			sunOrMoonUp;
  GLfloat		sunDirection[3];	// or moon
  GLfloat		sunColor[4];
  GLfloat		sunScaledColor[4];
  GLfloat		celestialTransform[4][4];
  GLfloat		sunBrightness;
  GLfloat		ambientColor[4];
  SceneDatabase*	scene;
  BackgroundRenderer*	background;
  int			triangleCount;
  static const GLint	SunLight;

  static const float dimDensity;
  static const GLfloat dimnessColor[4];
  static const GLfloat blindnessColor[4];
  float teleporterProximity;

  bool		abgr;
  int		useQualityValue;
  bool		useDepthComplexityOn;
  bool		useWireframeOn;
  bool		useHiddenLineOn;
  float		panelOpacity;
  float         radarOpacity;
  int		radarSize;
  int		maxMotionFactor;
  bool		useStencilOn;
  ViewType	viewType;
  bool		inOrder;
  int		depthRange;
  int		numDepthRanges;
  double	depthRangeSize;
  bool		useDimming;
  bool		canUseHiddenLine;
  bool		exposed;
  bool		lastFrame;
  bool		sameFrame;
  bool		needStyleUpdate;
  bool		rebuildTanks;
  bool		fogActive;
  fvec4		fogColor;

  std::vector<FlareLight>	flareLightList;
  RenderNodeList		shadowList;
  RenderNodeGStateList		orderedList;
};


//
// SceneRenderer
//

inline MainWindow&		SceneRenderer::getWindow() const
{
  return *window;
}

inline bool			SceneRenderer::getBlank() const
{
  return blank;
}

inline void			SceneRenderer::setBlank(bool _blank)
{
  blank = _blank;
}

inline bool			SceneRenderer::getInvert() const
{
  return invert;
}

inline void			SceneRenderer::setInvert(bool _invert)
{
  invert = _invert;
}

inline const ViewFrustum&	SceneRenderer::getViewFrustum() const
{
  return frustum;
}

inline ViewFrustum&		SceneRenderer::getViewFrustum()
{
  return frustum;
}

inline float			SceneRenderer::getLengthPerPixel() const
{
  return lengthPerPixel;
}

inline const OpenGLLight&	SceneRenderer::getLight(int index) const
{
  return *(lights[index]);
}

inline const GLfloat*		SceneRenderer::getSunColor() const
{
  return sunColor;
}

inline const GLfloat*		SceneRenderer::getSunScaledColor() const
{
  return sunScaledColor;
}

inline GLfloat			SceneRenderer::getSunBrightness() const
{
  return sunBrightness;
}

inline const GLfloat* SceneRenderer::getAmbientColor() const
{
  return ambientColor;
}

inline const GLfloat*		SceneRenderer::getCelestialTransform() const
{
  return &celestialTransform[0][0];
}

inline SceneDatabase*		SceneRenderer::getSceneDatabase() const
{
  return scene;
}

inline BackgroundRenderer*	SceneRenderer::getBackground()
{
  return background;
}

inline bool			SceneRenderer::isLastFrame() const
{
  return lastFrame;
}

inline bool			SceneRenderer::isSameFrame() const
{
  return sameFrame;
}

inline int			SceneRenderer::useQuality() const
{
  return useQualityValue;
}

inline void SceneRenderer::addRenderNode(RenderNode* node,
					 const OpenGLGState* gstate)
{
  if (inOrder || gstate->getNeedsSorting()) {
    // nodes will be drawn in the same order received
    orderedList.append(node, gstate);
  } else {
    // store node in gstate bucket
    gstate->addRenderNode(node);
  }
}

inline void SceneRenderer::addShadowNode(RenderNode* node)
{
  shadowList.append(node);
}


#endif // BZF_SCENE_RENDERER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
