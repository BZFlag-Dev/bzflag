
#include "common.h"

// interface header
#include "OpenGLPassState.h"

// system headers
#include <assert.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>

// common headers
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "SceneRenderer.h"
#include "BzfDisplay.h"
#include "BzfWindow.h"
#include "TextureManager.h"
#include "OpenGLGState.h"
#include "bzfio.h"

// bzflag headers
#include "bzflag/guiplaying.h"
#include "bzflag/MainWindow.h"
#include "bzflag/RadarRenderer.h"
#include "bzflag/BackgroundRenderer.h"


#define TEXMGR (TextureManager::instance())


const GLbitfield AttribBits = GL_ALL_ATTRIB_BITS;


//============================================================================//
//============================================================================//

void (*OpenGLPassState::resetMatrixFunc)(void) = NULL;

OpenGLPassState::DrawMode
  OpenGLPassState::drawMode = OpenGLPassState::DRAW_NONE;

bool  OpenGLPassState::drawingEnabled = false;
float OpenGLPassState::screenWidth    = 0.36f;
float OpenGLPassState::screenDistance = 0.60f;

int OpenGLPassState::attribStackDepth    = 0;
int OpenGLPassState::minAttribStackDepth = 0;
int OpenGLPassState::maxAttribStackDepth = 16; // 16 are guaranteed

bool OpenGLPassState::creatingList = false;

unsigned int OpenGLPassState::stateLists[DRAW_MODE_COUNT] = {
  INVALID_GL_LIST_ID
};


//============================================================================//
//============================================================================//

const char* OpenGLPassState::GetDrawModeName(DrawMode mode)
{
  switch (mode) {
    case DRAW_GENESIS:      { return "DrawGenesis";     }
    case DRAW_WORLD_START:  { return "DrawWorldStart";  }
    case DRAW_WORLD:        { return "DrawWorld";       }
    case DRAW_WORLD_ALPHA:  { return "DrawWorldAlpha";  }
    case DRAW_WORLD_SHADOW: { return "DrawWorldShadow"; }
    case DRAW_SCREEN_START: { return "DrawScreenStart"; }
    case DRAW_SCREEN:       { return "DrawScreen";      }
    case DRAW_RADAR:        { return "DrawRadar";       }
    case DRAW_NONE:         { return "none";            }
    default:                { return "unknown";         }
  }
  return "unknown";
}


//============================================================================//
//============================================================================//
//
//  Attribute Stack management
//

bool OpenGLPassState::TryAttribStackChange(int change)
{
  const int result = attribStackDepth + change;
  if ((result < minAttribStackDepth) ||
      (result > maxAttribStackDepth)) {
    return false;
  }
  attribStackDepth = result;
  return true;
}


bool OpenGLPassState::TryAttribStackPush()
{
  return TryAttribStackChange(+1);
}


bool OpenGLPassState::TryAttribStackPop()
{
  return TryAttribStackChange(-1);
}


bool OpenGLPassState::PushAttrib(unsigned int bits)
{
  if (!TryAttribStackPush()) {
    return false;
  }
  glPushAttrib(bits);
  return true;
}


bool OpenGLPassState::PopAttrib()
{
  if (!TryAttribStackPop()) {
    return false;
  }
  glPopAttrib();
  return true;
}


bool OpenGLPassState::TryAttribStackRangeChange(int endChange,
                                                int minChange,
                                                int maxChange)
{
  const int minDepth = attribStackDepth + minChange;
  const int maxDepth = attribStackDepth + maxChange;
  if ((minDepth < minAttribStackDepth) ||
      (maxDepth > maxAttribStackDepth)) {
    return false;
  }
  return TryAttribStackChange(endChange);
}


//============================================================================//
//============================================================================//
//
//  Display List management
//

static bool oldDrawingEnabled = false;


bool OpenGLPassState::NewList()
{
  if (creatingList) {
    return false;
  }
  oldDrawingEnabled = drawingEnabled;
  drawingEnabled = true;
  creatingList = true;
  return true;
}


bool OpenGLPassState::EndList()
{
  if (!creatingList) {
    return false;
  }
  drawingEnabled = oldDrawingEnabled;
  creatingList = false;
  return true;
}


bool OpenGLPassState::CreatingList()
{
  return creatingList;
}


//============================================================================//
//============================================================================//

void OpenGLPassState::InitContext(void* /*data*/)
{
  for (int mode = 0; mode < (int)DRAW_MODE_COUNT; mode++) {
    stateLists[mode] = glGenLists(1);
    glNewList(stateLists[mode], GL_COMPILE);
    {
      ResetModeState((DrawMode)mode);
    }
    glEndList();
  }
}


void OpenGLPassState::FreeContext(void* /*data*/)
{
  for (int mode = 0; mode < (int)DRAW_MODE_COUNT; mode++) {
    glDeleteLists(stateLists[mode], 1);
    stateLists[mode] = INVALID_GL_LIST_ID;
  }
}


void OpenGLPassState::BZDBCallback(const std::string& /*name*/, void* /*data*/)
{
  FreeContext(NULL);
  InitContext(NULL);
}


//============================================================================//
//============================================================================//

void OpenGLPassState::Init()
{
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

  InitContext(NULL);
  OpenGLGState::registerContextInitializer(FreeContext, InitContext, NULL);

  BZDB.addCallback("shadowAlpha", BZDBCallback, NULL);
  BZDB.addCallback("shadowMode",  BZDBCallback, NULL);
  BZDB.addCallback("_fogMode",    BZDBCallback, NULL);
  BZDB.addCallback("_fogStart",   BZDBCallback, NULL);
  BZDB.addCallback("_fogEnd",     BZDBCallback, NULL);
  BZDB.addCallback("_fogDensity", BZDBCallback, NULL);
  BZDB.addCallback("_fogColor",   BZDBCallback, NULL);
}


void OpenGLPassState::Free()
{
  BZDB.removeCallback("shadowAlpha", BZDBCallback, NULL);
  BZDB.removeCallback("shadowMode",  BZDBCallback, NULL);
  BZDB.removeCallback("_fogMode",    BZDBCallback, NULL);
  BZDB.removeCallback("_fogStart",   BZDBCallback, NULL);
  BZDB.removeCallback("_fogEnd",     BZDBCallback, NULL);
  BZDB.removeCallback("_fogDensity", BZDBCallback, NULL);
  BZDB.removeCallback("_fogColor",   BZDBCallback, NULL);

  OpenGLGState::registerContextInitializer(FreeContext, InitContext, NULL);
  FreeContext(NULL);
}


void OpenGLPassState::ConfigScreen(float width, float dist)
{
  screenWidth = width;
  screenDistance = dist;
}


//============================================================================//
//============================================================================//

void OpenGLPassState::ResetState()
{
  ExecuteAttribStack(0);
  glPushAttrib(AttribBits);
  attribStackDepth = 1;
  if ((drawMode >= DRAW_GENESIS) &&
      (drawMode <  DRAW_MODE_COUNT)) {
    glCallList(stateLists[drawMode]);
  }
}


void OpenGLPassState::ResetMatrices()
{
  if (resetMatrixFunc != NULL) {
    resetMatrixFunc();
  }
}


//============================================================================//
//============================================================================//

void OpenGLPassState::ClearMatrixStack(int stackDepthEnum)
{
  GLint currDepth = 0;
  glGetIntegerv(stackDepthEnum, &currDepth);
  for (/*no-op*/; currDepth > 1; currDepth--) {
    glPopMatrix();
  }
}


void OpenGLPassState::ExecuteAttribStack(int targetDepth)
{
  GLint currDepth = 0;
  glGetIntegerv(GL_ATTRIB_STACK_DEPTH, &currDepth);
  for (/*no-op*/; currDepth > targetDepth; currDepth--) {
    glPopAttrib();
  }
}


static bool CheckMatrixDepths(const char* name = "unknown")
{
  GLint depth = 0;

  glGetIntegerv(GL_MODELVIEW_STACK_DEPTH, &depth);
  if (depth != 1) {
    logDebugMessage(1, "GL_MODELVIEW_STACK_DEPTH for %s at %i\n", name, depth);
    return false;
  }
  glGetIntegerv(GL_PROJECTION_STACK_DEPTH, &depth);
  if (depth != 1) {
    logDebugMessage(1, "GL_PROJECTION_STACK_DEPTH for %s at %i\n", name, depth);
    return false;
  }
  glGetIntegerv(GL_TEXTURE_STACK_DEPTH, &depth);
  if (depth != 1) {
    logDebugMessage(1, "GL_TEXTURE_STACK_DEPTH for %s at %i\n", name, depth);
    return false;
  }

  glGetIntegerv(GL_ATTRIB_STACK_DEPTH, &depth);
  if (depth != 0) {
    logDebugMessage(1, "GL_ATTRIB_STACK_DEPTH for %s at %i\n", name, depth);
    return false;
  }
  glGetIntegerv(GL_CLIENT_ATTRIB_STACK_DEPTH, &depth);
  if (depth != 0) {
    logDebugMessage(1, "GL_CLIENT_ATTRIB_STACK_DEPTH for %s at %i\n", name, depth);
    return false;
  }

  return true;
}


//============================================================================//
//============================================================================//

void OpenGLPassState::ResetModeState(DrawMode mode)
{
  // depth test
  switch (mode) {
    case DRAW_WORLD_START:
    case DRAW_WORLD:
    case DRAW_WORLD_ALPHA:
    case DRAW_WORLD_SHADOW: {
      glEnable(GL_DEPTH_TEST);
      break;
    }
    default: {
      glDisable(GL_DEPTH_TEST);
      break;
    }
  }

  // depth mask
  switch (mode) {
    case DRAW_WORLD_START:
    case DRAW_WORLD: {
      glDepthMask(GL_TRUE);
      break;
    }
    default: {
      glDepthMask(GL_FALSE);
      break;
    }
  }

  // depth function
  glDepthFunc(GL_LEQUAL);
  if (GLEW_NV_depth_clamp) {
    glDisable(GL_DEPTH_CLAMP_NV);
  }

  // color mask
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  // blending
  switch (mode) {
    case DRAW_WORLD_START:
    case DRAW_WORLD:
    case DRAW_WORLD_SHADOW: {
      glDisable(GL_BLEND);
      break;
    }
    default: {
      glEnable(GL_BLEND);
      break;
    }
  }
  if (glBlendEquation != NULL) {
    glBlendEquation(GL_FUNC_ADD);
  }
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // culling
  switch (mode) {
    case DRAW_WORLD_START:
    case DRAW_WORLD:
    case DRAW_WORLD_ALPHA: {
      glEnable(GL_CULL_FACE);
      break;
    }
    default: {
      glDisable(GL_CULL_FACE);
      break;
    }
  }
  glCullFace(GL_BACK);

  // lighting
  switch (mode) {
    case DRAW_WORLD_START:
    case DRAW_WORLD:
    case DRAW_WORLD_ALPHA: {
      glEnable(GL_LIGHTING);
      break;
    }
    default: {
      glDisable(GL_LIGHTING);
      break;
    }
  }
  glDisable(GL_COLOR_MATERIAL);

  // fog
  switch (mode) {
    case DRAW_WORLD_START:
    case DRAW_WORLD:
    case DRAW_WORLD_ALPHA:
    case DRAW_WORLD_SHADOW: {
      RENDERER.setupFog();
      break;
    }
    default: {
      glDisable(GL_FOG);
      break;
    }
  }

  // lighting config
  glShadeModel(GL_SMOOTH);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,      GL_FALSE);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,  GL_TRUE);
  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

  glEnable(GL_NORMALIZE);

  glDisable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.5f);

  glEnable(GL_SCISSOR_TEST);

  glDisable(GL_COLOR_LOGIC_OP);
  glLogicOp(GL_INVERT);

  // stencil
  if (mode != DRAW_WORLD_SHADOW) {
    glDisable(GL_STENCIL_TEST);
  }
  else {
    if (BZDBCache::shadowMode == SceneRenderer::StencilShadows) {
      glEnable(GL_STENCIL_TEST);
    } else {
      glDisable(GL_STENCIL_TEST);
    }
  }
  glStencilMask(~0);
  if (GLEW_EXT_stencil_two_side) {
    glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
  }

  // FIXME -- multi-texturing?
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);
  glDisable(GL_TEXTURE_GEN_R);
  glDisable(GL_TEXTURE_GEN_Q);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  // polygon mode and offset
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDisable(GL_POLYGON_OFFSET_FILL);
  glDisable(GL_POLYGON_OFFSET_LINE);
  glDisable(GL_POLYGON_OFFSET_POINT);

  // line stipple
  glDisable(GL_LINE_STIPPLE);

  // polygon stipple
  if (mode != DRAW_WORLD_SHADOW) {
    glDisable(GL_POLYGON_STIPPLE);
  }
  else {
    if (BZDBCache::shadowMode == SceneRenderer::StencilShadows) {
      glDisable(GL_POLYGON_STIPPLE);
    } else {
      glEnable(GL_POLYGON_STIPPLE);
    }
  }

  glLineWidth(1.0f);
  glPointSize(1.0f);

  // color
  if (mode != DRAW_WORLD_SHADOW) {
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  }
  else {
    if (BZDBCache::shadowMode == SceneRenderer::StencilShadows) {
      glColor4f(0.0f, 0.0f, 0.0f, BZDBCache::shadowAlpha);
    } else {
      glColor3f(0.0f, 0.0f, 0.0f);
    }
  }

  // material
  const fvec4 ambient (0.2f, 0.2f, 0.2f, 1.0f);
  const fvec4 diffuse (0.8f, 0.8f, 0.8f, 1.0f);
  const fvec4 black   (0.0f, 0.0f, 0.0f, 1.0f);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
  glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);

  if (GLEW_VERSION_2_0) {
    glDisable(GL_POINT_SPRITE);
    fvec3 atten(1.0f, 0.0f, 0.0f);
    glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, atten);
    glPointParameterf(GL_POINT_SIZE_MIN, 0.0f);
    glPointParameterf(GL_POINT_SIZE_MAX, 1.0e9f);
    glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, 1.0f);
  }

  if (glUseProgram) {
    glUseProgram(0);
  }
}


//============================================================================//
//============================================================================//
//
//  Common routines
//

inline void SetMainScissor()
{
  MainWindow* window = getMainWindow();
  if (window != NULL) {
    const int wh = window->getHeight();
    const int vh = window->getViewHeight();
    const int yOffset = (wh - vh);

    const int w = window->getWidth();
    const int h = window->getViewHeight();
    const int x = window->getOriginX();
    const int y = window->getOriginY() + yOffset;

    glScissor(x, y, w, h);
  }
}


inline void OpenGLPassState::EnableCommon(DrawMode mode)
{
  CheckMatrixDepths(GetDrawModeName(mode));

  assert(drawMode == DRAW_NONE);

  drawMode = mode;
  drawingEnabled = true;

  if (mode != DRAW_RADAR) {
    SetMainScissor();
  }

  assert(attribStackDepth == 0);
  glPushAttrib(AttribBits);
  attribStackDepth = 1;

  glCallList(stateLists[mode]);
}


inline void OpenGLPassState::DisableCommon(DrawMode mode)
{
  assert(drawMode == mode);
  mode = mode; // avoid warnings

  drawMode = DRAW_NONE;
  drawingEnabled = false;

  ExecuteAttribStack(0);
  attribStackDepth = 0;

  if (GLEW_VERSION_2_0) {
    glDisable(GL_POINT_SPRITE);
  }
  if (glUseProgram) {
    glUseProgram(0);
  }

  TEXMGR.clearLastBoundID(); // FIXME - not needed? glPopAttrib() restores it?
}


//============================================================================//
//
//  Genesis
//

void OpenGLPassState::EnableDrawGenesis()
{
  EnableCommon(DRAW_GENESIS);
  resetMatrixFunc = ResetIdentityMatrices;
  ResetWorldMatrices();
}


void OpenGLPassState::ResetDrawGenesis()
{
  ResetWorldMatrices();
  ResetState();
  SetMainScissor(); // FIXME
}


void OpenGLPassState::DisableDrawGenesis()
{
  ResetIdentityMatrices();
  DisableCommon(DRAW_GENESIS);
}


//============================================================================//
//
//  WorldStart
//

void OpenGLPassState::EnableDrawWorldStart()
{
  EnableCommon(DRAW_WORLD_START);
  resetMatrixFunc = ResetWorldMatrices;
  ResetWorldMatrices();
}


void OpenGLPassState::ResetDrawWorldStart()
{
  ResetWorldMatrices();
  ResetState();
  SetMainScissor();
}


void OpenGLPassState::DisableDrawWorldStart()
{
  ResetWorldMatrices();
  DisableCommon(DRAW_WORLD_START);
}


//============================================================================//
//
//  World
//

void OpenGLPassState::EnableDrawWorld()
{
  EnableCommon(DRAW_WORLD);
  resetMatrixFunc = ResetWorldMatrices;
}


void OpenGLPassState::ResetDrawWorld()
{
  ResetWorldMatrices();
  ResetState();
  SetMainScissor();
}


void OpenGLPassState::DisableDrawWorld()
{
  ResetWorldMatrices();
  DisableCommon(DRAW_WORLD);
}


//============================================================================//
//
//  WorldAlpha
//

void OpenGLPassState::EnableDrawWorldAlpha()
{
  EnableCommon(DRAW_WORLD_ALPHA);
  resetMatrixFunc = ResetWorldMatrices;
}


void OpenGLPassState::ResetDrawWorldAlpha()
{
  ResetWorldMatrices();
  ResetState();
  SetMainScissor();
}


void OpenGLPassState::DisableDrawWorldAlpha()
{
  ResetWorldMatrices();
  DisableCommon(DRAW_WORLD_ALPHA);
}


//============================================================================//
//
//  WorldShadow
//

void OpenGLPassState::EnableDrawWorldShadow()
{
  EnableCommon(DRAW_WORLD_SHADOW);
  resetMatrixFunc = ResetWorldShadowMatrices;
}


void OpenGLPassState::ResetDrawWorldShadow()
{
  ResetWorldShadowMatrices();
  ResetState();
  SetMainScissor();
}


void OpenGLPassState::DisableDrawWorldShadow()
{
  RevertShadowMatrices();
  DisableCommon(DRAW_WORLD_SHADOW);
}


//============================================================================//
//
//  ScreenStart -- same as Screen
//

void OpenGLPassState::EnableDrawScreenStart()
{
  EnableCommon(DRAW_SCREEN_START);
  resetMatrixFunc = ResetScreenMatrices;

  SetupScreenMatrices();
  SetupScreenLighting();
}


void OpenGLPassState::ResetDrawScreenStart()
{
  ResetScreenMatrices();
  ResetState();
  SetupScreenLighting();
  SetMainScissor();
}


void OpenGLPassState::DisableDrawScreenStart()
{
//FIXME  RevertScreenLighting();
  RevertScreenMatrices();
  DisableCommon(DRAW_SCREEN_START);
}


//============================================================================//
//
//  Screen
//

void OpenGLPassState::EnableDrawScreen()
{
  EnableCommon(DRAW_SCREEN);
  resetMatrixFunc = ResetScreenMatrices;

  SetupScreenMatrices();
  SetupScreenLighting();
}


void OpenGLPassState::ResetDrawScreen()
{
  ResetScreenMatrices();
  ResetState();
  SetupScreenLighting();
  SetMainScissor();
}


void OpenGLPassState::DisableDrawScreen()
{
//FIXME  RevertScreenLighting();
  RevertScreenMatrices();
  DisableCommon(DRAW_SCREEN);
}


//============================================================================//
//
//  Radar
//

void OpenGLPassState::EnableDrawRadar()
{
  EnableCommon(DRAW_RADAR);
  resetMatrixFunc = ResetRadarMatrices;
  glMatrixMode(GL_TEXTURE); {
    ClearMatrixStack(GL_TEXTURE_STACK_DEPTH);
    glLoadIdentity();
  }
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  RadarRenderer* radar = getRadarRenderer();
  if (radar) {
    radar->executeScissor();
    radar->executeTransform(true);
  }
}


void OpenGLPassState::ResetDrawRadar()
{
  ResetRadarMatrices();
  ResetState();
  RadarRenderer* radar = getRadarRenderer();
  if (radar) {
    radar->executeScissor();
  }
}


void OpenGLPassState::DisableDrawRadar()
{
  resetMatrixFunc = ResetScreenMatrices;
  ResetScreenMatrices();
  DisableCommon(DRAW_RADAR);
}


//============================================================================//
//============================================================================//

void OpenGLPassState::SetupScreenMatrices()
{
  BzfDisplay* dpy = getDisplay();
  MainWindow* wnd = getMainWindow();
  BzfWindow* bzWnd = wnd->getWindow();
  if ((dpy == NULL) || (wnd == NULL) || (bzWnd == NULL)) {
    return;
  }

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  const int dsx = dpy->getWidth();
  const int dsy = dpy->getHeight();
  const int wsx = wnd->getWidth();
  const int wsy = wnd->getHeight();
  const int wox = wnd->getOriginX();
  const int woy = wnd->getOriginY();

  int wpx, wpy;
  bzWnd->getPosition(wpx, wpy);
  wpy = (dsy - (wpy + wsy)); // flip the y axis

  const float dist   = screenDistance;   // eye-to-screen (meters)
  const float width  = screenWidth;      // screen width (meters)
  const float spsx   = float(dsx);       // screen pixel size x
  const float spsy   = float(dsy);       // screen pixel size y
  const float wpsx   = float(wsx);       // window pixel size x
  const float wpsy   = float(wsy);       // window pixel size y
  const float wppx   = float(wox + wpx); // window pixel pos x
  const float wppy   = float(woy + wpy); // window pixel pos y
  const float halfSX = 0.5f * spsx;      // half screen pixel size x
  const float halfSY = 0.5f * spsy;      // half screen pixel size y

  const float zplane = dist * (spsx / width);
  const float znear  = zplane * 0.5f;
  const float zfar   = zplane * 2.0f;
  const float factor = (znear / zplane);
  const float left   = (wppx - halfSX) * factor;
  const float bottom = (wppy - halfSY) * factor;
  const float right  = ((wppx + wpsx) - halfSX) * factor;
  const float top    = ((wppy + wpsy) - halfSY) * factor;
  glFrustum(left, right, bottom, top, znear, zfar);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  // translate so that (0,0,0) is on the zplane,
  // on the window's bottom left corner
  const float distAdj = (zplane / znear);
  glTranslatef(left * distAdj, bottom * distAdj, -zplane);
}


void OpenGLPassState::RevertScreenMatrices()
{
  glMatrixMode(GL_TEXTURE); {
    ClearMatrixStack(GL_TEXTURE_STACK_DEPTH);
    glLoadIdentity();
  }
  glMatrixMode(GL_PROJECTION); {
    ClearMatrixStack(GL_PROJECTION_STACK_DEPTH);
    glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);
  }
  glMatrixMode(GL_MODELVIEW); {
    ClearMatrixStack(GL_MODELVIEW_STACK_DEPTH);
    glLoadIdentity();
  }
}


void OpenGLPassState::SetupScreenLighting()
{
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, fvec4(0.2f, 0.2f, 0.2f, 1.0f));

  // back light
  const fvec4 backLightPos  = fvec4(1.0f, 2.0f, 2.0f, 0.0f);
  const fvec4 backLightAmbt = fvec4(0.0f, 0.0f, 0.0f, 1.0f);
  const fvec4 backLightDiff = fvec4(0.5f, 0.5f, 0.5f, 1.0f);
  const fvec4 backLightSpec = fvec4(1.0f, 1.0f, 1.0f, 1.0f);
  glLightfv(GL_LIGHT0, GL_POSITION, backLightPos);
  glLightfv(GL_LIGHT0, GL_AMBIENT,  backLightAmbt);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  backLightDiff);
  glLightfv(GL_LIGHT0, GL_SPECULAR, backLightSpec);
  glEnable(GL_LIGHT0);

  // sunlight
  const fvec3* sunDirPtr = RENDERER.getSunDirection();
  if (sunDirPtr == NULL) {
    return;
  }
  const fvec3& sunDir  = *sunDirPtr;
  const fvec4& diffuse = RENDERER.getSunColor();
  const fvec4& ambient = RENDERER.getAmbientColor();

  // need the camera transformation for world placement
  glPushMatrix();
  glLoadIdentity();
  RENDERER.getViewFrustum().executeView();
  glLightfv(GL_LIGHT1, GL_POSITION, fvec4(sunDir, 0.0f));
  glPopMatrix();

  const float sf = 1.0f; // sunFactor;

  const fvec4 sunLightAmbt = ambient * sf;
  const fvec4 sunLightDiff = diffuse * sf;
  const fvec4 sunLightSpec = ambient * sf;

  glLightfv(GL_LIGHT1, GL_AMBIENT,  sunLightAmbt);
  glLightfv(GL_LIGHT1, GL_DIFFUSE,  sunLightDiff);
  glLightfv(GL_LIGHT1, GL_SPECULAR, sunLightSpec);
  glEnable(GL_LIGHT1);
}


void OpenGLPassState::RevertScreenLighting()
{
  glDisable(GL_LIGHT1);
  glDisable(GL_LIGHT0);
}


//============================================================================//
//============================================================================//

void OpenGLPassState::ResetIdentityMatrices()
{
  glMatrixMode(GL_TEXTURE); {
    ClearMatrixStack(GL_TEXTURE_STACK_DEPTH);
    glLoadIdentity();
  }
  glMatrixMode(GL_PROJECTION); {
    ClearMatrixStack(GL_PROJECTION_STACK_DEPTH);
    glLoadIdentity();
  }
  glMatrixMode(GL_MODELVIEW); {
    ClearMatrixStack(GL_MODELVIEW_STACK_DEPTH);
    glLoadIdentity();
  }
}


void OpenGLPassState::ResetWorldMatrices()
{
  ViewFrustum& vf = RENDERER.getViewFrustum();

  glMatrixMode(GL_TEXTURE); {
    ClearMatrixStack(GL_TEXTURE_STACK_DEPTH);
    glLoadIdentity();
  }
  glMatrixMode(GL_PROJECTION); {
    ClearMatrixStack(GL_PROJECTION_STACK_DEPTH);
    glLoadIdentity();
    vf.executeProjection();
  }
  glMatrixMode(GL_MODELVIEW); {
    ClearMatrixStack(GL_MODELVIEW_STACK_DEPTH);
    glLoadIdentity();
    vf.executeView();
  }
}


void OpenGLPassState::ResetWorldShadowMatrices()
{
  ResetWorldMatrices();
  RENDERER.getBackground()->multShadowMatrix();
}


void OpenGLPassState::ResetScreenMatrices()
{
  ResetIdentityMatrices();
  SetupScreenMatrices();
}


void OpenGLPassState::ResetRadarMatrices()
{
  ResetIdentityMatrices();
  RadarRenderer* radar = getRadarRenderer();
  if (radar) {
    radar->executeTransform(true);
  }
}


void OpenGLPassState::RevertShadowMatrices()
{
  ViewFrustum& vf = RENDERER.getViewFrustum();

  glMatrixMode(GL_TEXTURE); {
    ClearMatrixStack(GL_TEXTURE_STACK_DEPTH);
    glLoadIdentity();
  }
  glMatrixMode(GL_PROJECTION); {
    ClearMatrixStack(GL_PROJECTION_STACK_DEPTH);
    glLoadIdentity();
    vf.executeDeepProjection();
  }
  glMatrixMode(GL_MODELVIEW); {
    ClearMatrixStack(GL_MODELVIEW_STACK_DEPTH);
    // the code in BackgroundRenderer use glLoadIdentity()
  }
}


//============================================================================//
//============================================================================//
