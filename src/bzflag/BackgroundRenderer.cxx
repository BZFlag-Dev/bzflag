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

#include "common.h"
#include "global.h"
#include "OpenGLMaterial.h"
#include "OpenGLTexture.h"
#include "BackgroundRenderer.h"
#include "stars.h"
#include "TextureManager.h"
#include "StateDatabase.h"
#include "SceneRenderer.h"
#include "BZDBCache.h"
#include "daylight.h"
#include "MainWindow.h"
#include "SceneNode.h"
#include "TimeKeeper.h"
#include "TextUtils.h"

//static     bool         useMoonTexture = false;

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
    // OpenGLTexture *groundTexture = NULL;
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
    int width  = 0;
    int height = 0;
    numMountainTextures = 0;
    bool done = false;
    while (!done) {
      char text[256];
      sprintf(text, "mountain%d", numMountainTextures + 1);
      mountainTexture = tm.getTextureID(text, false);
      if (mountainTexture >= 0) {
        const ImageInfo &info = tm.getInfo(mountainTexture);
	height = info.y;
	width += info.x;
	numMountainTextures++;
      } else {
	done = true;
      }
    }

    if (numMountainTextures > 0) {
      mountainsAvailable = true;

      // prepare common gstate
      gstate.reset();
      gstate.setShading();
      gstate.setBlending();
      gstate.setMaterial(defaultMaterial);
      gstate.setAlphaFunc();

      if (numMountainTextures > 1)
	width -= 2 * numMountainTextures;
      // find power of two at least as large as height
      int scaledHeight = 1;
      while (scaledHeight < height) scaledHeight <<= 1;

      // choose minimum width
      int minWidth = scaledHeight;
      if (minWidth > scaledHeight) minWidth = scaledHeight;
      mountainsMinWidth = minWidth;

      // prepare each texture
      mountainsGState = new OpenGLGState[numMountainTextures];
      for (i = 0; i < numMountainTextures; i++) {
	char text[256];
	sprintf(text,"mountain%d",i+1);
	gstate.setTexture(tm.getTextureID(text));
	mountainsGState[i] = gstate.getState();
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
  if (BZDB.isTrue("texture")) {
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

void                BackgroundRenderer::resize() {
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
    glRotatef(atan2f(sunDirection[1], sunDirection[0]) * 180.0f / M_PI,
							0.0f, 0.0f, 1.0f);
    glRotatef(asinf(sunDirection[2]) * 180.0f / M_PI, 0.0f, -1.0f, 0.0f);
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
				atanf(60.0f * M_PI / 180.0f) / 60.0f;
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
    glRotatef(atan2f(moonDirection[1], moonDirection[0]) * 180.0f / M_PI,
							0.0f, 0.0f, 1.0f);
    glRotatef(asinf(moonDirection[2]) * 180.0f / M_PI, 0.0f, -1.0f, 0.0f);
    glRotatef(limbAngle * 180.0f / M_PI, 1.0f, 0.0f, 0.0f);
    glBegin(GL_TRIANGLE_STRIP);
    // glTexCoord2f(0,-1);
    glVertex3f(2.0f * worldSize, 0.0f, -moonRadius);
      for (int i = 0; i < moonSegements-1; i++) {
	const float angle = 0.5f * M_PI * float(i-(moonSegements/2)-1) / (moonSegements/2.0f);
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

  // rain stuff
	weather.set();
}

void			BackgroundRenderer::addCloudDrift(GLfloat uDrift,
							GLfloat vDrift)
{
  cloudDriftU += 0.01f * uDrift / cloudRepeats;
  cloudDriftV += 0.01f * vDrift / cloudRepeats;
  if (cloudDriftU > 1.0f) cloudDriftU -= 1.0f;
  else if (cloudDriftU < 0.0f) cloudDriftU += 1.0f;
  if (cloudDriftV > 1.0f) cloudDriftV -= 1.0f;
  else if (cloudDriftV < 0.0f) cloudDriftV += 1.0f;
}

void			BackgroundRenderer::renderSky(
				SceneRenderer& renderer, bool fullWindow)
{
  if (renderer.useQuality() > 0) {
    drawSky(renderer);
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

void			BackgroundRenderer::renderGround(
				SceneRenderer& renderer, bool fullWindow)
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

void BackgroundRenderer::renderGroundEffects(SceneRenderer& renderer)
{
  // zbuffer should be disabled.  either everything is coplanar with
  // the ground or is drawn back to front and is occluded by everything
  // drawn after it.  also use projection with very far clipping plane.

  if (renderer.useQuality() < 3) {
    drawGroundGrid(renderer);
  }

  if (!blank) {
    if (doShadows && shadowsVisible) {
      drawGroundShadows(renderer);
    }

    // draw light receivers on ground (little meshes under light sources so
    // the ground gets illuminated).  this is necessary because lighting is
    // performed only at a vertex, and the ground's vertices are a few
    // kilometers away.
    if (BZDBCache::blend && BZDBCache::lighting) {
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
  const GLfloat skySize = 1.3f * BZDBCache::worldSize;
  for (int i = 0; i < 4; i++) {
    skyPyramid[i][0] = skySize * squareShape[i][0];
    skyPyramid[i][1] = skySize * squareShape[i][1];
    skyPyramid[i][2] = 0.0f;
  }
  skyPyramid[4][0] = 0.0f;
  skyPyramid[4][1] = 0.0f;
  skyPyramid[4][2] = skySize;
}


void			BackgroundRenderer::drawSky(SceneRenderer& renderer)
{
  // rotate sky so that horizon-point-toward-sun-color is actually
  // toward the sun
  glPushMatrix();
  glRotatef((atan2f(sunDirection[1], sunDirection[0]) * 180.0f + 135.0f) / M_PI,
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

  glPopMatrix();
}

void			BackgroundRenderer::drawGround()
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

  simpleGroundList[styleIndex].execute();
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

void			BackgroundRenderer::drawGroundReceivers(
						SceneRenderer& renderer)
{
  static const int receiverRings = 4;
  static const int receiverSlices = 8;
  static const float receiverRingSize = 1.2f;	// meters
  static float angle[receiverSlices + 1][2];
  static bool init = false;

  if (!init) {
    init = true;
    const float receiverSliceAngle = 2.0f * M_PI / float(receiverSlices);
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

void			BackgroundRenderer::drawMountains(void)
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

  // need some workarounds on RIVA 128
  bool isRiva128 = (strncmp((const char*)glGetString(GL_RENDERER),
						"RIVA 128", 8) == 0);

  //
  // sky stuff
  //

  // sun first.  sun is a disk that should be about a half a degree wide
  // with a normal (60 degree) perspective.
  const float worldSize = BZDBCache::worldSize;
  const float sunRadius = 2.0f * worldSize * atanf(60.0f*M_PI/180.0f) / 60.0f;
  sunList.begin();
    glBegin(GL_TRIANGLE_FAN);
      glVertex3f(2.0f * worldSize, 0.0f, 0.0f);
      for (i = 0; i < 20; i++) {
	const float angle = 2.0f * M_PI * float(i) / 19.0f;
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
    const float angleScale = M_PI /
			(float)(numMountainTextures * numFacesPerTexture);
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
	    glNormal3f(-M_SQRT1_2 * cosf(angle),
			 -M_SQRT1_2 * sinf(angle),
			  M_SQRT1_2);
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
	    const float angle = M_PI + angleScale * (float)(i + n);
	    float frac = (float)i / (float)numFacesPerTexture;
	    if (numMountainTextures != 1)
	      frac = (frac * (float)(mountainsMinWidth - 2) + 1.0f) /
						(float)mountainsMinWidth;
	    glNormal3f(-M_SQRT1_2 * cosf(angle),
			 -M_SQRT1_2 * sinf(angle),
			  M_SQRT1_2);
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


WeatherRenderer::WeatherRenderer()
{
/*	OpenGLGState				rainGState;
	OpenGLGState				texturedRainState;
	OpenGLGState				puddleState;
	std::string					rainSkin;
	std::vector<std::string>	rainTextures;
	std::vector<rain>			raindrops;
	std::vector<puddle>			puddles;*/

	rainColor[0][0] = 0.75f;   rainColor[0][1] = 0.75f;   rainColor[0][2] = 0.75f;   rainColor[0][3] = 0.75f; 
	rainColor[1][0] = 0.0f;   rainColor[1][1] = 0.0f;   rainColor[1][2] = 0.0f;   rainColor[1][3] = 0.0f; 

	rainSize[0] = rainSize[1] = 1.0f;

	rainDensity  = 1000;
	rainSpeed = -100;
	rainSpeedMod = 10;
	rainSpread = 500;

	doPuddles = true;
	doLineRain = true;

	rainStartZ = -1;
	rainEndZ = 0;

	lastRainTime = 0;
	
	maxPuddleTime = 5;
	puddleSpeed = 1.0f;

	puddleColor[0] = puddleColor[1] = puddleColor[2] = puddleColor[3] = 1.0f;
}

WeatherRenderer::~WeatherRenderer()
{
	// kill the lists here
}

void WeatherRenderer::init ( void )
{
	OpenGLGStateBuilder gstate;

	static const GLfloat	white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	OpenGLMaterial rainMaterial(white, white, 0.0f);

	TextureManager &tm = TextureManager::instance();
	
	gstate.reset();
	gstate.setShading();
	gstate.setBlending((GLenum)GL_SRC_ALPHA, (GLenum)GL_ONE_MINUS_SRC_ALPHA);
	gstate.setMaterial(rainMaterial);
	gstate.setAlphaFunc();
	rainGState = gstate.getState();

	// just set up some default textures
	gstate.setTexture(tm.getTextureID("snowflake"));
	texturedRainState = gstate.getState();

	gstate.setTexture(tm.getTextureID("puddle"));
	puddleState = gstate.getState();

	buildPuddleList();
}

void WeatherRenderer::set ( void )
{
	//TextureManager &tm = TextureManager::instance();

	// check the bzdb and see if we need to change any rain stuff
	rainDensity = 0;

	if (BZDB.isSet("rainType") || BZDB.isSet("rainDensity"))
	{
		// default rain desnity
		rainDensity = 1000;

		// some defaults
		doLineRain = false;
		rainSize[0] = 1.0f; rainSize[1] = 1.0f;
		rainSpeed = -100.0f;
		rainSpeedMod = 50.0f;
		doPuddles = true;
		rainColor[0][0] = 0.75f;   rainColor[0][1] = 0.75f;   rainColor[0][2] = 0.75f;   rainColor[0][3] = 0.75f; 
		rainColor[1][0] = 0.0f;   rainColor[1][1] = 0.0f;   rainColor[1][2] = 0.0f;   rainColor[1][3] = 0.0f; 
		rainSize[0] = rainSize[1] = 1.0f;
		maxPuddleTime = 1.5F;
		puddleSpeed = 1.0f;
		puddleColor[0] = puddleColor[1] = puddleColor[2] = puddleColor[3] = 1.0f;

		rainSpread  = 500.0f;
		if (BZDB.isSet("rainSpread"))
			rainSpread = BZDB.eval("rainSpread");

		TextureManager &tm = TextureManager::instance();

		static const GLfloat	white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		OpenGLMaterial rainMaterial(white, white, 0.0f);

		OpenGLGStateBuilder gstate;
		gstate.reset();
		gstate.setShading();
		gstate.setBlending((GLenum)GL_SRC_ALPHA, (GLenum)GL_ONE_MINUS_SRC_ALPHA);
		gstate.setMaterial(rainMaterial);
		gstate.setAlphaFunc();

		OpenGLGStateBuilder puddleGStateBuilder(puddleState);

		puddleGStateBuilder.setTexture(tm.getTextureID("puddle"));

		if (BZDB.isSet("rainType"))
		{
			std::string rainType = TextUtils::tolower(BZDB.get("rainType"));

			if (rainType == "snow")
			{
				doBillBoards = false;
				gstate.setTexture(tm.getTextureID("snowflake"));
				rainSpeed = -20.0f;
				rainSpeedMod = 5.0f;
				doPuddles = false;
			}
			else if (rainType == "fatrain")
			{
				doBillBoards = false;
				rainSize[0] = 0.5f; rainSize[1] = 0.75f;
				gstate.setTexture(tm.getTextureID("raindrop"));
				rainSpeed = -50.0f;
				rainSpeedMod = 25.0f;
			}
			else if (rainType == "frog")
			{
				gstate.setTexture(tm.getTextureID("frog"));
				rainSpeed = -100.0f;
				rainSpeedMod = 5.0f;
				doPuddles = true;
				doBillBoards = true;
				// ewww
				puddleSpeed = 3.0f;
				puddleColor[0] = 0.75f;
				puddleColor[1] = puddleColor[2] = 0.0f;
				rainSize[0] = 2.0; rainSize[1] = 2.0f;
			}
			else if (rainType == "particle")
			{
				rainDensity = 500;
				gstate.setTexture(tm.getTextureID("red_super_bolt"));
				rainSpeed = -20.0f;
				rainSpeedMod = 5.0f;
				doPuddles = true;
				doBillBoards = true;
				puddleSpeed = 10.0f;
				// ewww
				puddleColor[0] = 1.0f;
				puddleColor[1] = puddleColor[2] = 0.0f;
				rainSize[0] = 1.0; rainSize[1] = 1.0f;
			}
			else if (rainType == "bubble")
			{
				gstate.setTexture(tm.getTextureID("bubble"));
				rainSpeed = 20.0f;
				rainSpeedMod = 1.0f;
				doBillBoards = true;
				doPuddles = false;
			}
			else if (rainType == "rain")
			{
				doLineRain = true;
				rainSize[0] = 0; rainSize[1] = 0.75f;
			}
		}

		if (BZDB.isSet("rainPuddleTexture"))
			puddleGStateBuilder.setTexture(tm.getTextureID(BZDB.get("rainPuddleTexture").c_str()));

		OpenGLMaterial puddleMaterial(puddleColor, puddleColor, 0.0f);
		puddleGStateBuilder.setMaterial(puddleMaterial);
		puddleState = puddleGStateBuilder.getState();

		// see if the texture is specificly overiden
		if (BZDB.isSet("rainTexture"))
			gstate.setTexture(tm.getTextureID(BZDB.get("rainTexture").c_str()));

		texturedRainState = gstate.getState();

		// if there is a specific overides for stuff
		if (BZDB.isSet("rainDensity"))
			rainDensity = (int)BZDB.eval("rainDensity");

		if (BZDB.isSet("rainSpeed"))
			rainSpeed = BZDB.eval("rainSpeed");

		if (BZDB.isSet("rainSpeedMod"))
			rainSpeedMod = BZDB.eval("rainSpeedMod");

		if (BZDB.isSet("rainSize"))
			BZDB.evalPair("rainSize",rainSize);

		if (BZDB.isSet("rainStartZ"))
			rainStartZ = BZDB.eval("rainStartZ");

		if (BZDB.isSet("rainEndZ"))
			rainEndZ = BZDB.eval("rainEndZ");

		if (BZDB.isSet("rainBaseColor"))
			BZDB.evalTriplet("rainBaseColor",rainColor[0]);

		if (BZDB.isSet("rainTopColor"))
			BZDB.evalTriplet("rainTopColor",rainColor[1]);

		if (BZDB.isSet("rainPuddleColor"))
			BZDB.evalTriplet("rainPuddleColor",puddleColor);

		if (BZDB.isSet("rainPuddleSpeed"))
			puddleSpeed = BZDB.eval("rainPuddleSpeed");

		if (BZDB.isSet("rainMaxPuddleTime"))
			maxPuddleTime = BZDB.eval("rainMaxPuddleTime");

		// make sure we know where to start and stop the rain
		// we want to compute the heights for us
		if (rainStartZ == -1 && rainEndZ == 0)
		{
			// check the dir
			if ( rainSpeed < 0)	// rain going down
			{	
				rainStartZ = 120.0f * BZDBCache::tankHeight;	// same as the clouds
				rainEndZ = 0;
			}
			else				// rain going up ( tiny bubbles
			{
				if ( rainSpeed == 0)
					rainSpeed = 0.1f;

				rainEndZ = 120.0f * BZDBCache::tankHeight;	// same as the clouds
				rainStartZ = 0;
			}
		}
		else // the specified rain start and end values, make sure they make sense with the directon
		{
			if (rainSpeed < 0)	// rain going down
			{
				if ( rainEndZ > rainStartZ)
				{
					float temp = rainStartZ;
					rainStartZ = rainEndZ;
					rainEndZ = temp;
				}
			}
			else // rain going up
			{
				if ( rainEndZ < rainStartZ)
				{
					float temp = rainStartZ;
					rainStartZ = rainEndZ;
					rainEndZ = temp;
				}
			}
		}

		float rainHeightDelta = rainEndZ-rainStartZ;

		if (raindrops.size() == 0)
		{
			for ( int drops = 0; drops< rainDensity; drops++)
			{
				rain drop;
				drop.speed = rainSpeed + (((float)bzfrand()*2.0f -1.0f)*rainSpeedMod);
				drop.pos[0] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);
				drop.pos[1] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);
				drop.pos[2] = rainStartZ+ (((float)bzfrand())*rainHeightDelta);
				raindrops.push_back(drop);
			}
			lastRainTime = TimeKeeper::getCurrent().getSeconds();
		}
	}

	// recompute the drops based on the posible new size
	buildDropList();
}

void WeatherRenderer::update ( void )
{
	// update the time
	float frameTime = TimeKeeper::getCurrent().getSeconds()-lastRainTime;
	lastRainTime = TimeKeeper::getCurrent().getSeconds();
	
	if (frameTime > 1.0f)
		frameTime = 1.0f;

	// update all the drops in the world
	std::vector<rain>::iterator itr = raindrops.begin();
	while (itr != raindrops.end())
	{
		if (updateDrop(itr,frameTime))
			itr++;
	}

	// update all the puddles
	std::vector<puddle>::iterator puddleItr = puddles.begin();

	while(puddleItr != puddles.end())
	{
		if ( updatePuddle(puddleItr,frameTime))
			puddleItr++;
	}
}

void WeatherRenderer::draw ( const SceneRenderer& sr )
{
	glDepthMask(0);
	if (doLineRain)	// we are doing line rain
	{
		rainGState.setState();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();	
		glBegin(GL_LINES);

		std::vector<rain>::iterator itr = raindrops.begin();
		while (itr != raindrops.end())
		{
			float alphaMod = 0;

			if ( itr->pos[2] < 5.0f)
				alphaMod = 1.0f - (5.0f/itr->pos[2]);

			float alphaVal = rainColor[0][3]-alphaMod;
			if (alphaVal < 0)
				alphaVal = 0;

			glColor4f(rainColor[0][0],rainColor[0][1],rainColor[0][2],alphaVal);
			glVertex3fv(itr->pos);

			alphaVal = rainColor[1][3]-alphaMod;
			if (alphaVal < 0)
				alphaVal = 0;

			glColor4f(rainColor[1][0],rainColor[1][1],rainColor[1][2],alphaVal);
			glVertex3f(itr->pos[0],itr->pos[1],itr->pos[2]+ (rainSize[1] - (itr->speed * 0.15f)));
			itr++;
		}
		glEnd();
	}
	else // 3d rain
	{
		texturedRainState.setState();
		glDisable(GL_CULL_FACE);
		glMatrixMode(GL_MODELVIEW);

		std::vector<rain>::iterator itr = raindrops.begin();
		while (itr != raindrops.end())
		{
			float alphaMod = 0;

			if ( itr->pos[2] < 2.0f)
				alphaMod = (2.0f - itr->pos[2])*0.5f;

			glColor4f(1,1,1,1.0f - alphaMod);
			glPushMatrix();
			glTranslatef(itr->pos[0],itr->pos[1],itr->pos[2]);
			if (doBillBoards)
				sr.getViewFrustum().executeBillboard();

			glRotatef(lastRainTime*10.0f * rainSpeed,0,0,1);
			
			dropList.execute();

			glPopMatrix();
			itr++;
		}
		glEnable(GL_CULL_FACE);
	}
	if (doPuddles)
	{
		std::vector<puddle>::iterator puddleItr = puddles.begin();

		puddleState.setState();
		glDisable(GL_CULL_FACE);
		glMatrixMode(GL_MODELVIEW);
		glColor4f(1,1,1,1.0f);

		while(puddleItr != puddles.end())
		{
			glPushMatrix();
			glTranslatef(puddleItr->pos[0],puddleItr->pos[1],puddleItr->pos[2]);

			float scale = puddleItr->time * rainSpeed*0.035f*puddleSpeed;
			float lifeTime = puddleItr->time/maxPuddleTime;

			glColor4f(1,1,1,1.0f - lifeTime);

			glScalef(scale,scale,scale);
			puddleList.execute();

			glPopMatrix();

			puddleItr++;
		}
	}
	glEnable(GL_CULL_FACE);
	glColor4f(1,1,1,1);
	glPopMatrix();
	glDepthMask(1);
}

void WeatherRenderer::rebuildContext ( void )
{
	buildPuddleList();
	buildDropList();
}

void WeatherRenderer::buildDropList ( void )
{
	dropList.begin();
	glPushMatrix();

	if (doBillBoards)
	{
		glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex3f(-rainSize[0],-rainSize[1],0);

		glTexCoord2f(1,0);
		glVertex3f(rainSize[0],-rainSize[1],0);

		glTexCoord2f(1,1);
		glVertex3f(rainSize[0],rainSize[1],0);

		glTexCoord2f(0,1);
		glVertex3f(-rainSize[0],rainSize[1],0);
		glEnd();
	}
	else
	{
		glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex3f(-rainSize[0],0,-rainSize[1]);

		glTexCoord2f(1,0);
		glVertex3f(rainSize[0],0,-rainSize[1]);

		glTexCoord2f(1,1);
		glVertex3f(rainSize[0],0,rainSize[1]);

		glTexCoord2f(0,1);
		glVertex3f(-rainSize[0],0,rainSize[1]);
		glEnd();

		glRotatef(120,0,0,1);

		glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex3f(-rainSize[0],0,-rainSize[1]);

		glTexCoord2f(1,0);
		glVertex3f(rainSize[0],0,-rainSize[1]);

		glTexCoord2f(1,1);
		glVertex3f(rainSize[0],0,rainSize[1]);

		glTexCoord2f(0,1);
		glVertex3f(-rainSize[0],0,rainSize[1]);
		glEnd();

		glRotatef(120,0,0,1);

		glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex3f(-rainSize[0],0,-rainSize[1]);

		glTexCoord2f(1,0);
		glVertex3f(rainSize[0],0,-rainSize[1]);

		glTexCoord2f(1,1);
		glVertex3f(rainSize[0],0,rainSize[1]);

		glTexCoord2f(0,1);
		glVertex3f(-rainSize[0],0,rainSize[1]);
		glEnd();
	}
	glPopMatrix();
	dropList.end();
}

void WeatherRenderer::buildPuddleList ( void )
{
	float scale = 1;
	puddleList.begin();

		glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex3f(-scale,-scale,0);

		glTexCoord2f(1,0);
		glVertex3f(scale,-scale,0);

		glTexCoord2f(1,1);
		glVertex3f(scale,scale,0);

		glTexCoord2f(0,1);
		glVertex3f(-scale,scale,0);
		glEnd();

	puddleList.end();
}

bool WeatherRenderer::updateDrop ( std::vector<rain>::iterator &drop, float frameTime )
{
	drop->pos[2] += drop->speed * frameTime;
	
	bool killDrop = false;

	if ( drop->speed < 0)
		killDrop = drop->pos[2] < rainEndZ;
	else
		killDrop = drop->pos[2] > rainEndZ;

	if ( killDrop )
	{
		if (doPuddles)
		{	
			puddle	thePuddle;
			thePuddle.pos[0] = drop->pos[0];thePuddle.pos[1] = drop->pos[1];
			thePuddle.pos[2] = rainEndZ;;
			thePuddle.time = 0.001f;
			puddles.push_back(thePuddle);
		}

		if ( (int)(raindrops.size()) <= rainDensity)
		{
			// reset this drop
			drop->pos[2] = rainStartZ;
			drop->speed = rainSpeed + ((float)(bzfrand()*2.0f -1.0f)*rainSpeedMod);
			drop->pos[0] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);
			drop->pos[1] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);

			// we need more rain!!!
			if ((int)(raindrops.size()) < rainDensity)
			{
				rain newDrop;
				newDrop.pos[2] = rainStartZ;
				newDrop.speed = rainSpeed + ((float)(bzfrand()*2.0f -1.0f)*rainSpeedMod);
				newDrop.pos[0] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);
				newDrop.pos[1] = (((float)bzfrand()*2.0f -1.0f)*rainSpread);
				raindrops.push_back(newDrop);
			}
			return true;
		}
		else	// we need less rain, so don't do this one
		{
			drop = raindrops.erase(drop);
			return false;
		}
	}
	return true;
}

bool WeatherRenderer::updatePuddle ( std::vector<puddle>::iterator &splash, float frameTime )
{
	if ( splash->time > maxPuddleTime )
	{
		splash = puddles.erase(splash);
		return false;
	}
	splash->time += frameTime;
	return true;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

