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

/* FIXME -- ugh.. the class header for this file is listed as a public header
 * and is used by other libs, yet this cxx is here.. bad bad bad.  need to
 * decouple this file from the bzflag front-end specific sources so that it
 * may be moved elsewhere.
 */

#include "common.h"

/* system implementation headers */
#include <string.h>

/* common implementation headers */
#include "bzfgl.h"
#include "SceneRenderer.h"
#include "SceneDatabase.h"
#include "SceneNode.h"
#include "MainWindow.h"
#include "OpenGLGState.h"
#include "RenderNode.h"
#include "BzfWindow.h"
#include "TankSceneNode.h"
#include "StateDatabase.h"
#include "TextUtils.h"
#include "BZDBCache.h"

/* FIXME - local implementation dependancies */
#include "BackgroundRenderer.h"
#include "HUDRenderer.h"
#include "LocalPlayer.h"
#include "daylight.h"
#include "World.h"


const GLint		SceneRenderer::SunLight = 0;	// also for the moon

#ifdef GL_ABGR_EXT
static int		strrncmp(const char* s1, const char* s2, int num)
{
  int len1 = strlen(s1) - 1;
  int len2 = strlen(s2) - 1;
  for (; len1 >= 0 && len2 >= 0 && num > 0; len1--, len2--, num--) {
    const int d = (int)s1[len1] - (int)s2[len2];
    if (d != 0) return d;
  }
  return 0;
}
#endif

//
// FlareLight
//

FlareLight::FlareLight(const float* _pos, const float* _color)
{
  pos[0] = _pos[0];
  pos[1] = _pos[1];
  pos[2] = _pos[2];
  color[0] = _color[0];
  color[1] = _color[1];
  color[2] = _color[2];
}

FlareLight::~FlareLight()
{
  // do nothing
}

//
// SceneRenderer
//

/* initialize the singleton */
template <>
SceneRenderer* Singleton<SceneRenderer>::_instance = (SceneRenderer*)0;

SceneRenderer::SceneRenderer() :
				window(NULL),
				blank(false),
				invert(false),
				sunBrightness(1.0f),
				scene(NULL),
				background(NULL),
				abgr(false),
				useQualityValue(2),
				useDepthComplexityOn(false),
				useCullingTreeOn(false),
				useCollisionTreeOn(false),
				useWireframeOn(false),
				useHiddenLineOn(false),
				panelOpacity(0.3f),
				radarSize(4),
				maxMotionFactor(5),
				useFogHack(false),
				viewType(Normal),
				inOrder(false),
				style(0),
				sceneIterator(NULL),
				depthRange(0),
				numDepthRanges(1),
				depthRangeSize(1.0),
				useDimming(false),
				canUseHiddenLine(false),
				exposed(true),
				lastFrame(true),
				sameFrame(false)
{
}

void SceneRenderer::setWindow(MainWindow* _window) {
  window = _window;

  // get visual info
  window->getWindow()->makeCurrent();
  GLint bits;
  if (!BZDB.isSet("zbuffer")) {
    glGetIntegerv(GL_DEPTH_BITS, &bits);
    BZDB.set("zbuffer", (bits > 0) ? "1" : "0");
  }
  glGetIntegerv(GL_STENCIL_BITS, &bits);
  useStencilOn = (bits > 0);

  // see if abgr extention is available and system is known to be
  // faster with abgr.
  const char* vendor = (const char*)glGetString(GL_VENDOR);
  const char* renderer = (const char*)glGetString(GL_RENDERER);
  const char* version = (const char*)glGetString(GL_VERSION);
  const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
  (void)vendor; (void)renderer; (void)version; (void)extensions; // silence g++
#ifdef GL_ABGR_EXT
  if ((extensions != NULL && strstr(extensions, "GL_EXT_abgr") != NULL) &&
      (vendor != NULL && strcmp(vendor, "SGI") == 0)) {
    // old hardware is faster with ABGR.  new hardware isn't.
    if ((renderer != NULL) &&
	(strncmp(renderer, "GR1", 3) == 0 ||
	 strncmp(renderer, "VGX", 3) == 0 ||
	 strncmp(renderer, "LIGHT", 5) == 0 ||
	 strrncmp(renderer, "-XS", 3) == 0 ||
	 strrncmp(renderer, "-XSM", 4) == 0 ||
	 strrncmp(renderer, "-XS24", 5) == 0 ||
	 strrncmp(renderer, "-XS24-Z", 7) == 0 ||
	 strrncmp(renderer, "-XZ", 3) == 0 ||
	 strrncmp(renderer, "-Elan", 5) == 0 ||
	 strrncmp(renderer, "-Extreme", 8) == 0))
      abgr = true;
  }
#endif

  // can only do hidden line if polygon offset is available
#if defined(GL_VERSION_1_1)
  canUseHiddenLine = true;
#elif defined(GL_EXT_polygon_offset)
  canUseHiddenLine = (extensions != NULL && strstr(extensions, "GL_EXT_polygon_offset") != NULL);
#endif

  // check if we're running OpenGL 1.1.  if so we'll use the fog hack
  // to fade the screen;  otherwise fall back on a full screen blended
  // polygon.
  if (version != NULL && strncmp(version, "1.1", 3) == 0)
    useFogHack = true;

  // prepare context with stuff that'll never change
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
  glGetIntegerv(GL_MAX_LIGHTS, &maxLights);
  reservedLights = 1;			// only one light between sun and moon
  maxLights -= reservedLights;		// can't use the reserved lights

  // prepare sun
  setTimeOfDay(unixEpoch);

/* FIXME
  // load flare light texture
  OpenGLGStateBuilder builder;
  builder.setTexture(tm.getTextureID("flare", OpenGLTexture::Max));
  flareGState = builder.getState();
*/

  // force nodes to update their styles
  notifyStyleChange();
}

SceneRenderer::~SceneRenderer()
{
  // free scene iterator first
  delete sceneIterator;

  // then free database
  delete scene;
}

bool			SceneRenderer::useABGR() const
{
  return abgr;
}

bool			SceneRenderer::useStencil() const
{
  return useStencilOn;
}

int			SceneRenderer::useQuality() const
{
  return useQualityValue;
}

SceneRenderer::ViewType	SceneRenderer::getViewType() const
{
  return viewType;
}

void			SceneRenderer::setZBufferSplit(bool on)
{
  if (!on) {
    if (numDepthRanges != 1) {
      numDepthRanges = 1;
      depthRangeSize = 1.0;
      glDepthRange(0.0, 1.0);
    }
  }
  else {
    GLint bits;
    glGetIntegerv(GL_DEPTH_BITS, &bits);
    if (bits > 18) {
      // number of independent slices to split depth buffer into
      numDepthRanges = 1 << (bits - 18);

      // size of a single range
      depthRangeSize = 1.0 / (double)numDepthRanges;
    }
    else {
      numDepthRanges = 1;
      depthRangeSize = 1.0;
    }
  }
}

void			SceneRenderer::setQuality(int value)
{
  if (value < 0) value = 0;
  else if (value > BZDB.eval("maxQuality")) value = (int)BZDB.eval("maxQuality");
  useQualityValue = value;
  notifyStyleChange();
  if (useQualityValue >= 2) {
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
  }
  else {
    glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
  }
  if (useQualityValue == 3)
    TankSceneNode::setMaxLOD(-1);
  else if (useQualityValue >= 1)
    TankSceneNode::setMaxLOD(3);
  else
    TankSceneNode::setMaxLOD(2);

  if (useQualityValue == 3)
    BZDB.set("flagChunks","32");
  else if (useQualityValue >= 2)
    BZDB.set("flagChunks","12");
  else
    BZDB.set("flagChunks","8");

  if (useQualityValue == 3)
    BZDB.set("moonSegments","64");
  else if (useQualityValue >= 2)
    BZDB.set("moonSegments","24");
  else
    BZDB.set("moonSegments","12");

  BZDB.set("useQuality",string_util::format("%d",value));
}

bool			SceneRenderer::useDepthComplexity() const
{
  return useDepthComplexityOn;
}

bool			SceneRenderer::useCullingTree() const
{
  return useCullingTreeOn;
}

bool			SceneRenderer::useCollisionTree() const
{
  return useCollisionTreeOn;
}

void			SceneRenderer::setDepthComplexity(bool on)
{
  if (on) {
    GLint bits;
    glGetIntegerv(GL_STENCIL_BITS, &bits);
    if (bits < 3) return;
  }
  useDepthComplexityOn = on;
}

void			SceneRenderer::setCullingTree(bool on)
{
  useCullingTreeOn = on;
}

void			SceneRenderer::setCollisionTree(bool on)
{
  useCollisionTreeOn = on;
}


void			SceneRenderer::setWireframe(bool on)
{
  useWireframeOn = on;
}

bool			SceneRenderer::useWireframe() const
{
  return useWireframeOn;
}

void			SceneRenderer::setHiddenLine(bool on)
{
  useHiddenLineOn = on && BZDB.isTrue("zbuffer") && canUseHiddenLine;
  if (!useHiddenLineOn) { depthRange = 0; return; }
#if defined(GL_VERSION_1_1)
  glPolygonOffset(1.0f, 2.0f);
#elif defined(GL_EXT_polygon_offset)
  glPolygonOffsetEXT(1.0f, 0.000004f);
#endif
}

bool			SceneRenderer::useHiddenLine() const
{
  return useHiddenLineOn;
}

void			SceneRenderer::setPanelOpacity(float opacity)
{
  bool needtoresize = opacity == 1.0f || panelOpacity == 1.0f;

  panelOpacity = opacity;
  notifyStyleChange();
  if (needtoresize) {
    if (window) {
      window->setFullView(panelOpacity < 1.0f);
      window->getWindow()->callResizeCallbacks();
    }
  }
}

float			SceneRenderer::getPanelOpacity() const
{
  return panelOpacity;
}

void			SceneRenderer::setRadarSize(int size)
{
  radarSize = size;
  notifyStyleChange();
  if (window) {
    window->getWindow()->callResizeCallbacks();
  }
}

int			SceneRenderer::getRadarSize() const
{
  return radarSize;
}


void			SceneRenderer::setMaxMotionFactor(int factor)
{
  maxMotionFactor = factor;
  notifyStyleChange();
  if (window) {
    window->getWindow()->callResizeCallbacks();
  }
}

int			SceneRenderer::getMaxMotionFactor() const
{
  return maxMotionFactor;
}

void			SceneRenderer::setDim(bool on)
{
  useDimming = on;
}

void			SceneRenderer::setViewType(ViewType _viewType)
{
  viewType = _viewType;
}

void			SceneRenderer::setExposed()
{
  exposed = true;
}

void			SceneRenderer::setSceneDatabase(SceneDatabase* db)
{
  // free scene iterator first
  delete sceneIterator;

  // then free database
  delete scene;

  scene = db;
  if (scene) {
    sceneIterator = scene->getRenderIterator();
    inOrder = scene->isOrdered();
    if (sceneIterator) {
      sceneIterator->resetFrustum(&frustum);
      sceneIterator->reset();
      SceneNode* node;
      while ((node = sceneIterator->getNext()) != NULL) {
	node->octreeState = SceneNode::OctreeCulled;
      }
    }
  }
  else {
    sceneIterator = NULL;
    inOrder = false;
  }
}

void			SceneRenderer::setBackground(BackgroundRenderer* br)
{
  background = br;
}

void			SceneRenderer::getGroundUV(const float p[2],
							float uv[2]) const
{
  float repeat = 0.01f;
    if (BZDB.isSet( "groundTexRepeat" ))
      repeat = BZDB.eval( "groundTexRepeat" );

    if (BZDB.eval("useQuality")>=3)
      repeat = BZDB.eval( "groundHighResTexRepeat" );

  uv[0] = repeat * p[0];
  uv[1] = repeat * p[1];
}

void			SceneRenderer::enableLight(int index, bool on)
{
  OpenGLLight::enableLight(index + reservedLights, on);
}

void			SceneRenderer::enableSun(bool on)
{
  if (BZDB.isTrue("lighting") && sunOrMoonUp)
    theSun.enableLight(SunLight, on);
}

void			SceneRenderer::addLight(OpenGLLight& light)
{
  // add light
  lights.push_back(&light);
}

void			SceneRenderer::addFlareLight(
				const float* pos, const float* color)
{
  flareLightList.push_back(FlareLight(pos, color));
}

int			SceneRenderer::getNumLights() const
{
  if ((int)lights.size() > maxLights) return maxLights;
  return lights.size();
}

int			SceneRenderer::getNumAllLights() const
{
  return lights.size();
}

void			SceneRenderer::clearLights()
{
  lights.clear();
}

void			SceneRenderer::setTimeOfDay(double julianDay)
{

  // get position of sun and moon at 0,0 lat/long
  float sunDir[3], moonDir[3];
  float latitude = BZDB.eval("latitude");
  float longitude = BZDB.eval("longitude");
  getSunPosition(julianDay, latitude, longitude, sunDir);
  getMoonPosition(julianDay, latitude, longitude, moonDir);
  ::getCelestialTransform(julianDay, latitude, longitude,
				(float(*)[4])celestialTransform);

  // set sun position
  if (sunDir[2] >= -0.009f) {
    // sun is our source of illumination
    sunOrMoonUp = true;
    theSun.setDirection(sunDir);
  }
  else if (moonDir[2] > -0.009f) {
    // moon is our source of illumination
    sunOrMoonUp = true;
    theSun.setDirection(moonDir);
  }
  else {
    // `the moon is down' (well, so is the sun, but that's not Shakespeare)
    // -- ambient only
    sunOrMoonUp = false;
  }

  // set sun and ambient colors
  GLfloat ambientColor[4];
  ::getSunColor(sunDir, sunColor, ambientColor, sunBrightness);
  theSun.setColor(sunColor);
  GLfloat maxComponent = sunColor[0];
  if (sunColor[1] > maxComponent) maxComponent = sunColor[1];
  if (sunColor[2] > maxComponent) maxComponent = sunColor[2];
  if (maxComponent <= 0.0f) maxComponent = 1.0f;
  sunScaledColor[0] = sunColor[0] / maxComponent;
  sunScaledColor[1] = sunColor[1] / maxComponent;
  sunScaledColor[2] = sunColor[2] / maxComponent;
  ambientColor[3] = 1.0f;
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

  if (background)
    background->setCelestial(*this, sunDir, moonDir);
}

void			SceneRenderer::render(
				bool _lastFrame,
				bool _sameFrame,
				bool fullWindow)
{
  static const GLfloat blindnessColor[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
  static const GLfloat dimnessColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  static const float dimDensity = 0.75f;
  bool               lighting   = BZDB.isTrue("lighting");

  lastFrame = _lastFrame;
  sameFrame = _sameFrame;

  // make sure there is something to render on
  if (!window) return;

  // avoid OpenGL calls as long as possible -- there's a good
  // chance we're waiting on the vertical retrace.

  // set the view frustum
  if (sceneIterator) sceneIterator->resetFrustum(&frustum);

  // get the important lights in the scene
  int i;
  int numLights = 0;
  if (!sameFrame) {
    clearLights();
    if (sceneIterator && !blank && lighting) {
      // add lights
      sceneIterator->reset();
      SceneNode* node;
      while ((node = sceneIterator->getNext()) != NULL)
	node->addLight(*this);
      numLights = lights.size();

      // pick maxLights most important light sources
      // should go by full lighting function but we'll just go by distance
      if (numLights > maxLights) {
	const GLfloat* eye = frustum.getEye();
	for (i = 0; i < maxLights; i++) {
	  GLfloat maxImportance = lights[i]->getImportance(eye);
	  for (int j = i + 1; j < numLights; j++) {
	    GLfloat importance = lights[j]->getImportance(eye);
	    if (importance > maxImportance) {
	      OpenGLLight* temp = lights[i];
	      lights[j] = lights[i];
	      lights[i] = temp;
	      maxImportance = importance;
	    }
	  }
	}
	numLights = maxLights;
      }
    }
  }

  // get the nodes to draw
  if (!blank) {
    // empty the render node lists in preparation for the next frame
    OpenGLGState::clearLists();
    orderedList.clear();
    shadowList.clear();
    flareLightList.clear();

    // make the lists of render nodes sorted in optimal rendering order
    if (sceneIterator) {
      sceneIterator->reset();
      SceneNode* node;
      while ((node = sceneIterator->getNext()) != NULL)
	node->getRenderNodes(*this);
    }

    // sort ordered list in reverse depth order
    if (!inOrder)
      orderedList.sort(frustum.getEye());
  }
  else {
    sceneIterator->reset();
    SceneNode* node;
    while ((node = sceneIterator->getNext()) != NULL)
      node->getRenderNodes(*this);
  }

  // add the shadow rendering nodes
  if (!blank && BZDBCache::shadows && scene) {
    scene->addShadowNodes(*this);
  }

  // prepare transforms
  // note -- lights should not be positioned before view is set
  frustum.executeDeepProjection();
  glPushMatrix();
  frustum.executeView();

  // turn sunlight on -- the ground needs it
  if (lighting && sunOrMoonUp) {
    theSun.execute(SunLight);
    theSun.enableLight(SunLight);
  }

  // turn on fog for teleporter blindness if close to a teleporter
  float teleporterProximity = 0.0f;
  if (!blank && LocalPlayer::getMyTank() && (LocalPlayer::getMyTank()->getTeam() != ObserverTeam))
    teleporterProximity = LocalPlayer::getMyTank()->getTeleporterProximity();

  float worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
  bool reallyUseFogHack = useFogHack && (useQualityValue >= 2);
  if (reallyUseFogHack) {
    if (useDimming) {
      const float density = dimDensity;
      glFogi(GL_FOG_MODE, GL_LINEAR);
      glFogf(GL_FOG_START, -density * 1000.0f * worldSize);
      glFogf(GL_FOG_END, (1.0f - density) * 1000.0f * worldSize);
      glFogfv(GL_FOG_COLOR, dimnessColor);
      glEnable(GL_FOG);
    }
    else if (teleporterProximity > 0.0f && useFogHack) {
      const float density = (teleporterProximity > 0.75f) ?
				1.0f : teleporterProximity / 0.75f;
      glFogi(GL_FOG_MODE, GL_LINEAR);
      glFogf(GL_FOG_START, -density * 1000.0f * worldSize);
      glFogf(GL_FOG_END, (1.0f - density) * 1000.0f * worldSize);
      glFogfv(GL_FOG_COLOR, blindnessColor);
      glEnable(GL_FOG);
    }
  }

  // set scissor

  glScissor(window->getOriginX(), window->getOriginY() + window->getHeight() - window->getViewHeight(),
      window->getWidth(), window->getViewHeight());

  if (useDepthComplexityOn) {
    glEnable(GL_STENCIL_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 0, 0xf);
    glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
  }
  if (useHiddenLineOn) {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  }
  else if (useWireframeOn) {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }

  // prepare z buffer
  if (BZDB.isTrue("zbuffer")) {
    if (sameFrame && ++depthRange == numDepthRanges) depthRange = 0;
    if (exposed || useHiddenLineOn || --depthRange < 0) {
      depthRange = numDepthRanges - 1;
      glClear(GL_DEPTH_BUFFER_BIT);
      exposed = false;
    }
    if (!sameFrame && numDepthRanges != 1) {
      if (useHiddenLineOn) {
	glDepthRange(0.0, 1.0);
      }
      else {
	GLclampd x_near = (GLclampd)depthRange * depthRangeSize;
	glDepthRange(x_near, x_near + depthRangeSize);
      }
    }
  }

  // draw start of background (no depth testing)
  OpenGLGState::resetState();
  if (background) {
    background->setBlank(blank);
    background->setInvert(invert);
    background->renderSkyAndGround(*this, fullWindow);
  }

  // prepare the other lights but don't turn them on yet --
  // we may need to turn them on when drawing the background.
  if (lighting) {
    for (i = 0; i < numLights; i++)
      lights[i]->execute(i + reservedLights);
  }

  // draw rest of background
  if (background)
    background->render(*this);

  if (!blank) {
    if (lighting) {
      // now turn on the remaining lights
      for (i = 0; i < numLights; i++)
	OpenGLLight::enableLight(i + reservedLights);
    }

    frustum.executeProjection();

    if (BZDB.isTrue("zbuffer")) glEnable(GL_DEPTH_TEST);

    if (useHiddenLineOn) {
#if defined(GL_VERSION_1_1)
      glEnable(GL_POLYGON_OFFSET_FILL);
#elif defined(GL_EXT_polygon_offset)
      glEnable(GL_POLYGON_OFFSET_EXT);
#endif
    }

    if (sceneIterator && useCullingTreeOn) {
      sceneIterator->drawCuller();
    }
    if (sceneIterator && useCollisionTreeOn && (World::getWorld() != NULL)) {
      World::getWorld()->drawCollisionGrid();
    }

    doRender();

    if (useHiddenLineOn) {
#if defined(GL_VERSION_1_1)
      glDisable(GL_POLYGON_OFFSET_FILL);
#elif defined(GL_EXT_polygon_offset)
      glDisable(GL_POLYGON_OFFSET_EXT);
#endif
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      doRender();
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    OpenGLGState::resetState();

    // shut off lights
    if (lighting) {
      theSun.enableLight(SunLight, false);
      for (i = 0; i < numLights; i++)
	OpenGLLight::enableLight(i + reservedLights, false);
    }

    if (BZDB.isTrue("zbuffer")) glDisable(GL_DEPTH_TEST);

    // FIXME -- must do post-rendering: flare lights, etc.
    // flare lights are in world coordinates.  trace ray to that world
    // position and calculate opacity.  if opaque then don't render
    // flare, otherwise modulate input color by opacity and draw a
    // billboard texture (constant size in screen space).
  }

  // back to original state
  if (!useHiddenLineOn && useWireframeOn)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glPopMatrix();

  if ((reallyUseFogHack && (teleporterProximity > 0.0f || useDimming)))
    glDisable(GL_FOG);

  if (!reallyUseFogHack) {
    float density = 0.0f;
    const GLfloat* color = NULL;
    if (useDimming) {
      density = dimDensity;
      color = dimnessColor;
    }
    else if (teleporterProximity > 0.0f) {
      density = (teleporterProximity > 0.75f) ?
			1.0f : teleporterProximity / 0.75f;
      color = blindnessColor;
    }
    if (density > 0.0f && color != NULL) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glMatrixMode(GL_MODELVIEW);
      glColor4f(color[0], color[1], color[2], density);

      // if low quality then use stipple -- it's probably much faster
      if (BZDBCache::blend && (useQualityValue >= 2)) {
	glEnable(GL_BLEND);
	glRectf(-1.0f, -1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);
      }
      else {
	OpenGLGState::setStipple(density);
	glEnable(GL_POLYGON_STIPPLE);
	glRectf(-1.0f, -1.0f, 1.0f, 1.0f);
	glDisable(GL_POLYGON_STIPPLE);
      }
    }
  }

  if (useDepthComplexityOn) {
    static const GLfloat depthColors[][3] = {
				{ 0.0f, 0.0f, 0.0f },	// black -- 0 times
				{ 0.5f, 0.0f, 1.0f },	// purple -- 1 time
				{ 0.0f, 0.0f, 1.0f },	// blue -- 2 times
				{ 0.0f, 1.0f, 1.0f },	// cyan -- 3 times
				{ 0.0f, 1.0f, 0.0f },	// green -- 4 times
				{ 1.0f, 1.0f, 0.0f },	// yellow -- 5 times
				{ 1.0f, 0.5f, 0.0f },	// orange -- 6 times
				{ 1.0f, 0.0f, 0.0f }	// red -- 7 or more
			};
    static const int numColors = countof(depthColors);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    for (i = 0; i < numColors; i++) {
      glStencilFunc(i == numColors - 1 ? GL_LEQUAL : GL_EQUAL, i, 0xf);
      glColor3fv(depthColors[i]);
      glRectf(-1.0f, -1.0f, 1.0f, 1.0f);
    }
    glDisable(GL_STENCIL_TEST);
  }
}

void			SceneRenderer::notifyStyleChange()
{
  style++;

/* FIXME
  // fixup my gstates
  OpenGLGStateBuilder builder(flareGState);
  builder.enableTexture(BZDBCache::texture));
  if (BZDB.isTrue("smooth")) {
    if (BZDBCache::texture))
      builder.setBlending(GL_ONE, GL_ONE);
    else
      builder.setBlending();
    builder.setSmoothing();
  }
  else {
    if (BZDBCache::texture)
      builder.setBlending(GL_ONE, GL_ONE);
    else
      builder.resetBlending();
    builder.setSmoothing(false);
  }
  flareGState = builder.getState();
*/
}

const RenderNodeList&	SceneRenderer::getShadowList() const
{
  return shadowList;
}

void			SceneRenderer::addRenderNode(
				RenderNode* node, const OpenGLGState* gstate)
{
  if (inOrder || gstate->isBlended()) {
    // nodes will be drawn in the same order received
    orderedList.append(node, gstate);
  }

  else {
    // store node in gstate bucket
    gstate->addRenderNode(node);
  }
}

void			SceneRenderer::addShadowNode(RenderNode* node)
{
  shadowList.append(node);
}

void			SceneRenderer::doRender()
{
  // NOTE -- this should go into a separate thread
  // now draw each render node list
  OpenGLGState::renderLists();

  // finally draw all the stuff in the ordered list.  turn
  // off depth buffer updates for potentially transparent stuff.
  glDepthMask(GL_FALSE);
  orderedList.render();
  glDepthMask(GL_TRUE);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
