/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
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

#include "common.h"
#include "OpenGLLight.h"
#include "ViewFrustum.h"
#include "RenderNode.h"

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

#include "AList.h"
BZF_DEFINE_ALIST(OpenGLLightList, OpenGLLight*);
BZF_DEFINE_ALIST(FlareLightList, FlareLight);

class SceneRenderer {
  public:
    enum ViewType {
			Normal,		// one view
			Stereo,		// binocular stereo
			Stacked,	// top-bottom stereo view
			ThreeChannel	// one wide view
    };

			SceneRenderer(MainWindow&);
			~SceneRenderer();

    static SceneRenderer*	getInstance() { return instance; }

    MainWindow&		getWindow() const;

    boolean		useABGR() const;
    boolean		useBlending() const;
    boolean		useSmoothing() const;
    boolean		useLighting() const;
    boolean		useTexture() const;
    boolean		useTextureReplace() const;
    boolean		useZBuffer() const;
    boolean		useStencil() const;
    int			useQuality() const;
    boolean		useShadows() const;
    boolean		useDithering() const;
    boolean		useDepthComplexity() const;
    boolean		useWireframe() const;
    boolean		useHiddenLine() const;
    boolean		isLastFrame() const;
    boolean		isSameFrame() const;
    ViewType		getViewType() const;
    int			getMaxLOD() const;

    void		setBlending(boolean on);
    void		setSmoothing(boolean on);
    void		setLighting(boolean on);
    void		setTexture(boolean on);
    void		setTextureReplace(boolean on);
    void		setZBuffer(boolean on);
    void		setZBufferSplit(boolean on);
    void		setQuality(int value);
    void		setShadows(boolean on);
    void		setDithering(boolean on);
    void		setDepthComplexity(boolean on);
    void		setWireframe(boolean on);
    void		setHiddenLine(boolean on);
    void		setDim(boolean on);
    void		setViewType(ViewType);
    void		setMaxLOD(int maxLOD);

    void		setExposed();

    void		getGroundUV(const float p[2], float uv[2]) const;

    boolean		getBlank() const;
    boolean		getInvert() const;
    void		setBlank(boolean blank = True);
    void		setInvert(boolean invert = True);

    const ViewFrustum&	getViewFrustum() const;
    ViewFrustum&	getViewFrustum();

    int			getNumLights() const;
    int			getNumAllLights() const;
    const OpenGLLight&	getLight(int index) const;
    void		enableLight(int index, boolean = True);
    void		clearLights();
    void		addLight(OpenGLLight&);
    void		addFlareLight(const float* pos, const float* color);

    void		setTimeOfDay(double julianDay);
    const GLfloat*	getSunColor() const;
    const GLfloat*	getSunScaledColor() const;
    GLfloat		getSunBrightness() const;
    void		enableSun(boolean = True);
    const GLfloat*	getCelestialTransform() const;
    float		getLatitude();
    float		getLongitude();
    void		setLatitude(float latitude);
    void		setLongitude(float longitude);

    SceneDatabase*	getSceneDatabase() const;
    void		setSceneDatabase(SceneDatabase*);

    BackgroundRenderer*	getBackground();
    void		setBackground(BackgroundRenderer*);

    const RenderNodeList& getShadowList() const;

    void		render(boolean lastFrame = True,
				boolean sameFrame = False,
				boolean fullWindow = False);
    boolean		testAndSetStyle(int& _style) const
				{ if (_style == style) return True;
				  _style = style; return False; }
    void		notifyStyleChange();
    void		addRenderNode(RenderNode* node, const OpenGLGState*);
    void		addShadowNode(RenderNode* node);
    boolean		getShowFlagHelp() const;
    void		setShowFlagHelp(boolean showFlagHelp);
    boolean		getScore() const;
    void		setScore(boolean showScore);

  private:
    // disallowed -- don't want to deal with potential state problems
			SceneRenderer(const SceneRenderer&);
    SceneRenderer&	operator=(const SceneRenderer&);

    void		doRender();

  private:
    MainWindow&		window;
    boolean		blank;
    boolean		invert;
    ViewFrustum		frustum;
    GLint		maxLights;
    GLint		reservedLights;
    OpenGLLightList	lights;
    OpenGLLight		theSun;
    boolean		sunOrMoonUp;
    GLfloat		sunDirection[3];	// or moon
    GLfloat		sunColor[3];
    GLfloat		sunScaledColor[3];
    GLfloat		celestialTransform[16];
    GLfloat		sunBrightness;
    float		latitude, longitude;
    SceneDatabase*	scene;
    BackgroundRenderer*	background;
    static const GLint	SunLight;

    boolean		abgr;
    boolean		useBlendingOn;
    boolean		useSmoothingOn;
    boolean		useLightingOn;
    boolean		useTextureOn;
    boolean		useTextureReplaceOn;
    int			useQualityValue;
    boolean		useShadowsOn;
    boolean		useDitheringOn;
    boolean		useDepthComplexityOn;
    boolean		useWireframeOn;
    boolean		useHiddenLineOn;
    boolean		useFogHack;
    boolean		useZBufferOn;
    boolean		useStencilOn;
    ViewType		viewType;
    int			maxLOD;
    RenderNodeList	shadowList;
    RenderNodeGStateList orderedList;
    boolean		inOrder;
    int			style;
    SceneIterator*	sceneIterator;
    int			depthRange;
    int			numDepthRanges;
    double		depthRangeSize;
    boolean		useDimming;
    boolean		canUseHiddenLine;
    boolean		exposed;
    boolean		lastFrame;
    boolean		sameFrame;
    FlareLightList	flareLightList;
    OpenGLGState	flareGState;
	  boolean		showFlagHelp;
    boolean		showScore;

    static SceneRenderer* instance;
};

//
// SceneRenderer
//

inline MainWindow&		SceneRenderer::getWindow() const
{
  return window;
}

inline boolean			SceneRenderer::getBlank() const
{
  return blank;
}

inline void			SceneRenderer::setBlank(boolean _blank)
{
  blank = _blank;
}

inline boolean			SceneRenderer::getInvert() const
{
  return invert;
}

inline void			SceneRenderer::setInvert(boolean _invert)
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

inline boolean			SceneRenderer::isLastFrame() const
{
  return lastFrame;
}

inline boolean			SceneRenderer::isSameFrame() const
{
  return sameFrame;
}

#endif // BZF_SCENE_RENDERER_H
