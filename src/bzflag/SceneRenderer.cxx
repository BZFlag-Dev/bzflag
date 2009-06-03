/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* FIXME -- ugh.. the class header for this file is listed as a public header
 * and is used by other libs, yet this cxx is here.. bad bad bad.  need to
 * decouple this file from the bzflag front-end specific sources so that it
 * may be moved elsewhere.
 */

// interface header
#include "SceneRenderer.h"

// common headers
#include "SceneDatabase.h"
#include "MainWindow.h"
#include "GameTime.h"
#include "DynamicColor.h"
#include "TextureMatrix.h"
#include "TankSceneNode.h"
#include "StateDatabase.h"
#include "TextUtils.h"
#include "ParseColor.h"
#include "BZDBCache.h"
#include "MeshSceneNode.h"
#include "FlagSceneNode.h"
#include "LinkManager.h"
#include "MeshFace.h"

// local headers -- FIXME, local dependencies for a global interface
#include "BackgroundRenderer.h"
#include "DynamicWorldText.h"
#include "LocalPlayer.h"
#include "Roaming.h"
#include "TrackMarks.h"
#include "World.h"
#include "Daylight.h"
#include "playing.h"
#include "guiplaying.h"


//
// SceneRenderer
//

const int   SceneRenderer::SunLight = 0;		// also for the moon
const float SceneRenderer::dimDensity = 0.75f;
const fvec4 SceneRenderer::dimnessColor(0.0f, 0.0f, 0.0f, 1.0f);
const fvec4 SceneRenderer::blindnessColor(1.0f, 1.0f, 0.0f, 1.0f);

/* initialize the singleton */
template <>
SceneRenderer* Singleton<SceneRenderer>::_instance = (SceneRenderer*)0;


SceneRenderer::SceneRenderer()
: window(NULL)
, specialMode(NoSpecial)
, blank(false)
, invert(false)
, mirror(false)
, drawingMirror(false)
, mapFog(false)
, sunBrightness(1.0f)
, scene(NULL)
, background(NULL)
, useQualityValue(_HIGH_QUALITY)
, panelOpacity(0.3f)
, radarSize(4)
, maxMotionFactor(5)
, viewType(Normal)
, inOrder(false)
, useDimming(false)
, canUseHiddenLine(false)
, exposed(true)
, lastFrame(true)
, sameFrame(false)
, needStyleUpdate(true)
, rebuildTanks(true)
{
  lightsSize = 4;
  lights = new OpenGLLight*[lightsSize];
  lightsCount = 0;
  dynamicLights = 0;

  // init the track mark manager
  TrackMarks::init();

  return;
}


void SceneRenderer::setWindow(MainWindow* _window) {
  window = _window;

  // get visual info
  window->getWindow()->makeCurrent();
  GLint bits;
  if (!BZDB.isSet("zbuffer")) {
//	  glGetIntegerv(GL_DEPTH_BITS, &bits);
    BZDB.set("zbuffer", "1");
  }
  glGetIntegerv(GL_STENCIL_BITS, &bits);
  useStencilOn = (bits > 0);
  if (!useStencilOn) {
    BZDB.setBool("stencilShadows", false);
  }

  // can only do hidden line if polygon offset is available
  canUseHiddenLine = true;

  // prepare context with stuff that'll never change
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
  glGetIntegerv(GL_MAX_LIGHTS, &maxLights);
  reservedLights = 1;			// only one light between sun and moon
  maxLights -= reservedLights;		// can't use the reserved lights

  // prepare sun
  setTimeOfDay(Daylight::unixEpoch);

  // force nodes to update their styles
  notifyStyleChange();
}


SceneRenderer::~SceneRenderer()
{
  // free database
  delete scene;

  // free lights list
  delete[] lights;

  // kill the track manager
  TrackMarks::kill();
}


bool SceneRenderer::useStencil() const
{
  return useStencilOn;
}


SceneRenderer::ViewType	SceneRenderer::getViewType() const
{
  return viewType;
}


void SceneRenderer::setQuality(int value)
{
  // 0 = Low
  // 1 = Medium
  // 2 = High
  // 3 = Experimental

  if (value < 0) {
    value = 0;
  } else if (value > BZDB.eval("maxQuality")) {
    value = BZDB.evalInt("maxQuality");
  }
  if (useQualityValue != value) {
    rebuildTanks = true;
  }
  useQualityValue = value;

  notifyStyleChange();

  if (useQualityValue >= _HIGH_QUALITY) {
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    // GL_NICEST for polygon smoothing seems to make some drivers
    // cause massive slowdowns and "spikes" when drawing the radar
    glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
  } else {
    glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
  }

  if (useQualityValue >= _HIGH_QUALITY)
    TankSceneNode::setMaxLOD(-1);
  else if (useQualityValue >= _MEDIUM_QUALITY)
    TankSceneNode::setMaxLOD(3);
  else
    TankSceneNode::setMaxLOD(2);

  if (useQualityValue >= _EXPERIMENTAL_QUALITY) {
    BZDB.set("maxFlagLOD", "8"); // 256 quads
  } else if (useQualityValue >= _HIGH_QUALITY) {
    BZDB.set("maxFlagLOD", "6"); //  64 quads
  } else {
    BZDB.set("maxFlagLOD", "4"); //  16 quads
  }

  if (useQualityValue >= _HIGH_QUALITY)
    BZDB.set("moonSegments","64");
  else if (useQualityValue >= _MEDIUM_QUALITY)
    BZDB.set("moonSegments","24");
  else
    BZDB.set("moonSegments","12");

  if (useQualityValue > _LOW_QUALITY) {
    // this can be modified by OpenGLMaterial
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  } else {
    // OpenGLMaterial will not modify if (quality <= 0)
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
  }

  // this setting helps keep those specular highlights
  // highlighting when applied to a dark textured surface.
  if (useQualityValue >= _MEDIUM_QUALITY) {
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
  } else {
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
  }

  BZDB.set("useQuality", TextUtils::format("%d", value));
}


void SceneRenderer::setSpecialMode(SpecialMode mode)
{
  specialMode = NoSpecial;

  // avoid built-in visibility cheats
  if (ROAM.isRoaming()) {
    switch (mode) {
      case WireFrame: {
        specialMode = mode;
        break;
      }
      case HiddenLine: {
        if (BZDBCache::zbuffer) {
          specialMode = mode;
        }
        break;
      }
      case DepthComplexity: {
        GLint bits;
        glGetIntegerv(GL_STENCIL_BITS, &bits);
        if (bits >= 3) {
          specialMode = mode;
        }
        break;
      }
      default: {
        break;
      }
    }
  }

  BZDB.setInt("specialMode", specialMode);

  return;
}


SceneRenderer::SpecialMode SceneRenderer::getSpecialMode() const
{
  return specialMode;
}


void SceneRenderer::setRebuildTanks()
{
  rebuildTanks = true;
}


void SceneRenderer::setupBackgroundMaterials()
{
  if (background) {
    background->setupSkybox();
    background->setupGroundMaterials();
  }
  return;
}


void SceneRenderer::setPanelOpacity(float opacity)
{
  bool needToResize = opacity == 1.0f || panelOpacity == 1.0f;

  panelOpacity = opacity;
  notifyStyleChange();
  if (needToResize) {
    if (window) {
      window->setFullView(panelOpacity < 1.0f);
      window->getWindow()->callResizeCallbacks();
    }
  }
}


float SceneRenderer::getPanelOpacity() const
{
  return panelOpacity;
}


void SceneRenderer::setRadarSize(int size)
{
  radarSize = size;
  notifyStyleChange();
  if (window) {
    window->getWindow()->callResizeCallbacks();
  }
}


int SceneRenderer::getRadarSize() const
{
  return radarSize;
}


void SceneRenderer::setMaxMotionFactor(int factor)
{
  if (factor < -11) {
    factor = -11;
  }
  maxMotionFactor = factor;
  notifyStyleChange();
  if (window) {
    window->getWindow()->callResizeCallbacks();
  }
}


int SceneRenderer::getMaxMotionFactor() const
{
  return maxMotionFactor;
}


void SceneRenderer::setDim(bool on)
{
  useDimming = on;
}


void SceneRenderer::setViewType(ViewType _viewType)
{
  viewType = _viewType;
}


void SceneRenderer::setExposed()
{
  exposed = true;
}


void SceneRenderer::clearRadar(float opacity)
{
  int size = window->getHeight() - window->getViewHeight();
  float op = (opacity > 1.0f) ? 1.0f : (opacity < 0.0f) ? 0.0f : opacity;
  glScissor(window->getOriginX(), 0, size, size);
  glClearColor(0.0f, 0.0f, 0.0f, op);
  glClear(GL_COLOR_BUFFER_BIT);
}


void SceneRenderer::setSceneDatabase(SceneDatabase* db)
{
  // update the styles
  needStyleUpdate = true;

  // free the current database
  delete scene;

  scene = db;
  if (scene) {
    inOrder = scene->isOrdered();
  } else {
    inOrder = false;
  }

  // update the background materials
  setupBackgroundMaterials();

  return;
}


void SceneRenderer::setBackground(BackgroundRenderer* br)
{
  background = br;
}


void SceneRenderer::getGroundUV(const fvec2& p, fvec2& uv) const
{
  float repeat = 0.01f;
  if (useQualityValue >= _HIGH_QUALITY) {
    repeat = BZDB.eval("groundHighResTexRepeat");
  }
  else if (BZDB.isSet("groundTexRepeat")) {
    repeat = BZDB.eval("groundTexRepeat");
  }
  uv = repeat * p;
}


void SceneRenderer::enableLight(int index, bool on)
{
  OpenGLLight::enableLight(index + reservedLights, on);
}


void SceneRenderer::enableSun(bool on)
{
  if (BZDBCache::lighting && sunOrMoonUp) {
    OpenGLLight::enableLight(SunLight, on);
  }
}


void SceneRenderer::setupSun()
{
  if (BZDBCache::lighting && sunOrMoonUp) {
    theSun.execute(SunLight, BZDB.isTrue("lightLists"));
  }
}


void SceneRenderer::addLight(OpenGLLight& light)
{
  // add a light, and grow the maximum list size if required
  lightsCount++;
  if (lightsCount > lightsSize) {
    OpenGLLight** newList = new OpenGLLight*[lightsSize * 2];
    memcpy (newList, lights, lightsSize * sizeof(OpenGLLight*));
    delete[] lights;
    lights = newList;
    lightsSize = lightsSize * 2;
  }
  lights[lightsCount - 1] = &light;
  return;
}


int SceneRenderer::getNumLights() const
{
  return dynamicLights;
}


int SceneRenderer::getNumAllLights() const
{
  return lightsCount;
}


void SceneRenderer::clearLights()
{
  lightsCount = 0;
  dynamicLights = 0;
  return;
}


void SceneRenderer::setTimeOfDay(double julianDay)
{

  // get position of sun and moon at 0,0 lat/long
  fvec3 sunDir, moonDir;
  float latitude, longitude;
  if (!BZDB.isTrue(StateDatabase::BZDB_SYNCLOCATION)) {
    // use local (client) settings
    latitude  = BZDB.eval("latitude");
    longitude = BZDB.eval("longitude");
  } else {
    // server settings
    latitude  = BZDB.eval(StateDatabase::BZDB_LATITUDE);
    longitude = BZDB.eval(StateDatabase::BZDB_LONGITUDE);
  }

  Daylight::getSunPosition(julianDay, latitude, longitude, sunDir);
  Daylight::getMoonPosition(julianDay, latitude, longitude, moonDir);
  Daylight::getCelestialTransform(julianDay, latitude, longitude,
                                  celestialTransform);

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
  Daylight::getSunColor(sunDir, sunColor, ambientColor, sunBrightness);
  theSun.setColor(sunColor);
  float maxComponent = sunColor.r;
  if (sunColor.y > maxComponent) { maxComponent = sunColor.y; }
  if (sunColor.z > maxComponent) { maxComponent = sunColor.z; }
  if (maxComponent <= 0.0f)      { maxComponent = 1.0f; }
  sunScaledColor.rgb() = sunColor.rgb() / maxComponent;
  ambientColor.a = 1.0f;
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

  if (background) {
    background->setCelestial(*this, sunDir, moonDir);
  }
}


static int sortLights (const void* a, const void* b)
{
  // the higher getImportance(), the closer it is to the beginning
  const OpenGLLight* lightA = *((const OpenGLLight**) a);
  const OpenGLLight* lightB = *((const OpenGLLight**) b);
  const float valA = lightA->getImportance();
  const float valB = lightB->getImportance();

  // first sort by cull
  if (valA < 0.0f) {
    if (valB >= 0.0f) {
      return +1;
    } else {
      return 0;
    }
  }
  if (valB < 0.0f) {
    if (valA >= 0.0f) {
      return -1;
    } else {
      return 0;
    }
  }

  // sort by grounded state
  const bool groundedA = lightA->getOnlyGround();
  const bool groundedB = lightB->getOnlyGround();
  if (groundedA && !groundedB) {
    return +1;
  }
  if (!groundedA && groundedB) {
    return -1;
  }

  // sort by importance
  if (valA > valB) {
    return -1;
  } else {
    return +1;
  }
}


void SceneRenderer::render(bool _lastFrame, bool _sameFrame, bool _fullWindow)
{
  lastFrame  = _lastFrame;
  sameFrame  = _sameFrame;
  fullWindow = _fullWindow;

  // set the special mode
  if (!ROAM.isRoaming() && (specialMode != NoSpecial)) {
    setSpecialMode(NoSpecial);
  }

  triangleCount = 0;
  RenderNode::resetTriangleCount();
  if (background) {
    background->resetTriangleCount();
  }

  // update the SceneNode, Background, and TrackMark styles
  if (needStyleUpdate) {
    updateNodeStyles();
  }

  if (rebuildTanks) {
    TankGeometryMgr::deleteLists();
    TankGeometryMgr::buildLists();
    rebuildTanks = false;
  }

  // update the dynamic colors
  DYNCOLORMGR.update();

  // update the texture matrices
  TEXMATRIXMGR.update();

  // make sure there is something to render on
  if (!window) {
    return;
  }

  // setup the viewport LOD scale
  MeshSceneNode::setLodScale(window->getWidth(), frustum.getFOVx(),
			     window->getViewHeight(), frustum.getFOVy());
  {
    const int pixelsX = window->getWidth();
    const int pixelsY = window->getViewHeight();
    const float fovx = frustum.getFOVx();
    const float fovy = frustum.getFOVy();
    const float lppx = 2.0f * sinf(fovx * 0.5f) / (float)pixelsX;
    const float lppy = 2.0f * sinf(fovy * 0.5f) / (float)pixelsY;
    const float lpp = (lppx < lppy) ? lppx : lppy;
    static BZDB_float lodScale("lodScale");
    lengthPerPixel = lpp * lodScale;
  }

  // get the track mark sceneNodes (only for BSP)
  if (scene) {
    TrackMarks::addSceneNodes(scene);
  }

  // turn on fog for teleporter blindness if close to a teleporter
  teleporterProximity = 0.0f;
  if (!blank && LocalPlayer::getMyTank() &&
      (LocalPlayer::getMyTank()->getTeam() != ObserverTeam)) {
    teleporterProximity = LocalPlayer::getMyTank()->getTeleporterProximity();
  }

  // fog setup
  mapFog = setupMapFog();

  mirror = (BZDB.get(StateDatabase::BZDB_MIRROR) != "none")
	   && BZDB.isTrue("userMirror");

  clearZbuffer = true;
  drawGround = true;

  // draw the mirror pass
  if (mirror) {
    drawMirror();
  }

  // the real scene
  renderScene();

  // finalize
  if (mapFog) {
    glDisable(GL_FOG);
  }
  renderPostDimming();

  triangleCount = RenderNode::getTriangleCount();
  if (background) {
    triangleCount += background->getTriangleCount();
  }

  return;
}


void SceneRenderer::drawMirror()
{
  drawingMirror = true;
  drawGround = false;

  // flip for the reflection drawing
  frustum.flipVertical();
  glFrontFace(GL_CW);

  // different occluders for the mirror
  if (scene) { scene->setOccluderManager(1); }

  // the reflected scene
  renderScene();

  // different occluders for the mirror
  if (scene) { scene->setOccluderManager(0); }

  // flip back
  frustum.flipVertical();
  glFrontFace(GL_CCW);

  fvec4 mirrorColor;
  if (!parseColorString(BZDB.get(StateDatabase::BZDB_MIRROR), mirrorColor)) {
    mirrorColor.r = mirrorColor.g = mirrorColor.b = 0.0f;
    mirrorColor.a = 0.5f;
  } else if (mirrorColor.a == 1.0f) {
    // probably a mistake
    mirrorColor.a = 0.5f;
  }
  if (invert) {
    mirrorColor.r = 1.0f - mirrorColor.r;
    mirrorColor.b = 1.0f - mirrorColor.b;
    mirrorColor.a = 0.2f;
  }

  // darken the reflection
  if (!mapFog) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // if low quality then use stipple -- it's probably much faster
    if (BZDBCache::blend && (useQualityValue >= _MEDIUM_QUALITY)) {
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
      glColor4fv(mirrorColor);
      glRectf(-1.0f, -1.0f, +1.0f, +1.0f);
      glDisable(GL_BLEND);
    } else {
      glColor3fv(mirrorColor);
      OpenGLGState::setStipple(mirrorColor.a);
      glEnable(GL_POLYGON_STIPPLE);
      glRectf(-1.0f, -1.0f, +1.0f, +1.0f);
      glDisable(GL_POLYGON_STIPPLE);
    }
  } else {
    // need the proper matrices for fog generation
    // if low quality then use stipple -- it's probably much faster
    frustum.executeView();
    frustum.executeProjection();
    const float extent = BZDBCache::worldSize * 10.0f;
    if (BZDBCache::blend && (useQualityValue >= _MEDIUM_QUALITY)) {
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
      glColor4fv(mirrorColor);
      glRectf(-extent, -extent, +extent, +extent);
      glDisable(GL_BLEND);
    } else {
      glColor3fv(mirrorColor);
      OpenGLGState::setStipple(mirrorColor.a);
      glEnable(GL_POLYGON_STIPPLE);
      glRectf(-extent, -extent, +extent, +extent);
      glDisable(GL_POLYGON_STIPPLE);
    }
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }

  clearZbuffer = false;
  drawingMirror = false;
}


void SceneRenderer::renderScene()
{
  int i;
  const bool lightLists = BZDB.isTrue("lightLists");

  // avoid OpenGL calls as long as possible -- there's a good
  // chance we're waiting on the vertical retrace.

  // get a list of the dynamic lights
  getLights();

  // get the obstacle sceneNodes and shadowNodes
  getRenderNodes();

  // update the required flag phases
  // (after the sceneNodes have been added, before the rendering)
  float waveSpeed = 1.0f;
  const World* world = World::getWorld();
  if (world) {
    static const fvec3 pos(0.0f, 0.0f, 0.0f);
    fvec3 wind;
    world->getWind(wind, pos);
    const float speed = sqrtf((wind[0] * wind[0]) + (wind[1] * wind[1]));
    waveSpeed = 1.0f + (speed * 0.1f);
  }
  FlagSceneNode::waveFlags(waveSpeed);

  // prepare transforms
  // note -- lights should not be positioned before view is set
  frustum.executeDeepProjection();
  frustum.executeView();

  // turn sunlight on -- the ground needs it
  if (BZDBCache::lighting && sunOrMoonUp) {
    theSun.execute(SunLight, lightLists);
    OpenGLLight::enableLight(SunLight, true);
  }

  // set scissor
  const int windowYOffset = window->getHeight() - window->getViewHeight();
  glScissor(window->getOriginX(), window->getOriginY() + windowYOffset,
            window->getWidth(), window->getViewHeight());

  const bool origStencilShadows = BZDBCache::stencilShadows;
  switch (specialMode) {
    case DepthComplexity: {
      if (BZDBCache::stencilShadows) {
        BZDB.setBool("stencilShadows", false);
      }
      glEnable(GL_STENCIL_TEST);
      if (!mirror || (clearZbuffer)) {
        glClear(GL_STENCIL_BUFFER_BIT);
      }
      glStencilFunc(GL_ALWAYS, 0, 0xf);
      glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
      break;
    }
    case HiddenLine: {
      if (!mirror || (clearZbuffer)) {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
      }
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      break;
    }
    case WireFrame: {
      if (!mirror || (clearZbuffer)) {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
      }
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      break;
    }
    default: {
      // do nothing
      break;
    }
  }

  // prepare z buffer
  if (BZDBCache::zbuffer && clearZbuffer) {
    glClear(GL_DEPTH_BUFFER_BIT);
  }

  // draw start of background (no depth testing)
  OpenGLGState::resetState();
  if (background) {
    background->setBlank(blank);
    background->setInvert(invert);

    const bool avoidSkyFog = (mapFog && BZDB.isTrue("_fogNoSky"));
    if (avoidSkyFog) {
      glDisable(GL_FOG);
      background->renderSky(*this, fullWindow, mirror);
      glEnable(GL_FOG);
    } else {
      background->renderSky(*this, fullWindow, mirror);
    }

    if (drawGround) {
      background->renderGround(*this, fullWindow);
    }
  }

  // prepare the other lights but don't turn them on yet --
  // we may need to turn them on when drawing the background.
  if (BZDBCache::lighting) {
    for (i = 0; i < dynamicLights; i++) {
      lights[i]->execute(i + reservedLights, lightLists);
    }
  }

  // draw rest of background
  // (ground grid, shadows, fake shot lights, mountains, clouds)
  if (background) {
    background->renderGroundEffects(*this, mirror && clearZbuffer);
  }

  if (!blank) {
    if (BZDBCache::lighting) {
      // now turn on the remaining lights
      for (i = 0; i < dynamicLights; i++) {
	OpenGLLight::enableLight(i + reservedLights, true);
      }
    }

    frustum.executeProjection();

    if (BZDBCache::zbuffer) {
      glEnable(GL_DEPTH_TEST);
    }

    if (specialMode == HiddenLine) {
      glEnable(GL_POLYGON_OFFSET_FILL);
    }

    if (ROAM.isRoaming()) {
      if (scene && BZDBCache::showCullingGrid) {
        scene->drawCuller();
      }
      if (scene && BZDBCache::showCollisionGrid && (world != NULL)) {
        world->drawCollisionGrid();
      }
    }

    ///////////////////////
    // THE BIG RENDERING //
    ///////////////////////
    doRender();


    if (specialMode == HiddenLine) {
      glDisable(GL_POLYGON_OFFSET_FILL);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      doRender();
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    OpenGLGState::resetState();

    // shut off lights
    if (BZDBCache::lighting) {
      OpenGLLight::enableLight(SunLight, false);
      for (i = 0; i < dynamicLights; i++) {
	OpenGLLight::enableLight(i + reservedLights, false);
      }
    }

    if (BZDBCache::zbuffer) {
      glDisable(GL_DEPTH_TEST);
    }

    // FIXME -- must do post-rendering: flare lights, etc.
    // flare lights are in world coordinates.  trace ray to that world
    // position and calculate opacity.  if opaque then don't render
    // flare, otherwise modulate input color by opacity and draw a
    // billboard texture (constant size in screen space).
  }

  // back to original state
  if (specialMode == WireFrame) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // do depth complexity
  if (specialMode == DepthComplexity) {
    renderDepthComplexity();
    BZDB.setBool("stencilShadows", origStencilShadows);
  }

  return;
}


void draw3rdPersonTarget(SceneRenderer* /*renderer*/)
{
  LocalPlayer* myTank = LocalPlayer::getMyTank();

  if (!myTank || !thirdPersonVars.b3rdPerson) {
    return;
  }

  const float *myTankPos   = myTank->getPosition();
  const float *myTankDir   = myTank->getForward();
  const float muzzleHeight = myTank->getMuzzleHeight();

  const float radCon = 57.295779513082320876798154814105f;

  glPushMatrix();
  float distance = thirdPersonVars.nearTargetDistance;
  glTranslatef(myTankPos[0] + (myTankDir[0] * distance),
               myTankPos[1] + (myTankDir[1] * distance),
               myTankPos[2] + muzzleHeight);
  glRotatef(myTank->getAngle() * radCon, 0.0f, 0.0f, 1.0f);

  float size = thirdPersonVars.nearTargetSize;
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);

  glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
  glBegin(GL_LINES);
  glVertex3f(0.0f,  0.0f,   1.00f * size);
  glVertex3f(0.0f,  0.0f,   0.25f * size);
  glVertex3f(0.0f,  0.0f,  -1.00f * size);
  glVertex3f(0.0f,  0.0f,  -0.25f * size);
  glVertex3f(0.0f,   1.00f * size,  0.0f);
  glVertex3f(0.0f,   0.25f * size,  0.0f);
  glVertex3f(0.0f,  -1.00f * size,  0.0f);
  glVertex3f(0.0f,  -0.25f * size,  0.0f);

  glEnd();
  glPopMatrix();

  glPushMatrix();
  distance = thirdPersonVars.farTargetDistance;
  glTranslatef(myTankPos[0] + (myTankDir[0] * distance),
               myTankPos[1] + (myTankDir[1] * distance),
               myTankPos[2] + muzzleHeight);
  glRotatef(myTank->getAngle() * radCon, 0.0f, 0.0f, 1.0f);
  size = thirdPersonVars.farTargetSize;
  glColor4f(0.5f, 0.5f, 0.5f, 0.25f);
  glBegin(GL_LINES);
  glVertex3f(0.0f,  0.0f,   1.00f * size);
  glVertex3f(0.0f,  0.0f,   0.25f * size);
  glVertex3f(0.0f,  0.0f,  -1.00f * size);
  glVertex3f(0.0f,  0.0f,  -0.25f * size);
  glVertex3f(0.0f,   1.00f * size,  0.0f);
  glVertex3f(0.0f,   0.25f * size,  0.0f);
  glVertex3f(0.0f,  -1.00f * size,  0.0f);
  glVertex3f(0.0f,  -0.25f * size,  0.0f);
  glEnd();

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_LIGHTING);
  glPopMatrix();
}


void SceneRenderer::doRender()
{
  const bool mirrorPass = (mirror && clearZbuffer);

  // render the ground tank tracks
  if (!mirrorPass) {
    TrackMarks::renderGroundTracks();
  }

  // NOTE -- this should go into a separate thread
  // now draw each render node list
  OpenGLGState::renderLists();

  draw3rdPersonTarget(this);

  // render the environmental conditions
  if (background) {
    // do not update for the second mirror pass
    background->renderEnvironment(*this, !mirror || clearZbuffer);
  }

  // draw all the stuff in the ordered list.  turn
  // off depth buffer updates for potentially transparent stuff.
  glDepthMask(GL_FALSE);
  orderedList.render();
  drawLinkDebug();
  glDepthMask(GL_TRUE);

  // render the obstacle tank tracks
  if (!mirrorPass) {
    TrackMarks::renderObstacleTracks();
  }

  return;
}


bool SceneRenderer::setupMapFog()
{
  const std::string modeStr = BZDB.get("_fogMode");
  if (modeStr.empty() || (modeStr == "none")) {
    glDisable(GL_FOG);
    glHint(GL_FOG_HINT, GL_FASTEST);
    return false;
  }

  GLenum  fogMode    = GL_EXP;
  float   fogDensity = 0.001f;
  float   fogStart   = BZDBCache::worldSize * 0.5f;
  float   fogEnd     = BZDBCache::worldSize;
  fvec4   fogColor(0.25f, 0.25f, 0.25f, 0.25f);

  // parse the values;
  if (modeStr == "linear") {
    fogMode = GL_LINEAR;
  } else if (modeStr == "exp") {
    fogMode = GL_EXP;
  } else if (modeStr == "exp2") {
    fogMode = GL_EXP2;
  } else {
    fogMode = GL_EXP;
  }

  fogDensity = BZDB.eval(StateDatabase::BZDB_FOGDENSITY);
  fogStart   = BZDB.eval(StateDatabase::BZDB_FOGSTART);
  fogEnd     = BZDB.eval(StateDatabase::BZDB_FOGEND);
  if (!parseColorString(BZDB.get(StateDatabase::BZDB_FOGCOLOR), fogColor)) {
    fogColor.r = fogColor.g = fogColor.b = 0.1f;
    fogColor.a = 0.0f; // has no effect
  }
  if (BZDB.evalInt("fogEffect") >= 1) {
    glHint(GL_FOG_HINT, GL_NICEST);
  } else {
    glHint(GL_FOG_HINT, GL_FASTEST);
  }

  // setup GL fog
  glFogi(GL_FOG_MODE,    fogMode);
  glFogf(GL_FOG_DENSITY, fogDensity);
  glFogf(GL_FOG_START,   fogStart);
  glFogf(GL_FOG_END,     fogEnd);
  glFogfv(GL_FOG_COLOR,  fogColor);
  glEnable(GL_FOG);

  return true;
}


void SceneRenderer::renderPostDimming()
{
  float density = 0.0f;
  const float* color = NULL;
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
    if (BZDBCache::blend && (useQualityValue >= _MEDIUM_QUALITY)) {
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
  return;
}


void SceneRenderer::renderDepthComplexity()
{
  static const fvec4 depthColors[] = {
    fvec4(0.0f, 0.0f, 0.0f, 1.0f), // black -- 0 times
    fvec4(0.5f, 0.0f, 1.0f, 1.0f), // purple -- 1 time
    fvec4(0.0f, 0.0f, 1.0f, 1.0f), // blue -- 2 times
    fvec4(0.0f, 1.0f, 1.0f, 1.0f), // cyan -- 3 times
    fvec4(0.0f, 1.0f, 0.0f, 1.0f), // green -- 4 times
    fvec4(1.0f, 1.0f, 0.0f, 1.0f), // yellow -- 5 times
    fvec4(1.0f, 0.5f, 0.0f, 1.0f), // orange -- 6 times
    fvec4(1.0f, 0.0f, 0.0f, 1.0f)  // red -- 7 or more
  };
  static const int numColors = countof(depthColors);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
  for (int i = 0; i < numColors; i++) {
    glStencilFunc(i == numColors - 1 ? GL_LEQUAL : GL_EQUAL, i, 0xf);
    glColor3fv(depthColors[i]);
    glRectf(-1.0f, -1.0f, 1.0f, 1.0f);
  }
  glDisable(GL_STENCIL_TEST);

  return;
}


void SceneRenderer::getRenderNodes()
{
  // get the nodes to draw
  if (blank) {
    return;
  }

  // empty the render node lists in preparation for the next frame
  OpenGLGState::clearLists();
  orderedList.clear();
  shadowList.clear();

  // make the lists of render nodes sorted in optimal rendering order
  if (scene) {
    scene->addRenderNodes(*this, true, true);
  }

  DYNAMICWORLDTEXT.addRenderNodes(*this);

  // sort ordered list in reverse depth order
  if (!inOrder) {
    orderedList.sort(frustum.getEye());
  }

  // add the shadow rendering nodes
  if (scene && BZDBCache::shadows && (getSunDirection() != NULL) &&
      (!mirror || !clearZbuffer) && !BZDB.isTrue(StateDatabase::BZDB_NOSHADOWS)) {
    setupShadowPlanes();
    scene->addShadowNodes(*this, true, true);
    DYNAMICWORLDTEXT.addShadowNodes(*this);
  }

  return;
}


void SceneRenderer::getLights()
{
  if (sameFrame) {
    return;
  }

  lightsCount = 0;
  dynamicLights = 0;

  // get the important lights in the scene
  if (scene && !blank && BZDBCache::lighting) {
    // get the potential dynamic lights
    scene->addLights(*this);

    // calculate the light importances
    int i;
    for (i = 0; i < lightsCount; i++) {
      lights[i]->calculateImportance(frustum);
    }

    // sort by cull state, grounded state, and importance
    qsort(lights, lightsCount, sizeof(OpenGLLight*), sortLights);

    // count the unculled valid lights and potential dynamic lights
    // (negative values indicate culled lights)
    int unculledCount = 0;
    for (i = 0; i < lightsCount; i++) {
      // any value below 0.0f is culled
      if (lights[i]->getImportance() >= 0.0f) {
        unculledCount++;
        if (!lights[i]->getOnlyGround()) {
          dynamicLights++;
        }
      }
    }

    // set the total light count to the number of unculled lights
    lightsCount = unculledCount;

    // limit the dynamic OpenGL light count
    if (dynamicLights > maxLights) {
      dynamicLights = maxLights;
    }
  }

  return;
}


void SceneRenderer::disableLights(const Extents& exts)
{
  // temporarily turn off non-applicable lights for big meshes
  for (int i = 0; i < dynamicLights; i++) {
    const fvec4& pos = lights[i]->getPosition();
    const float dist = lights[i]->getMaxDist();

    if ((pos.x < (exts.mins.x - dist)) || (pos.x > (exts.maxs.x + dist)) ||
	(pos.y < (exts.mins.y - dist)) || (pos.y > (exts.maxs.y + dist)) ||
	(pos.z < (exts.mins.z - dist)) || (pos.z > (exts.maxs.z + dist))) {
      OpenGLLight::enableLight(i + reservedLights, false);
    }
  }
  return;
}


void SceneRenderer::reenableLights()
{
  // reenable the disabled lights
  for (int i = 0; i < dynamicLights; i++) {
    OpenGLLight::enableLight(i + reservedLights, true);
  }
  return;
}


void SceneRenderer::notifyStyleChange()
{
  needStyleUpdate = true;
  return;
}


void SceneRenderer::updateNodeStyles()
{
  needStyleUpdate = false;
  if (scene) {
    scene->updateNodeStyles();
  }
  if (background) {
    background->notifyStyleChange();
  }
  TrackMarks::notifyStyleChange();
  DYNAMICWORLDTEXT.notifyStyleChange();
  needStyleUpdate = false;
  return;
}


const RenderNodeList& SceneRenderer::getShadowList() const
{
  return shadowList;
}


const fvec3* SceneRenderer::getSunDirection() const
{
  if (background) {
    return background->getSunDirection();
  } else {
    return NULL;
  }
}


const Extents* SceneRenderer::getVisualExtents() const
{
  if (scene) {
    return scene->getVisualExtents();
  } else {
    return NULL;
  }
}


int SceneRenderer::getFrameTriangleCount() const
{
  return triangleCount;
}


int SceneRenderer::getShadowPlanes(const fvec4 **planes) const
{
  if (shadowPlaneCount <= 0) {
    *planes = NULL;
    return 0;
  }
  *planes = shadowPlanes;
  return shadowPlaneCount;
}


void SceneRenderer::setupShadowPlanes()
{
  shadowPlaneCount = 0;

  const fvec3* sunDirPtr = getSunDirection();
  if (sunDirPtr == NULL) {
    return; // no sun = no shadows, simple
  }
  const fvec3& sunDir = *sunDirPtr;

  // FIXME: As a first cut, we'll assume that
  //	    the frustum top points towards Z.
  if (frustum.getUp().z < 0.999f) {
    return; // tilted views are not supported
  }

  const fvec3& eye = frustum.getEye();

  // we project the frustum onto the ground plane, and then
  // use those lines to generate planes in the direction of
  // the sun's light. that is the potential shadow volume.

  // The frustum planes are as follows:
  // 0: left
  // 1: right
  // 2: bottom
  fvec4* planes = shadowPlanes;

  shadowPlaneCount = 2;

  fvec2 edge;

  // left edge
  edge = frustum.getSide(1).xy();
  planes[0].x = (edge.x * sunDir.z);
  planes[0].y = (edge.y * sunDir.z);
  planes[0].z = -fvec2::dot(edge, sunDir.xy());
  planes[0].w = -fvec2::dot(planes[0].xy(), eye.xy());
  // right edge
  edge = frustum.getSide(1).xy();
  planes[1].x = (edge.x * sunDir.z);
  planes[1].y = (edge.y * sunDir.z);
  planes[1].z = -fvec2::dot(edge, sunDir.xy());
  planes[1].w = -fvec2::dot(planes[1].xy(), eye.xy());
  // only use the bottom edge if we have some height (about one jump's worth)
  if (eye.z > 20.0f) {
    // bottom edge
    edge = frustum.getSide(3).xy();
    planes[2].x = (edge.x * sunDir.z);
    planes[2].y = (edge.y * sunDir.z);
    planes[2].z = -fvec2::dot(edge, sunDir.xy());
    const float slope = frustum.getSide(3).z /
                        frustum.getSide(3).xy().length();
    const fvec2 point = eye.xy() + (eye.z * slope * frustum.getSide(3).xy());
    planes[2].w = -fvec2::dot(planes[2].xy(), point);
    shadowPlaneCount++;
  }
}


//============================================================================//
//
//  drawLinkDebug(), and friends
//

static void srcSTPPoint(const fvec3& center, const fvec3& dir, float scale)
{
  scale = (scale == 0.0f) ? 1.0f : (1.0f / scale);
  glVertex3fv(center + (scale * dir));
}


static void srcSTPLine(const fvec3& center, const fvec3& dir, float scale)
{
  scale = (scale == 0.0f) ? 1.0f : (1.0f / scale);
  glVertex3fv(center);
  glVertex3fv(center + (scale * dir));
}


static void dstSTPPoint(const fvec3& center, const fvec3& dir, float scale)
{
  scale = (scale == 0.0f) ? 1.0f : scale;
  glVertex3fv(center + (scale * dir));
}


static void dstSTPLine(const fvec3& center, const fvec3& dir, float scale)
{
  scale = (scale == 0.0f) ? 1.0f : scale;
  glVertex3fv(center);
  glVertex3fv(center + (scale * dir));
}


void SceneRenderer::drawLinkDebug() const
{
  static BZDB_string bzdbStr("debugLinkDraw");
  const std::string str = bzdbStr;
  if (str.empty() || (str == "0")) {
    return;
  }
  if (BZDB.isTrue("_forbidDebug")) {
    return;
  }

  const std::string::size_type npos = std::string::npos;
  const bool drawSrc = (str.find('s') != npos) || (str == "1");
  const bool drawDst = (str.find('d') != npos) || (str == "1");
  const bool drawCon = (str.find('c') != npos) || (str == "1");
  if (!drawSrc && !drawDst && !drawCon) {
    return;
  }

  const float alpha = 0.3f;
  const fvec4 colors[3] = {
    fvec4(0.8f, 0.0f, 0.0f, alpha), // red
    fvec4(0.0f, 0.6f, 0.0f, alpha), // green
    fvec4(0.0f, 0.0f, 1.0f, alpha)  // blue
  };
  const fvec4 nrmlColor(0.5f, 0.5f, 0.5f, alpha);

  OpenGLGState::resetState();

  glShadeModel(GL_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POINT_SMOOTH);
  glPointSize(4.0f);
  glLineWidth(2.49f);

  if (drawSrc) {
    const LinkManager::FaceSet& linkSrcs = linkManager.getLinkSrcSet();
    LinkManager::FaceSet::const_iterator srcIt;
    for (srcIt = linkSrcs.begin(); srcIt != linkSrcs.end(); ++srcIt) {
      const MeshFace* face = *srcIt;
      const MeshFace::SpecialData* sd = face->getSpecialData();
      const MeshFace::LinkGeometry& geo = sd->linkSrcGeo;
      const fvec3& normal = face->getPlane().xyz();
      glBegin(GL_LINES);
      glColor4fv(nrmlColor); srcSTPLine(geo.center, normal,   1.0f);
      glColor4fv(colors[0]); srcSTPLine(geo.center, geo.sDir, geo.sScale);
      glColor4fv(colors[1]); srcSTPLine(geo.center, geo.tDir, geo.tScale);
      glColor4fv(colors[2]); srcSTPLine(geo.center, geo.pDir, geo.pScale);
      glEnd();
      glBegin(GL_POINTS);
      glColor4fv(nrmlColor); srcSTPPoint(geo.center, normal,   1.0f);
      glColor4fv(colors[0]); srcSTPPoint(geo.center, geo.sDir, geo.sScale);
      glColor4fv(colors[1]); srcSTPPoint(geo.center, geo.tDir, geo.tScale);
      glColor4fv(colors[2]); srcSTPPoint(geo.center, geo.pDir, geo.pScale);
      glColor4fv(colors[0]); glVertex3fv(geo.center);
      glEnd();
    }
  }

  if (drawDst) {
    const LinkManager::FaceSet& linkDsts = linkManager.getLinkSrcSet();
    LinkManager::FaceSet::const_iterator dstIt;
    for (dstIt = linkDsts.begin(); dstIt != linkDsts.end(); ++dstIt) {
      const MeshFace* face = *dstIt;
      const MeshFace::SpecialData* sd = face->getSpecialData();
      const MeshFace::LinkGeometry& geo = sd->linkDstGeo;
      const fvec3& normal = face->getPlane().xyz();
      glBegin(GL_LINES);
      glColor4fv(nrmlColor); dstSTPLine(geo.center, normal,   1.0f);
      glColor4fv(colors[0]); dstSTPLine(geo.center, geo.sDir, geo.sScale);
      glColor4fv(colors[1]); dstSTPLine(geo.center, geo.tDir, geo.tScale);
      glColor4fv(colors[2]); dstSTPLine(geo.center, geo.pDir, geo.pScale);
      glEnd();
      glBegin(GL_POINTS);
      glColor4fv(nrmlColor); dstSTPPoint(geo.center, normal,   1.0f);
      glColor4fv(colors[0]); dstSTPPoint(geo.center, geo.sDir, geo.sScale);
      glColor4fv(colors[1]); dstSTPPoint(geo.center, geo.tDir, geo.tScale);
      glColor4fv(colors[2]); dstSTPPoint(geo.center, geo.pDir, geo.pScale);
      glColor4fv(colors[0]); glVertex3fv(geo.center);
      glEnd();
    }
  }

  glPointSize(10.0f);

  if (drawCon) {
    // load a basic 1D texture
    const float texData[8] = {
      1.0f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f
    };
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, 0);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_ALPHA, countof(texData), 0,
                 GL_ALPHA, GL_FLOAT, texData);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    const float texDist = 10.0f;
    const float texScale = (1.0f / texDist);
    const float texPeriod = 0.5f;
    const float phase = (float)fmod(GameTime::getStepTime(), (double)texPeriod);
    const float offset = phase / texPeriod;

    glBegin(GL_LINES);
    const LinkManager::LinkMap& linkMap = linkManager.getLinkMap();
    LinkManager::LinkMap::const_iterator mapIt;
    for (mapIt = linkMap.begin(); mapIt != linkMap.end(); ++mapIt) {
      const MeshFace* src = mapIt->first;
      std::set<const MeshFace*> doneFaces;
      const LinkManager::IntVec& dstIDs = mapIt->second.dstIDs;
      for (size_t d = 0; d < dstIDs.size(); d++) {
        const MeshFace* dst = linkManager.getLinkDstFace(dstIDs[d]);
        if (doneFaces.find(dst) == doneFaces.end()) {
          doneFaces.insert(dst);
          const fvec3 srcPos = src->calcCenter();
          const fvec3 dstPos = dst->calcCenter();
          const float len = (srcPos - dstPos).length();
          const float txcd0 = offset;
          const float txcd1 = offset + (len * texScale);
          glColor4fv(colors[0]); glTexCoord1f(txcd0); glVertex3fv(srcPos);
          glColor4fv(colors[1]); glTexCoord1f(txcd1); glVertex3fv(dstPos);
          if (src == dst) {
            glEnd();
            glDisable(GL_TEXTURE_1D);
            glBegin(GL_POINTS);
            const fvec4 yellow(1.0f, 1.0f, 0.0f, alpha);
            glColor4fv(yellow); glVertex3fv(src->calcCenter());
            glEnd();
            glEnable(GL_TEXTURE_1D);
            glBegin(GL_LINES);
          }
        }
      }
    }
    glEnd();

    glDisable(GL_TEXTURE_1D);
  }

  glPointSize(1.0f);
  glLineWidth(1.0f);
  glDisable(GL_POINT_SMOOTH);
  glDisable(GL_LINE_SMOOTH);
  glDisable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glShadeModel(GL_FLAT);
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
