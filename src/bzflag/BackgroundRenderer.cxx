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

// bzflag common headers
#include "common.h"
#include "global.h"

// interface header
#include "BackgroundRenderer.h"

// system impl headers
#include <stdio.h>
#include <math.h>
#include <string>

// common impl headers
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "OpenGLDisplayList.h"
#include "OpenGLMaterial.h"
#include "TextureManager.h"
#include "StateDatabase.h"
#include "BZDBCache.h"

// local impl headers
#include "daylight.h"
#include "stars.h"
#include "MainWindow.h"
#include "SceneNode.h"
#include "SceneRenderer.h"
#include "WeatherRenderer.h"

//static     bool	 useMoonTexture = false;

static const GLfloat	squareShape[4][2] =
				{ {  1.0f,  1.0f }, { -1.0f,  1.0f },
				  { -1.0f, -1.0f }, {  1.0f, -1.0f } };

GLfloat			BackgroundRenderer::skyPyramid[5][3];
const GLfloat		BackgroundRenderer::cloudRepeats = 3.0f;
static const int	NumMountainFaces = 16;

const GLfloat		BackgroundRenderer::groundColor[4][3] = {
				{ 0.0f, 0.35f, 0.0f },
				{ 0.0f, 0.20f, 0.0f },
				{ 0.0f, 0.61f, 0.0f },
				{ 0.0f, 0.35f, 0.0f }
			};
const GLfloat		BackgroundRenderer::groundColorInv[4][3] = {
				{ 0.35f, 0.00f, 0.35f },
				{ 0.20f, 0.00f, 0.20f },
				{ 0.61f, 0.00f, 0.61f },
				{ 0.35f, 0.00f, 0.35f }
			};
const GLfloat		BackgroundRenderer::receiverColor[3] =
				{ 0.3f, 0.55f, 0.3f };
const GLfloat		BackgroundRenderer::receiverColorInv[3] =
				{ 0.55f, 0.3f, 0.55f };

BackgroundRenderer::BackgroundRenderer(const SceneRenderer&) :
				blank(false),
				invert(false),
				style(0),
				gridSpacing(60.0f),	// meters
				gridCount(4.0f),
				mountainsAvailable(false),
				numMountainTextures(0),
				mountainsGState(NULL),
				mountainsList(NULL),
				cloudDriftU(0.0f),
				cloudDriftV(0.0f)
{
  static bool init = false;
  OpenGLGStateBuilder gstate;
  static const GLfloat	black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  static const GLfloat	white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  OpenGLMaterial defaultMaterial(black, black, 0.0f);
  OpenGLMaterial rainMaterial(white, white, 0.0f);

  int i;

  // initialize global to class stuff
  if (!init) {
    init = true;
    resizeSky();
  }

  TextureManager &tm = TextureManager::instance();

  // ground
  {
    // load texture for normal ground
    int groundTextureID = -1;

    if (userTextures[0].size())
      groundTextureID = tm.getTextureID( userTextures[0].c_str(), false );

    if (groundTextureID < 0)
      groundTextureID = tm.getTextureID( BZDB.get("stdGroundTexture").c_str(), true );

    // gstates
    gstate.reset();
    groundGState[0] = gstate.getState();
    gstate.reset();
    gstate.setMaterial(defaultMaterial);
    groundGState[1] = gstate.getState();
    gstate.reset();
    gstate.setTexture(groundTextureID);
    groundGState[2] = gstate.getState();
    gstate.reset();
    gstate.setMaterial(defaultMaterial);
    gstate.setTexture(groundTextureID);
    groundGState[3] = gstate.getState();

    // load texture for inverted ground
    groundTextureID = -1;
    if (userTextures[1].size())
      groundTextureID = tm.getTextureID( userTextures[1].c_str(), false );

    if (groundTextureID < 0)
      groundTextureID = tm.getTextureID( BZDB.get("zoneGroundTexture").c_str(), false );

    // gstates
    gstate.reset();
    invGroundGState[0] = gstate.getState();
    gstate.reset();
    gstate.setMaterial(defaultMaterial);
    invGroundGState[1] = gstate.getState();
    gstate.reset();
    gstate.setTexture(groundTextureID);
    invGroundGState[2] = gstate.getState();
    gstate.reset();
    gstate.setMaterial(defaultMaterial);
    gstate.setTexture(groundTextureID);
    invGroundGState[3] = gstate.getState();
  }

  // make grid stuff
  gstate.reset();
  gstate.setBlending();
  gstate.setSmoothing();
  gridGState = gstate.getState();

  // make receiver stuff
  {
    // gstates
    gstate.reset();
    gstate.setShading();
    gstate.setBlending((GLenum)GL_ONE, (GLenum)GL_ONE);
    receiverGState = gstate.getState();
  }

  // sun shadow stuff
  gstate.reset();
  gstate.setStipple(0.5f);
  gstate.setCulling((GLenum)GL_NONE);
  sunShadowsGState = gstate.getState();

 /* useMoonTexture = BZDBCache::texture && (BZDB.eval("useQuality")>2);
  int moonTexture = -1;
  if (useMoonTexture){
    moonTexture = tm.getTextureID( "moon" );
    useMoonTexture = moonTexture>= 0;
  }*/
  // sky stuff
  gstate.reset();
  gstate.setShading();
  skyGState = gstate.getState();
  gstate.reset();
  sunGState = gstate.getState();
  gstate.reset();
  gstate.setBlending((GLenum)GL_ONE, (GLenum)GL_ONE);
 // if (useMoonTexture)
 //   gstate.setTexture(*moonTexture);
  moonGState[0] = gstate.getState();
  gstate.reset();
 // if (useMoonTexture)
 //   gstate.setTexture(*moonTexture);
  moonGState[1] = gstate.getState();
  gstate.reset();
  starGState[0] = gstate.getState();
  gstate.reset();
  gstate.setBlending();
  gstate.setSmoothing();
  starGState[1] = gstate.getState();

  // make cloud stuff
  cloudsAvailable = false;
  int cloudsTexture = tm.getTextureID( "clouds" );
  if (cloudsTexture >=0) {
    cloudsAvailable = true;
    gstate.reset();
    gstate.setShading();
    gstate.setBlending((GLenum)GL_SRC_ALPHA, (GLenum)GL_ONE_MINUS_SRC_ALPHA);
    gstate.setMaterial(defaultMaterial);
    gstate.setTexture(cloudsTexture);
    gstate.setAlphaFunc();
    cloudsGState = gstate.getState();
  }

  // rain stuff
	weather.init();

  // make mountain stuff
  mountainsAvailable = false;
  {
    int mountainTexture = -1;
    int width = 0;
    int height = 0;
    numMountainTextures = 0;
    bool done = false;
    while (!done) {
      char text[256];
      sprintf (text, "mountain%d", numMountainTextures + 1);
      mountainTexture = tm.getTextureID (text, false);
      if (mountainTexture >= 0) {
	const ImageInfo & info = tm.getInfo (mountainTexture);
	height = info.y;
	width += info.x;
	numMountainTextures++;
      }
      else {
	done = true;
      }
    }

    if (numMountainTextures > 0) {
      mountainsAvailable = true;

      // prepare common gstate
      gstate.reset ();
      gstate.setShading ();
      gstate.setBlending ();
      gstate.setMaterial (defaultMaterial);
      gstate.setAlphaFunc ();

      if (numMountainTextures > 1) {
	width -= 2 * numMountainTextures;
      }
      // find power of two at least as large as height
      int scaledHeight = 1;
      while (scaledHeight < height) {
	scaledHeight <<= 1;
      }

      // choose minimum width
      int minWidth = scaledHeight;
      if (minWidth > scaledHeight) {
	minWidth = scaledHeight;
      }
      mountainsMinWidth = minWidth;

      // prepare each texture
      mountainsGState = new OpenGLGState[numMountainTextures];
      for (i = 0; i < numMountainTextures; i++) {
	char text[256];
	sprintf (text, "mountain%d", i + 1);
	gstate.setTexture (tm.getTextureID (text));
	mountainsGState[i] = gstate.getState ();
      }
      mountainsList = new OpenGLDisplayList[numMountainTextures];
    }
  }

  // create display lists
  doInitDisplayLists();

  // recreate display lists when context is recreated
  OpenGLGState::registerContextInitializer(initDisplayLists, (void*)this);

  notifyStyleChange();
}

BackgroundRenderer::~BackgroundRenderer()
{
  OpenGLGState::unregisterContextInitializer(initDisplayLists, (void*)this);
  delete[] mountainsGState;
  delete[] mountainsList;
}

void			BackgroundRenderer::notifyStyleChange()
{
  if (BZDBCache::texture) {
    if (BZDBCache::lighting)
      styleIndex = 3;
    else
      styleIndex = 2;
  } else {
    if (BZDBCache::lighting)
      styleIndex = 1;
    else
      styleIndex = 0;
  }

  // some stuff is drawn only for certain states
  cloudsVisible = (styleIndex >= 2 && cloudsAvailable && BZDBCache::blend);
  mountainsVisible = (styleIndex >= 2 && mountainsAvailable);
  shadowsVisible = BZDB.isTrue("shadows");
  starGStateIndex = BZDB.isTrue("smooth");

  // fixup gstates
  OpenGLGStateBuilder gstate;
  gstate.reset();
  if (BZDB.isTrue("smooth")) {
    gstate.setBlending();
    gstate.setSmoothing();
  }
  gridGState = gstate.getState();
}

void		BackgroundRenderer::resize() {
  resizeSky();
  doInitDisplayLists();
}


void			BackgroundRenderer::setCelestial(
				const SceneRenderer& renderer,
				const float sunDir[3], const float moonDir[3])
{
  // save sun and moon positions
  sunDirection[0] = sunDir[0];
  sunDirection[1] = sunDir[1];
  sunDirection[2] = sunDir[2];
  moonDirection[0] = moonDir[0];
  moonDirection[1] = moonDir[1];
  moonDirection[2] = moonDir[2];

  // change sky colors according to the sun position
  GLfloat colors[4][3];
  getSkyColor(sunDirection, colors);
  skyZenithColor[0] = colors[0][0];
  skyZenithColor[1] = colors[0][1];
  skyZenithColor[2] = colors[0][2];
  skySunDirColor[0] = colors[1][0];
  skySunDirColor[1] = colors[1][1];
  skySunDirColor[2] = colors[1][2];
  skyAntiSunDirColor[0] = colors[2][0];
  skyAntiSunDirColor[1] = colors[2][1];
  skyAntiSunDirColor[2] = colors[2][2];
  skyCrossSunDirColor[0] = colors[3][0];
  skyCrossSunDirColor[1] = colors[3][1];
  skyCrossSunDirColor[2] = colors[3][2];

  // get a few other things concerning the sky
  doShadows = areShadowsCast(sunDirection);
  doStars = areStarsVisible(sunDirection);
  doSunset = getSunsetTop(sunDirection, sunsetTop);

  // make pretransformed display list for sun
  sunXFormList.begin();
    glPushMatrix();
    glRotatef((GLfloat)(atan2f(sunDirection[1], (sunDirection[0])) * 180.0 / M_PI),
							0.0f, 0.0f, 1.0f);
    glRotatef((GLfloat)(asinf(sunDirection[2]) * 180.0 / M_PI), 0.0f, -1.0f, 0.0f);
    sunList.execute();
    glPopMatrix();
  sunXFormList.end();

  // compute display list for moon
  float coverage = moonDir[0] * sunDir[0] +
				moonDir[1] * sunDir[1] +
				moonDir[2] * sunDir[2];
  // hack coverage to lean towards full
  coverage = (coverage < 0.0f) ? -sqrtf(-coverage) : coverage * coverage;
  float worldSize = BZDBCache::worldSize;
  const float moonRadius = 2.0f * worldSize *
				atanf((float)((60.0 * M_PI / 180.0) / 60.0));
  // limbAngle is dependent on moon position but sun is so much farther
  // away that the moon's position is negligible.  rotate sun and moon
  // so that moon is on the horizon in the +x direction, then compute
  // the angle to the sun position in the yz plane.
  float sun2[3];
  const float moonAzimuth = atan2f(moonDirection[1], moonDirection[0]);
  const float moonAltitude = asinf(moonDirection[2]);
  sun2[0] = sunDir[0] * cosf(moonAzimuth) + sunDir[1] * sinf(moonAzimuth);
  sun2[1] = sunDir[1] * cosf(moonAzimuth) - sunDir[0] * sinf(moonAzimuth);
  sun2[2] = sunDir[2] * cosf(moonAltitude) - sun2[0] * sinf(moonAltitude);
  const float limbAngle = atan2f(sun2[2], sun2[1]);

  int moonSegements = (int)BZDB.eval("moonSegments");
  moonList.begin();
    glPushMatrix();
    glRotatef((GLfloat)(atan2f(moonDirection[1], moonDirection[0]) * 180.0 / M_PI),
							0.0f, 0.0f, 1.0f);
    glRotatef((GLfloat)(asinf(moonDirection[2]) * 180.0 / M_PI), 0.0f, -1.0f, 0.0f);
    glRotatef((float)(limbAngle * 180.0 / M_PI), 1.0f, 0.0f, 0.0f);
    glBegin(GL_TRIANGLE_STRIP);
    // glTexCoord2f(0,-1);
    glVertex3f(2.0f * worldSize, 0.0f, -moonRadius);
      for (int i = 0; i < moonSegements-1; i++) {
	const float angle = (float)(0.5 * M_PI * double(i-(moonSegements/2)-1) / (moonSegements/2.0));
	float sinAngle = sinf(angle);
	float cosAngle = cosf(angle);
	// glTexCoord2f(coverage*cosAngle,sinAngle);
	glVertex3f(2.0f * worldSize, coverage * moonRadius * cosAngle,moonRadius * sinAngle);

	// glTexCoord2f(cosAngle,sinAngle);
	glVertex3f(2.0f * worldSize, moonRadius * cosAngle,moonRadius * sinAngle);
      }
    // glTexCoord2f(0,1);
    glVertex3f(2.0f * worldSize, 0.0f, moonRadius);
    glEnd();
    glPopMatrix();
  moonList.end();

  // make pretransformed display list for stars
  starXFormList.begin();
    glPushMatrix();
    glMultMatrixf(renderer.getCelestialTransform());
    glScalef(worldSize, worldSize, worldSize);
    starList.execute();
    glPopMatrix();
  starXFormList.end();

}


void BackgroundRenderer::addCloudDrift(GLfloat uDrift, GLfloat vDrift)
{
  cloudDriftU += 0.01f * uDrift / cloudRepeats;
  cloudDriftV += 0.01f * vDrift / cloudRepeats;
  if (cloudDriftU > 1.0f) cloudDriftU -= 1.0f;
  else if (cloudDriftU < 0.0f) cloudDriftU += 1.0f;
  if (cloudDriftV > 1.0f) cloudDriftV -= 1.0f;
  else if (cloudDriftV < 0.0f) cloudDriftV += 1.0f;
}


void BackgroundRenderer::renderSky(SceneRenderer& renderer, bool fullWindow,
				   bool mirror)
{
  if (renderer.useQuality() > 0) {
    drawSky(renderer, mirror);
  } else {
    // low detail -- draw as damn fast as ya can, ie cheat.  use glClear()
    // to draw solid color sky and ground.
    MainWindow& window = renderer.getWindow();
    const int x = window.getOriginX();
    const int y = window.getOriginY();
    const int width = window.getWidth();
    const int height = window.getHeight();
    const int viewHeight = window.getViewHeight();
    const SceneRenderer::ViewType viewType = renderer.getViewType();

    // draw sky
    glDisable(GL_DITHER);
    glPushAttrib(GL_SCISSOR_BIT);
    glScissor(x, y + height - (viewHeight >> 1), width, (viewHeight >> 1));
    glClearColor(skyZenithColor[0], skyZenithColor[1], skyZenithColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw ground -- first get the color (assume it's all green)
    GLfloat groundColor = 0.1f + 0.15f * renderer.getSunColor()[1];
    if (fullWindow && viewType == SceneRenderer::ThreeChannel)
      glScissor(x, y, width, height >> 1);
    else if (fullWindow && viewType == SceneRenderer::Stacked)
      glScissor(x, y, width, height >> 1);
#ifndef USE_GL_STEREO
    else if (fullWindow && viewType == SceneRenderer::Stereo)
      glScissor(x, y, width, height >> 1);
#endif
    else
      glScissor(x, y + height - viewHeight, width, (viewHeight + 1) >> 1);
    if (invert) glClearColor(groundColor, 0.0f, groundColor, 0.0f);
    else glClearColor(0.0f, groundColor, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // back to normal
    glPopAttrib();
    if (BZDB.isTrue("dither")) glEnable(GL_DITHER);
  }
}


void BackgroundRenderer::renderGround(SceneRenderer& renderer,
				      bool fullWindow)
{
  if (renderer.useQuality() > 0) {
    drawGround();
  } else {
    // low detail -- draw as damn fast as ya can, ie cheat.  use glClear()
    // to draw solid color sky and ground.
    MainWindow& window = renderer.getWindow();
    const int x = window.getOriginX();
    const int y = window.getOriginY();
    const int width = window.getWidth();
    const int height = window.getHeight();
    const int viewHeight = window.getViewHeight();
    const SceneRenderer::ViewType viewType = renderer.getViewType();

    // draw sky
    glDisable(GL_DITHER);
    glPushAttrib(GL_SCISSOR_BIT);
    glScissor(x, y + height - (viewHeight >> 1), width, (viewHeight >> 1));
    glClearColor(skyZenithColor[0], skyZenithColor[1], skyZenithColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw ground -- first get the color (assume it's all green)
    GLfloat groundColor = 0.1f + 0.15f * renderer.getSunColor()[1];
    if (fullWindow && viewType == SceneRenderer::ThreeChannel)
      glScissor(x, y, width, height >> 1);
    else if (fullWindow && viewType == SceneRenderer::Stacked)
      glScissor(x, y, width, height >> 1);
#ifndef USE_GL_STEREO
    else if (fullWindow && viewType == SceneRenderer::Stereo)
      glScissor(x, y, width, height >> 1);
#endif
    else
      glScissor(x, y + height - viewHeight, width, (viewHeight + 1) >> 1);
    if (invert) glClearColor(groundColor, 0.0f, groundColor, 0.0f);
    else glClearColor(0.0f, groundColor, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // back to normal
    glPopAttrib();
    if (BZDB.isTrue("dither")) glEnable(GL_DITHER);
  }
}


void BackgroundRenderer::renderGroundEffects(SceneRenderer& renderer,
					     bool drawingMirror)
{
  // zbuffer should be disabled.  either everything is coplanar with
  // the ground or is drawn back to front and is occluded by everything
  // drawn after it.  also use projection with very far clipping plane.

  if ((renderer.useQuality() < 3) && !drawingMirror) {
    drawGroundGrid(renderer);
  }

  if (!blank) {
    if (doShadows && shadowsVisible && !drawingMirror) {
      drawGroundShadows(renderer);
    }

    // draw light receivers on ground (little meshes under light sources so
    // the ground gets illuminated).  this is necessary because lighting is
    // performed only at a vertex, and the ground's vertices are a few
    // kilometers away.
    if (BZDBCache::blend && BZDBCache::lighting && !drawingMirror) {
      drawGroundReceivers(renderer);
    }

    if (renderer.useQuality() > 1) {
      // light the mountains (so that they get dark when the sun goes down).
      // don't do zbuffer test since they occlude all drawn before them and
      // are occluded by all drawn after.
      if (mountainsVisible) drawMountains();

      // draw clouds
      if (cloudsVisible) {
	cloudsGState.setState();
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glTranslatef(cloudDriftU, cloudDriftV, 0.0f);
	cloudsList.execute();
	glLoadIdentity();	// maybe works around bug in some systems
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
      }
    }
  }
}


void BackgroundRenderer::renderEnvironment(SceneRenderer& renderer)
{
  if (renderer.useQuality() < 3)
    return;

  if (!blank) {
    weather.update();
    weather.draw(renderer);
  }
}


void BackgroundRenderer::resizeSky() {
  // sky pyramid must fit inside far clipping plane
  // (adjusted for the deepProjection matrix)
  const GLfloat skySize = 3.0f * BZDBCache::worldSize;
  for (int i = 0; i < 4; i++) {
    skyPyramid[i][0] = skySize * squareShape[i][0];
    skyPyramid[i][1] = skySize * squareShape[i][1];
    skyPyramid[i][2] = 0.0f;
  }
  skyPyramid[4][0] = 0.0f;
  skyPyramid[4][1] = 0.0f;
  skyPyramid[4][2] = skySize;
}


void BackgroundRenderer::drawSky(SceneRenderer& renderer, bool mirror)
{
  // rotate sky so that horizon-point-toward-sun-color is actually
  // toward the sun
  glPushMatrix();
  glRotatef((GLfloat)((atan2f(sunDirection[1], sunDirection[0]) * 180.0 + 135.0) / M_PI),
							0.0f, 0.0f, 1.0f);

  // draw sky
  skyGState.setState();
  if (!doSunset) {
    // just a pyramid
    glBegin(GL_TRIANGLE_FAN);
      glColor3fv(skyZenithColor);
      glVertex3fv(skyPyramid[4]);
      glColor3fv(skyCrossSunDirColor);
      glVertex3fv(skyPyramid[0]);
      glColor3fv(skySunDirColor);
      glVertex3fv(skyPyramid[3]);
      glColor3fv(skyCrossSunDirColor);
      glVertex3fv(skyPyramid[2]);
      glColor3fv(skyAntiSunDirColor);
      glVertex3fv(skyPyramid[1]);
      glColor3fv(skyCrossSunDirColor);
      glVertex3fv(skyPyramid[0]);
    glEnd();
  }
  else {
    // overall shape is a pyramid, but the solar sides are two
    // triangles each.  the top triangle is all zenith color,
    // the bottom goes from zenith to sun-dir color.
    glBegin(GL_TRIANGLE_FAN);
      glColor3fv(skyZenithColor);
      glVertex3fv(skyPyramid[4]);
      glColor3fv(skyCrossSunDirColor);
      glVertex3fv(skyPyramid[2]);
      glColor3fv(skyAntiSunDirColor);
      glVertex3fv(skyPyramid[1]);
      glColor3fv(skyCrossSunDirColor);
      glVertex3fv(skyPyramid[0]);
    glEnd();

    GLfloat sunsetTopPoint[3];
    sunsetTopPoint[0] = skyPyramid[3][0] * (1.0f - sunsetTop);
    sunsetTopPoint[1] = skyPyramid[3][1] * (1.0f - sunsetTop);
    sunsetTopPoint[2] = skyPyramid[4][2] * sunsetTop;
    glBegin(GL_TRIANGLES);
      glColor3fv(skyZenithColor);
      glVertex3fv(skyPyramid[4]);
      glColor3fv(skyCrossSunDirColor);
      glVertex3fv(skyPyramid[0]);
      glColor3fv(skyZenithColor);
      glVertex3fv(sunsetTopPoint);
      glVertex3fv(skyPyramid[4]);
      glVertex3fv(sunsetTopPoint);
      glColor3fv(skyCrossSunDirColor);
      glVertex3fv(skyPyramid[2]);
      glColor3fv(skyZenithColor);
      glVertex3fv(sunsetTopPoint);
      glColor3fv(skyCrossSunDirColor);
      glVertex3fv(skyPyramid[0]);
      glColor3fv(skySunDirColor);
      glVertex3fv(skyPyramid[3]);
      glColor3fv(skyCrossSunDirColor);
      glVertex3fv(skyPyramid[2]);
      glColor3fv(skyZenithColor);
      glVertex3fv(sunsetTopPoint);
      glColor3fv(skySunDirColor);
      glVertex3fv(skyPyramid[3]);
    glEnd();
  }

  glLoadIdentity();
  renderer.getViewFrustum().executeOrientation();

  if (mirror) {
    glEnable(GL_CLIP_PLANE0);
    const GLdouble plane[4] = {0.0, 0.0, +1.0, 0.0};
    glClipPlane(GL_CLIP_PLANE0, plane);
  }

  if (sunDirection[2] > -0.009f) {
    sunGState.setState();
    glColor3fv(renderer.getSunScaledColor());
    sunXFormList.execute();
  }

  if (doStars) {
    starGState[starGStateIndex].setState();
    starXFormList.execute();
  }

  if (moonDirection[2] > -0.009f) {
    moonGState[doStars ? 1 : 0].setState();
    glColor3f(1.0f, 1.0f, 1.0f);
 //   if (useMoonTexture)
 //     glEnable(GL_TEXTURE_2D);
    moonList.execute();
  }

  if (mirror) {
    glDisable(GL_CLIP_PLANE0);
  }

  glPopMatrix();
}


void BackgroundRenderer::drawGround()
{
  // draw ground
  glNormal3f(0.0f, 0.0f, 1.0f);
  if (invert) {
    if (BZDBCache::texture) {
      glColor3f(1.0f, 1.0f, 1.0f);
    } else {
      glColor3fv(groundColorInv[styleIndex]);
    }
    invGroundGState[styleIndex].setState();
  } else {
    if (BZDBCache::texture) {
      glColor3f(1.0f, 1.0f, 1.0f);
    }
    else if (BZDB.isSet("GroundOverideColor")) {
      float color[3];
      sscanf(BZDB.get("GroundOverideColor").c_str(),"%f %f %f",
	     &color[0], &color[1], &color[2]);
      glColor3fv(color);
    }
    else {
      glColor3fv(groundColor[styleIndex]);
    }
    groundGState[styleIndex].setState();
  }

  if (BZDB.evalInt("useQuality") >= 3) {
    drawGroundCentered();
  } else {
    simpleGroundList[styleIndex].execute();
  }
}

void BackgroundRenderer::drawGroundCentered()
{
  const ViewFrustum& frustum = RENDERER.getViewFrustum();
  const float* center = frustum.getEye();

  const float groundSize = 10.0f * BZDBCache::worldSize;
  const float repeat = BZDB.eval("groundHighResTexRepeat");

  // vertices
  const float vXmin = -groundSize;
  const float vXmax = +groundSize;
  const float vYmin = -groundSize;
  const float vYmax = +groundSize;
  const GLfloat vertices[5][2] = {
    {center[0], center[1]},
    {vXmin, vYmin}, {vXmax, vYmin}, {vXmax, vYmax}, {vXmin, vYmax}
  };

  // texcoords
  const float tcenterX = center[0] * repeat;
  const float tcenterY = center[1] * repeat;
  const float tXmin = -groundSize * repeat;
  const float tXmax = +groundSize * repeat;
  const float tYmin = -groundSize * repeat;
  const float tYmax = +groundSize * repeat;
  const GLfloat texcoords[5][2] = {
    {tcenterX, tcenterY},
    {tXmin, tYmin}, {tXmax, tYmin}, {tXmax, tYmax}, {tXmin, tYmax}
  };

  const GLubyte fan[6] = { 0, 1, 2, 3, 4, 1};

  glVertexPointer(2, GL_FLOAT, 0, vertices);
  glEnableClientState(GL_VERTEX_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glDrawElements(GL_TRIANGLE_FAN, 6, GL_UNSIGNED_BYTE, fan);

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  return;
}


void			BackgroundRenderer::drawGroundGrid(
						SceneRenderer& renderer)
{
  const GLfloat* pos = renderer.getViewFrustum().getEye();
  const GLfloat xhalf = gridSpacing * (gridCount + floorf(pos[2] / 4.0f));
  const GLfloat yhalf = gridSpacing * (gridCount + floorf(pos[2] / 4.0f));
  const GLfloat x0 = floorf(pos[0] / gridSpacing) * gridSpacing;
  const GLfloat y0 = floorf(pos[1] / gridSpacing) * gridSpacing;
  GLfloat i;

  gridGState.setState();

  // x lines
  if (doShadows) glColor3f(0.0f, 0.75f, 0.5f);
  else glColor3f(0.0f, 0.4f, 0.3f);
  glBegin(GL_LINES);
    for (i = -xhalf; i <= xhalf; i += gridSpacing) {
      glVertex2f(x0 + i, y0 - yhalf);
      glVertex2f(x0 + i, y0 + yhalf);
    }
  glEnd();

  /* z lines */
  if (doShadows) glColor3f(0.5f, 0.75f, 0.0f);
  else glColor3f(0.3f, 0.4f, 0.0f);
  glBegin(GL_LINES);
    for (i = -yhalf; i <= yhalf; i += gridSpacing) {
      glVertex2f(x0 - xhalf, y0 + i);
      glVertex2f(x0 + xhalf, y0 + i);
    }
  glEnd();
}

void			BackgroundRenderer::drawGroundShadows(
						SceneRenderer& renderer)
{
  // draw sun shadows -- always stippled so overlapping shadows don't
  // accumulate darkness.  make and multiply by shadow projection matrix.
  GLfloat shadowProjection[16];
  shadowProjection[0] = shadowProjection[5] = shadowProjection[15] = 1.0f;
  shadowProjection[8] = -sunDirection[0] / sunDirection[2];
  shadowProjection[9] = -sunDirection[1] / sunDirection[2];
  shadowProjection[1] = shadowProjection[2] =
  shadowProjection[3] = shadowProjection[4] =
  shadowProjection[6] = shadowProjection[7] =
  shadowProjection[10] = shadowProjection[11] =
  shadowProjection[12] = shadowProjection[13] =
  shadowProjection[14] = 0.0f;
  glPushMatrix();
  glMultMatrixf(shadowProjection);

  // disable color updates
  SceneNode::setColorOverride(true);

  sunShadowsGState.setState();
  glColor3f(0.0f, 0.0f, 0.0f);
  renderer.getShadowList().render();

  // enable color updates
  SceneNode::setColorOverride(false);

  OpenGLGState::resetState();

  glPopMatrix();
}


void BackgroundRenderer::drawGroundReceivers(SceneRenderer& renderer)
{
  static const int receiverRings = 4;
  static const int receiverSlices = 8;
  static const float receiverRingSize = 1.2f;	// meters
  static float angle[receiverSlices + 1][2];
  static bool init = false;

  if (!init) {
    init = true;
    const float receiverSliceAngle = (float)(2.0 * M_PI / double(receiverSlices));
    for (int i = 0; i <= receiverSlices; i++) {
      angle[i][0] = cosf((float)i * receiverSliceAngle);
      angle[i][1] = sinf((float)i * receiverSliceAngle);
    }
  }

  const int count = renderer.getNumAllLights();
  if (count == 0) {
    return;
  }

  // bright sun dims intensity of ground receivers
  const float B = 1.0f - (0.6f * renderer.getSunBrightness());

  receiverGState.setState();
  glPushMatrix();
  int i, j;
  for (int k = 0; k < count; k++) {
    const OpenGLLight& light = renderer.getLight(k);
    if (light.getOnlyReal()) {
      continue;
    }

    const GLfloat* pos = light.getPosition();
    const GLfloat* lightColor = light.getColor();
    const GLfloat* atten = light.getAttenuation();

    // point under light
    float d = pos[2];
    float I = B / (atten[0] + d * (atten[1] + d * atten[2]));

    // if I is too low, don't bother drawing anything
    if (I < 0.02f) {
      continue;
    }

    // move to the light's position
    glTranslatef(pos[0], pos[1], 0.0f);

    // modulate light color by ground color
    float color[3];
    if (invert) {
      color[0] = receiverColorInv[0] * lightColor[0];
      color[1] = receiverColorInv[1] * lightColor[1];
      color[2] = receiverColorInv[2] * lightColor[2];
    } else {
      color[0] = receiverColor[0] * lightColor[0];
      color[1] = receiverColor[1] * lightColor[1];
      color[2] = receiverColor[2] * lightColor[2];
    }

    // draw ground receiver, computing lighting at each vertex ourselves
    glBegin(GL_TRIANGLE_FAN);
    {
      glColor3f(I * color[0] > 1.0f ? 1.0f : I * color[0],
		I * color[1] > 1.0f ? 1.0f : I * color[1],
		I * color[2] > 1.0f ? 1.0f : I * color[2]);
      glVertex2f(0.0f, 0.0f);

      // inner ring
      d = receiverRingSize + pos[2];
      I = B / (atten[0] + d * (atten[1] + d * atten[2]));
      I *= pos[2] / hypotf(receiverRingSize, pos[2]);
      glColor3f(I * color[0] > 1.0f ? 1.0f : I * color[0],
		I * color[1] > 1.0f ? 1.0f : I * color[1],
		I * color[2] > 1.0f ? 1.0f : I * color[2]);
      for (j = 0; j <= receiverSlices; j++) {
	glVertex2f(receiverRingSize * angle[j][0],
		   receiverRingSize * angle[j][1]);
      }
    }
    glEnd();

    for (i = 1; i < receiverRings; i++) {
      const GLfloat innerSize = receiverRingSize * GLfloat(i * i);
      const GLfloat outerSize = receiverRingSize * GLfloat((i + 1) * (i + 1));

      // compute inner and outer lit colors
      float d = innerSize + pos[2];
      float I = B / (atten[0] + d * (atten[1] + d * atten[2]));
      I *= pos[2] / hypotf(innerSize, pos[2]);
      float innerColor[3];
      innerColor[0] = I * color[0];
      innerColor[1] = I * color[1];
      innerColor[2] = I * color[2];
      if (innerColor[0] > 1.0f) innerColor[0] = 1.0f;
      if (innerColor[1] > 1.0f) innerColor[1] = 1.0f;
      if (innerColor[2] > 1.0f) innerColor[2] = 1.0f;

      if (i + 1 == receiverRings) {
	I = 0.0f;
      } else {
	d = outerSize + pos[2];
	I = B / (atten[0] + d * (atten[1] + d * atten[2]));
	I *= pos[2] / hypotf(outerSize, pos[2]);
      }
      float outerColor[3];
      outerColor[0] = I * color[0];
      outerColor[1] = I * color[1];
      outerColor[2] = I * color[2];
      if (outerColor[0] > 1.0f) outerColor[0] = 1.0f;
      if (outerColor[1] > 1.0f) outerColor[1] = 1.0f;
      if (outerColor[2] > 1.0f) outerColor[2] = 1.0f;

      glBegin(GL_QUAD_STRIP);
      {
	for (j = 0; j <= receiverSlices; j++) {
	  glColor3fv(innerColor);
	  glVertex2f(angle[j][0] * innerSize, angle[j][1] * innerSize);
	  glColor3fv(outerColor);
	  glVertex2f(angle[j][0] * outerSize, angle[j][1] * outerSize);
	}
      }
      glEnd();
    }

    glTranslatef(-pos[0], -pos[1], 0.0f);
  }
  glPopMatrix();
}


void BackgroundRenderer::drawMountains(void)
{
  glColor3f(1.0f, 1.0f, 1.0f);
  for (int i = 0; i < numMountainTextures; i++) {
    mountainsGState[i].setState();
    mountainsList[i].execute();
  }
}

extern SceneRenderer*	getSceneRenderer();

void			BackgroundRenderer::doInitDisplayLists()
{
  int i, j;
  SceneRenderer& renderer = *(getSceneRenderer());

	weather.rebuildContext();

  // need some workarounds on RIVA 128
  bool isRiva128 = (strncmp((const char*)glGetString(GL_RENDERER),
						"RIVA 128", 8) == 0);

  //
  // sky stuff
  //

  // sun first.  sun is a disk that should be about a half a degree wide
  // with a normal (60 degree) perspective.
  const float worldSize = BZDBCache::worldSize;
  const float sunRadius = (float)(2.0 * worldSize * atanf((float)(60.0*M_PI/180.0)) / 60.0);
  sunList.begin();
    glBegin(GL_TRIANGLE_FAN);
      glVertex3f(2.0f * worldSize, 0.0f, 0.0f);
      for (i = 0; i < 20; i++) {
	const float angle = (float)(2.0 * M_PI * double(i) / 19.0);
	glVertex3f(2.0f * worldSize, sunRadius * sinf(angle),
					sunRadius * cosf(angle));
      }
    glEnd();
  sunList.end();

  // make (empty) moon list
  moonList.begin();
  moonList.end();

  // make stars list
  starList.begin();
    glBegin(GL_POINTS);
    for (i = 0; i < (int)NumStars; i++) {
      glColor3fv(stars[i]);
      glVertex3fv(stars[i] + 3);
    }
    glEnd();
  starList.end();

  //
  // ground
  //

  // RIVA 128 can't repeat texture uv's too much.  if we're using one
  // of those, only texture the ground inside the game area.
  float uv[2];
  const GLfloat groundSize = 10.0f * worldSize;
  const GLfloat gameSize = 0.5f * worldSize;
  GLfloat groundPlane[4][3];
  GLfloat gameArea[4][3];
  for (i = 0; i < 4; i++) {
    groundPlane[i][0] = groundSize * squareShape[i][0];
    groundPlane[i][1] = groundSize * squareShape[i][1];
    groundPlane[i][2] = 0.0f;
    gameArea[i][0] = gameSize * squareShape[i][0];
    gameArea[i][1] = gameSize * squareShape[i][1];
    gameArea[i][2] = 0.0f;
  }
  if (isRiva128) {
    simpleGroundList[2].begin();
      glBegin(GL_TRIANGLE_STRIP);
	renderer.getGroundUV(gameArea[0], uv);
	glTexCoord2f(uv[0], uv[1]);
	glVertex2fv(gameArea[0]);
	renderer.getGroundUV(gameArea[1], uv);
	glTexCoord2f(uv[0], uv[1]);
	glVertex2fv(gameArea[1]);
	renderer.getGroundUV(gameArea[3], uv);
	glTexCoord2f(uv[0], uv[1]);
	glVertex2fv(gameArea[3]);
	renderer.getGroundUV(gameArea[2], uv);
	glTexCoord2f(uv[0], uv[1]);
	glVertex2fv(gameArea[2]);
      glEnd();

      glTexCoord2f(0.0f, 0.0f);
      glBegin(GL_TRIANGLE_STRIP);
	glVertex2fv(gameArea[0]);
	glVertex2fv(groundPlane[0]);

	glVertex2fv(gameArea[1]);
	glVertex2fv(groundPlane[1]);

	glVertex2fv(gameArea[2]);
	glVertex2fv(groundPlane[2]);

	glVertex2fv(gameArea[3]);
	glVertex2fv(groundPlane[3]);

	glVertex2fv(gameArea[0]);
	glVertex2fv(groundPlane[0]);
      glEnd();
    simpleGroundList[2].end();
  } else {
    int i, j;
    GLfloat xmin, xmax;
    GLfloat ymin, ymax;
    GLfloat xdist, ydist;
    GLfloat xtexmin, xtexmax;
    GLfloat ytexmin, ytexmax;
    GLfloat xtexdist, ytexdist;
    float vec[2];

#define GROUND_DIVS	(4)	//FIXME -- seems to be enough

    xmax = groundPlane[0][0];
    ymax = groundPlane[0][1];
    xmin = groundPlane[2][0];
    ymin = groundPlane[2][1];
    xdist = (xmax - xmin) / (float)GROUND_DIVS;
    ydist = (ymax - ymin) / (float)GROUND_DIVS;

    renderer.getGroundUV (groundPlane[0], vec);
    xtexmax = vec[0];
    ytexmax = vec[1];
    renderer.getGroundUV (groundPlane[2], vec);
    xtexmin = vec[0];
    ytexmin = vec[1];
    xtexdist = (xtexmax - xtexmin) / (float)GROUND_DIVS;
    ytexdist = (ytexmax - ytexmin) / (float)GROUND_DIVS;

    simpleGroundList[2].begin();

      for (i = 0; i < GROUND_DIVS; i++) {
	GLfloat yoff, ytexoff;

	yoff = ymin + ydist * (GLfloat)i;
	ytexoff = ytexmin + ytexdist * (GLfloat)i;

	glBegin(GL_TRIANGLE_STRIP);

	glTexCoord2f(xtexmin, ytexoff + ytexdist);
	glVertex2f(xmin, yoff + ydist);
	glTexCoord2f(xtexmin, ytexoff);
	glVertex2f(xmin, yoff);

	for (j = 0; j < GROUND_DIVS; j++) {
	  GLfloat xoff, xtexoff;

	  xoff = xmin + xdist * (GLfloat)(j + 1);
	  xtexoff = xtexmin + xtexdist * (GLfloat)(j + 1);

	  glTexCoord2f(xtexoff, ytexoff + ytexdist);
	  glVertex2f(xoff, yoff + ydist);
	  glTexCoord2f(xtexoff, ytexoff);
	  glVertex2f(xoff, yoff);
	}
	glEnd();
      }

    simpleGroundList[2].end();
  }

  simpleGroundList[0].begin();
    glBegin(GL_TRIANGLE_STRIP);
      glVertex2fv(groundPlane[0]);
      glVertex2fv(groundPlane[1]);
      glVertex2fv(groundPlane[3]);
      glVertex2fv(groundPlane[2]);
    glEnd();
  simpleGroundList[0].end();
  simpleGroundList[1] = simpleGroundList[0];
  simpleGroundList[3] = simpleGroundList[2];

  //
  // clouds
  //

  if (cloudsAvailable) {
    // make vertices for cloud polygons
    GLfloat cloudsOuter[4][3], cloudsInner[4][3];
    const GLfloat uvScale = 0.25f;
    for (i = 0; i < 4; i++) {
      cloudsOuter[i][0] = groundPlane[i][0];
      cloudsOuter[i][1] = groundPlane[i][1];
      cloudsOuter[i][2] = groundPlane[i][2] + 120.0f * BZDBCache::tankHeight;
      cloudsInner[i][0] = uvScale * cloudsOuter[i][0];
      cloudsInner[i][1] = uvScale * cloudsOuter[i][1];
      cloudsInner[i][2] = cloudsOuter[i][2];
    }

    // make cloud display list.  RIVA 128 doesn't interpolate alpha,
    // so on that system use full alpha everywhere.
    GLfloat minAlpha = 0.0f;
    if (isRiva128)
      minAlpha = 1.0f;
    cloudsList.begin();
      glNormal3f(0.0f, 0.0f, 1.0f);
      // inner clouds -- full opacity
      glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2f(uvScale * cloudRepeats * squareShape[3][0],
		     uvScale * cloudRepeats * squareShape[3][1]);
	glVertex3fv(cloudsInner[3]);
	glTexCoord2f(uvScale * cloudRepeats * squareShape[2][0],
		     uvScale * cloudRepeats * squareShape[2][1]);
	glVertex3fv(cloudsInner[2]);
	glTexCoord2f(uvScale * cloudRepeats * squareShape[1][0],
		     uvScale * cloudRepeats * squareShape[1][1]);
	glVertex3fv(cloudsInner[1]);
	glTexCoord2f(uvScale * cloudRepeats * squareShape[0][0],
		     uvScale * cloudRepeats * squareShape[0][1]);
	glVertex3fv(cloudsInner[0]);
      glEnd();

      // outer clouds -- fade to zero opacity at outer edge
      glBegin(GL_TRIANGLE_STRIP);
	glColor4f(1.0f, 1.0f, 1.0f, minAlpha);
	glTexCoord2f(cloudRepeats * squareShape[1][0],
		     cloudRepeats * squareShape[1][1]);
	glVertex3fv(cloudsOuter[1]);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2f(uvScale * cloudRepeats * squareShape[1][0],
		     uvScale * cloudRepeats * squareShape[1][1]);
	glVertex3fv(cloudsInner[1]);

	glColor4f(1.0f, 1.0f, 1.0f, minAlpha);
	glTexCoord2f(cloudRepeats * squareShape[2][0],
		     cloudRepeats * squareShape[2][1]);
	glVertex3fv(cloudsOuter[2]);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2f(uvScale * cloudRepeats * squareShape[2][0],
		     uvScale * cloudRepeats * squareShape[2][1]);
	glVertex3fv(cloudsInner[2]);

	glColor4f(1.0f, 1.0f, 1.0f, minAlpha);
	glTexCoord2f(cloudRepeats * squareShape[3][0],
		     cloudRepeats * squareShape[3][1]);
	glVertex3fv(cloudsOuter[3]);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2f(uvScale * cloudRepeats * squareShape[3][0],
		     uvScale * cloudRepeats * squareShape[3][1]);
	glVertex3fv(cloudsInner[3]);

	glColor4f(1.0f, 1.0f, 1.0f, minAlpha);
	glTexCoord2f(cloudRepeats * squareShape[0][0],
		     cloudRepeats * squareShape[0][1]);
	glVertex3fv(cloudsOuter[0]);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2f(uvScale * cloudRepeats * squareShape[0][0],
		     uvScale * cloudRepeats * squareShape[0][1]);
	glVertex3fv(cloudsInner[0]);

	glColor4f(1.0f, 1.0f, 1.0f, minAlpha);
	glTexCoord2f(cloudRepeats * squareShape[1][0],
		     cloudRepeats * squareShape[1][1]);
	glVertex3fv(cloudsOuter[1]);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2f(uvScale * cloudRepeats * squareShape[1][0],
		     uvScale * cloudRepeats * squareShape[1][1]);
	glVertex3fv(cloudsInner[1]);
      glEnd();
    cloudsList.end();
  }

  //
  // mountains
  //

  if (numMountainTextures > 0) {
    // prepare display lists.  need at least NumMountainFaces, but
    // we also need a multiple of the number of subtextures.  put
    // all the faces using a given texture into the same list.
    const int numFacesPerTexture = (NumMountainFaces +
				numMountainTextures - 1) / numMountainTextures;
    const float angleScale = (float)(M_PI / (numMountainTextures * numFacesPerTexture));
    int n = numFacesPerTexture / 2;
    float hightScale = mountainsMinWidth / 256.0f;
    for (j = 0; j < numMountainTextures; n += numFacesPerTexture, j++) {
      mountainsList[j].begin();
	glBegin(GL_TRIANGLE_STRIP);
	  for (i = 0; i <= numFacesPerTexture; i++) {
	    const float angle = angleScale * (float)(i + n);
	    float frac = (float)i / (float)numFacesPerTexture;
	    if (numMountainTextures != 1)
	      frac = (frac * (float)(mountainsMinWidth - 2) + 1.0f) /
						(float)mountainsMinWidth;
	    glNormal3f((float)(-M_SQRT1_2 * cosf(angle)),
			 (float)(-M_SQRT1_2 * sinf(angle)),
			  (float)M_SQRT1_2);
	    glTexCoord2f(frac, 0.02f);
	    glVertex3f(2.25f * worldSize * cosf(angle),
			 2.25f * worldSize * sinf(angle),
			 0.0f);
	    glTexCoord2f(frac, 0.99f);
	    glVertex3f(2.25f * worldSize * cosf(angle),
			 2.25f * worldSize * sinf(angle),
			 0.45f * worldSize*hightScale);
	  }
	glEnd();
	glBegin(GL_TRIANGLE_STRIP);
	  for (i = 0; i <= numFacesPerTexture; i++) {
	    const float angle = (float)(M_PI + angleScale * (double)(i + n));
	    float frac = (float)i / (float)numFacesPerTexture;
	    if (numMountainTextures != 1)
	      frac = (frac * (float)(mountainsMinWidth - 2) + 1.0f) /
						(float)mountainsMinWidth;
	    glNormal3f((float)(-M_SQRT1_2 * cosf(angle)),
			 (float)(-M_SQRT1_2 * sinf(angle)),
			  (float)M_SQRT1_2);
	    glTexCoord2f(frac, 0.02f);
	    glVertex3f(2.25f * worldSize * cosf(angle),
			 2.25f * worldSize * sinf(angle),
			 0.0f);
	    glTexCoord2f(frac, 0.99f);
	    glVertex3f(2.25f * worldSize * cosf(angle),
			 2.25f * worldSize * sinf(angle),
			 0.45f * worldSize*hightScale);
	  }
	glEnd();
      mountainsList[j].end();
    }
  }

  //
  // update objects in sky.  the appearance of these objects will
  // be wrong until setCelestial is called with the appropriate
  // arguments.
  //

  static const float up[3] = { 0.0f, 0.0f, 1.0f };
  setCelestial(renderer, up, up);
}

void			BackgroundRenderer::initDisplayLists(void* self)
{
  ((BackgroundRenderer*)self)->doInitDisplayLists();
}

const GLfloat*	BackgroundRenderer::getSunDirection() const
{
  if (areShadowsCast(sunDirection)) {
    return sunDirection;
  } else {
    return NULL;
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

