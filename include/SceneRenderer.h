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

#define RENDERER (SceneRenderer::instance())

class SceneDatabase;
class SceneIterator;
class SceneNode;
class BackgroundRenderer;
class HUDRenderer;
class MainWindow;


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
    Anaglyph		// red-blue stereo
  };

  void		setWindow(MainWindow* _window);
  MainWindow&	getWindow() const;

  bool		useABGR() const;
  bool		useStencil() const;
  int			useQuality() const;
  bool		useDepthComplexity() const;
  bool		useWireframe() const;
  bool		useHiddenLine() const;
  float		getPanelOpacity() const;
  int			getRadarSize() const;
  int			getMaxMotionFactor() const;
  bool		isLastFrame() const;
  bool		isSameFrame() const;
  ViewType		getViewType() const;

  void		setSmoothing(bool on);
  void		setZBuffer(bool on);
  void		setZBufferSplit(bool on);
  void		setQuality(int value);
  void		setDepthComplexity(bool on);
  void		setWireframe(bool on);
  void		setHiddenLine(bool on);
  void		setPanelOpacity(float opacity);
  void		setRadarSize(int size);
  void		setMaxMotionFactor(int size);
  void		setDim(bool on);
  void		setViewType(ViewType);

  void		setExposed();

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

  void		setTimeOfDay(double julianDay);
  const GLfloat*	getSunColor() const;
  const GLfloat*	getSunScaledColor() const;
  GLfloat		getSunBrightness() const;
  void		enableSun(bool = true);
  const GLfloat*	getCelestialTransform() const;

  SceneDatabase*	getSceneDatabase() const;
  void		setSceneDatabase(SceneDatabase*);

  BackgroundRenderer*	getBackground();
  void		setBackground(BackgroundRenderer*);

  const RenderNodeList& getShadowList() const;

  void		render(bool lastFrame = true,
		       bool sameFrame = false,
		       bool fullWindow = false);
  bool		testAndSetStyle(int& _style) const
  { if (_style == style) return true;
  _style = style; return false; }
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

  void		doRender();

  MainWindow*		window;
  bool		blank;
  bool		invert;
  ViewFrustum		frustum;
  GLint		maxLights;
  GLint		reservedLights;
  std::vector<OpenGLLight*>	lights;
  OpenGLLight		theSun;
  bool		sunOrMoonUp;
  GLfloat		sunDirection[3];	// or moon
  GLfloat		sunColor[3];
  GLfloat		sunScaledColor[3];
  GLfloat		celestialTransform[16];
  GLfloat		sunBrightness;
  SceneDatabase*	scene;
  BackgroundRenderer*	background;
  static const GLint	SunLight;

  bool		abgr;
  int			useQualityValue;
  bool		useDepthComplexityOn;
  bool		useWireframeOn;
  bool		useHiddenLineOn;
  float		panelOpacity;
  int			radarSize;
  int			maxMotionFactor;
  bool		useFogHack;
  bool		useStencilOn;
  ViewType		viewType;
  RenderNodeList	shadowList;
  RenderNodeGStateList orderedList;
  bool		inOrder;
  int			style;
  SceneIterator*	sceneIterator;
  int			depthRange;
  int			numDepthRanges;
  double		depthRangeSize;
  bool		useDimming;
  bool		canUseHiddenLine;
  bool		exposed;
  bool		lastFrame;
  bool		sameFrame;
  std::vector<FlareLight>	flareLightList;

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

inline const GLfloat*		SceneRenderer::getCelestialTransform() const
{
  return celestialTransform;
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

#endif // BZF_SCENE_RENDERER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
