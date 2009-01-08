//
// TODO:
// - go back to counting matrix push/pops (just for modelview?)
//   would have to make sure that display lists are also handled
//   (GL_MODELVIEW_STACK_DEPTH could help current situation, but
//    requires the ARB_imaging extension)
// - use materials instead of raw calls (again, handle dlists)
//

#include "common.h"

#include "assert.h"

// implementation header
#include "OpenGLPassState.h"

// system headers
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
using std::max;
using std::string;
using std::vector;
using std::set;

// common headers
#include "StateDatabase.h"
#include "SceneRenderer.h"
#include "BzfDisplay.h"
#include "BzfWindow.h"
#include "TextureManager.h"

// bzflag headers
#include "../bzflag/playing.h"
#include "../bzflag/MainWindow.h"


#define LuaLog printf // FIXME


/******************************************************************************/
/******************************************************************************/

void (*OpenGLPassState::resetMatrixFunc)(void) = NULL;

unsigned int OpenGLPassState::resetStateList = 0;
set<unsigned int> OpenGLPassState::occlusionQueries;

OpenGLPassState::DrawMode OpenGLPassState::drawMode = OpenGLPassState::DRAW_NONE;
OpenGLPassState::DrawMode OpenGLPassState::prevDrawMode = OpenGLPassState::DRAW_NONE;

bool  OpenGLPassState::drawingEnabled = false;
float OpenGLPassState::screenWidth = 0.36f;
float OpenGLPassState::screenDistance = 0.60f;

static bool haveGL20 = false;


/******************************************************************************/
/******************************************************************************/

void OpenGLPassState::Init()
{
  resetStateList = glGenLists(1);
  glNewList(resetStateList, GL_COMPILE); {
    ResetGLState();
  }
  glEndList();

  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

  haveGL20 = !!GLEW_VERSION_2_0;
}


void OpenGLPassState::Free()
{
  glDeleteLists(resetStateList, 1);

  if (haveGL20) {
    set<unsigned int>::const_iterator it;
    for (it = occlusionQueries.begin(); it != occlusionQueries.end(); ++it) {
      glDeleteQueries(1, &(*it));
    }
  }
}


void OpenGLPassState::ConfigScreen(float width, float dist)
{
  screenWidth = width;
  screenDistance = dist;
}


/******************************************************************************/
/******************************************************************************/

void OpenGLPassState::ResetState()
{
  glCallList(resetStateList);
}


void OpenGLPassState::ResetMatrices()
{
  resetMatrixFunc();
}


/******************************************************************************/
/******************************************************************************/

void OpenGLPassState::ClearMatrixStack(int stackDepthEnum)
{
  GLint depth = 0;
  glGetIntegerv(stackDepthEnum, &depth);
  for (int i = 0; i < depth - 1; i++) {
    glPopMatrix();
  }
}


static bool CheckMatrixDepths(const char* name = "unknown")
{
  GLint depth = 0;

  glGetIntegerv(GL_MODELVIEW_STACK_DEPTH, &depth);
  if (depth != 1) {
    LuaLog("GL_MODELVIEW_STACK_DEPTH for %s at %i\n", name, depth);
    return false;
  }
  glGetIntegerv(GL_PROJECTION_STACK_DEPTH, &depth);
  if (depth != 1) {
    LuaLog("GL_PROJECTION_STACK_DEPTH for %s at %i\n", name, depth);
    return false;
  }
  glGetIntegerv(GL_TEXTURE_STACK_DEPTH, &depth);
  if (depth != 1) {
    LuaLog("GL_TEXTURE_STACK_DEPTH for %s at %i\n", name, depth);
    return false;
  }

  glGetIntegerv(GL_ATTRIB_STACK_DEPTH, &depth);
  if (depth != 0) {
    LuaLog("GL_ATTRIB_STACK_DEPTH for %s at %i\n", name, depth);
    return false;
  }
  glGetIntegerv(GL_CLIENT_ATTRIB_STACK_DEPTH, &depth);
  if (depth != 0) {
    LuaLog("GL_CLIENT_ATTRIB_STACK_DEPTH for %s at %i\n", name, depth);
    return false;
  }

  return true;
}


void OpenGLPassState::ResetGLState()
{
  glDisable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glDepthMask(GL_FALSE);
  if (GLEW_NV_depth_clamp) {
    glDisable(GL_DEPTH_CLAMP_NV);
  }
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  glEnable(GL_BLEND);
  if (glBlendEquation != NULL) {
    glBlendEquation(GL_FUNC_ADD);
  }
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glDisable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.5f);

  glDisable(GL_LIGHTING);

  glShadeModel(GL_SMOOTH);

  glDisable(GL_COLOR_LOGIC_OP);
  glLogicOp(GL_INVERT);

  // FIXME glViewport(gl); depends on the mode

  // FIXME -- depends on the mode       glDisable(GL_FOG);

  glDisable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glDisable(GL_SCISSOR_TEST);

  glDisable(GL_STENCIL_TEST);
  glStencilMask(~0);
  if (GLEW_EXT_stencil_two_side) {
    glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
  }

  // FIXME -- multitexturing
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);
  glDisable(GL_TEXTURE_GEN_R);
  glDisable(GL_TEXTURE_GEN_Q);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDisable(GL_POLYGON_OFFSET_FILL);
  glDisable(GL_POLYGON_OFFSET_LINE);
  glDisable(GL_POLYGON_OFFSET_POINT);

  glDisable(GL_LINE_STIPPLE);

  glDisable(GL_CLIP_PLANE4);
  glDisable(GL_CLIP_PLANE5);

  glLineWidth(1.0f);
  glPointSize(1.0f);

  if (haveGL20) {
    glDisable(GL_POINT_SPRITE);
  }
  if (haveGL20) {
    GLfloat atten[3] = { 1.0f, 0.0f, 0.0f };
    glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, atten);
    glPointParameterf(GL_POINT_SIZE_MIN, 0.0f);
    glPointParameterf(GL_POINT_SIZE_MAX, 1.0e9f); // FIXME?
    glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, 1.0f);
  }

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  const float ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
  const float diffuse[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
  const float black[4]   = { 0.0f, 0.0f, 0.0f, 1.0f };
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);

  if (glUseProgram) {
    glUseProgram(0);
  }
}


/******************************************************************************/
/******************************************************************************/
//
//  Common routines
//

const GLbitfield AttribBits =
  GL_COLOR_BUFFER_BIT |
  GL_DEPTH_BUFFER_BIT |
  GL_ENABLE_BIT       |
  GL_LIGHTING_BIT     |
  GL_LINE_BIT         |
  GL_POINT_BIT        |
  GL_POLYGON_BIT      |
  GL_VIEWPORT_BIT;


inline void OpenGLPassState::EnableCommon(DrawMode mode)
{
  CheckMatrixDepths();
  assert(drawMode == DRAW_NONE);
  drawMode = mode;
  drawingEnabled = true;
  glPushAttrib(AttribBits);
  glCallList(resetStateList);
  // FIXME  --  not needed by shadow or radar   (use a WorldCommon ? )
  glEnable(GL_NORMALIZE);
  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
}


inline void OpenGLPassState::DisableCommon(DrawMode mode)
{
  mode = mode;
  assert(drawMode == mode);
  // FIXME  --  not needed by shadow or radar
  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR); // FIXME for bz???
  drawMode = DRAW_NONE;
  drawingEnabled = false;
  glPopAttrib();
  if (glUseProgram) {
    glUseProgram(0);
  }
}


/******************************************************************************/
//
//  Genesis
//

void OpenGLPassState::EnableDrawGenesis()
{
  EnableCommon(DRAW_WORLD_START);
  resetMatrixFunc = ResetWorldMatrices;
  ResetWorldMatrices();
  SetupWorldLighting();
}


void OpenGLPassState::DisableDrawGenesis()
{
  ResetWorldMatrices();
  RevertWorldLighting();
  DisableCommon(DRAW_WORLD_START);
}


void OpenGLPassState::ResetDrawGenesis()
{
  ResetWorldMatrices();
  glCallList(resetStateList);
}


/******************************************************************************/
//
//  WorldStart
//

void OpenGLPassState::EnableDrawWorldStart()
{
  EnableCommon(DRAW_WORLD_START);
  resetMatrixFunc = ResetWorldMatrices;
  ResetWorldMatrices();
  SetupWorldLighting();
}


void OpenGLPassState::DisableDrawWorldStart()
{
  ResetWorldMatrices();
  RevertWorldLighting();
  DisableCommon(DRAW_WORLD_START);
}


void OpenGLPassState::ResetDrawWorldStart()
{
  ResetWorldMatrices();
  glCallList(resetStateList);
}


/******************************************************************************/
//
//  World
//

void OpenGLPassState::EnableDrawWorld()
{
  EnableCommon(DRAW_WORLD);
  resetMatrixFunc = ResetWorldMatrices;
  SetupWorldLighting();
}


void OpenGLPassState::DisableDrawWorld()
{
  ResetWorldMatrices();
  RevertWorldLighting();
  DisableCommon(DRAW_WORLD);
}


void OpenGLPassState::ResetDrawWorld()
{
  ResetWorldMatrices();
  glCallList(resetStateList);
}


/******************************************************************************/
//
//  WorldAlpha -- the same as World
//

void OpenGLPassState::EnableDrawWorldAlpha()
{
  EnableCommon(DRAW_WORLD);
  resetMatrixFunc = ResetWorldMatrices;
  SetupWorldLighting();
}


void OpenGLPassState::DisableDrawWorldAlpha()
{
  ResetWorldMatrices();
  RevertWorldLighting();
  DisableCommon(DRAW_WORLD);
}


void OpenGLPassState::ResetDrawWorldAlpha()
{
  ResetWorldMatrices();
  glCallList(resetStateList);
}


/******************************************************************************/
//
//  WorldShadow
//

void OpenGLPassState::EnableDrawWorldShadow()
{
  EnableCommon(DRAW_WORLD_SHADOW);
  resetMatrixFunc = ResetWorldShadowMatrices;
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glPolygonOffset(1.0f, 1.0f);
  glEnable(GL_POLYGON_OFFSET_FILL);
  glEnable(GL_VERTEX_PROGRAM_ARB);
//FIXME  glBindProgramARB(GL_VERTEX_PROGRAM_ARB, unitDrawer->unitShadowGenVP);
}


void OpenGLPassState::DisableDrawWorldShadow()
{
  glDisable(GL_VERTEX_PROGRAM_ARB);
  glDisable(GL_POLYGON_OFFSET_FILL);
  ResetWorldShadowMatrices();
  DisableCommon(DRAW_WORLD_SHADOW);
}


void OpenGLPassState::ResetDrawWorldShadow()
{
  ResetWorldShadowMatrices();
  glCallList(resetStateList);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glPolygonOffset(1.0f, 1.0f);
  glEnable(GL_POLYGON_OFFSET_FILL);
}


/******************************************************************************/
//
//  ScreenStart -- same as Screen
//

void OpenGLPassState::EnableDrawScreenStart()
{
  EnableCommon(DRAW_SCREEN);
  resetMatrixFunc = ResetScreenMatrices;

  SetupScreenMatrices();
  SetupScreenLighting();
  glCallList(resetStateList);
  glEnable(GL_NORMALIZE);
}


void OpenGLPassState::DisableDrawScreenStart()
{
  RevertScreenLighting();
  RevertScreenMatrices();
  DisableCommon(DRAW_SCREEN);
  glEnable(GL_SCISSOR_TEST); // FIXME ?
}


void OpenGLPassState::ResetDrawScreenStart()
{
  ResetScreenMatrices();
  glCallList(resetStateList);
}


/******************************************************************************/
//
//  Screen
//

void OpenGLPassState::EnableDrawScreen()
{
  EnableCommon(DRAW_SCREEN);
  resetMatrixFunc = ResetScreenMatrices;

  SetupScreenMatrices();
  SetupScreenLighting();
  glCallList(resetStateList);
  glEnable(GL_NORMALIZE);
}


void OpenGLPassState::DisableDrawScreen()
{
  RevertScreenLighting();
  RevertScreenMatrices();
  DisableCommon(DRAW_SCREEN);
  glEnable(GL_SCISSOR_TEST); // FIXME ?
}


void OpenGLPassState::ResetDrawScreen()
{
  ResetScreenMatrices();
  glCallList(resetStateList);
}


/******************************************************************************/
//
//  Radar
//

void OpenGLPassState::EnableDrawRadar()
{
  if (drawMode == DRAW_SCREEN) {
    prevDrawMode = DRAW_SCREEN;
    drawMode = DRAW_NONE;
  }
  EnableCommon(DRAW_RADAR);
  resetMatrixFunc = ResetRadarMatrices;
  // CMiniMap::DrawForReal() does not setup the texture matrix
  glMatrixMode(GL_TEXTURE); {
    ClearMatrixStack(GL_TEXTURE_STACK_DEPTH);
    glLoadIdentity();
  }
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
//FIXME  if (minimap) {
//FIXME    glScalef(1.0f / (float)minimap->GetSizeX(),
//FIXME             1.0f / (float)minimap->GetSizeY(), 1.0f);
//FIXME  }
}


void OpenGLPassState::DisableDrawRadar()
{
  if (prevDrawMode != DRAW_SCREEN) {
    DisableCommon(DRAW_RADAR);
  }
  else {
    glPopAttrib();
    resetMatrixFunc = ResetScreenMatrices;
    ResetScreenMatrices();
    prevDrawMode = DRAW_NONE;
    drawMode = DRAW_SCREEN;
  }
}


void OpenGLPassState::ResetDrawRadar()
{
  ResetRadarMatrices();
  glCallList(resetStateList);
}


/******************************************************************************/
/******************************************************************************/

void OpenGLPassState::SetupWorldLighting()
{
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
//FIXME  glLightfv(GL_LIGHT1, GL_POSITION, mapInfo->light.sunDir);
  glEnable(GL_LIGHT1);
}


void OpenGLPassState::RevertWorldLighting()
{
  glDisable(GL_LIGHT1);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
}


void OpenGLPassState::SetupScreenMatrices()
{
  BzfDisplay* dpy = getDisplay();
  MainWindow* wnd = getMainWindow();
  BzfWindow* bzWnd = wnd->getWindow();
  if ((dpy == NULL) || (wnd == NULL) || (bzWnd == NULL)) {
    return;
  }

  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
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
  const float znear  = zplane * 0.5;
  const float zfar   = zplane * 2.0;
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
  const float* diffuse = RENDERER.getSunColor();
  const float* ambient = RENDERER.getAmbientColor();
  const float* sunDir  = RENDERER.getSunDirection();
  if ((diffuse == NULL) || (ambient == NULL) || (sunDir == NULL)) {
    return;
  }

  // back light
  const float backLightPos[4]  = { 1.0f, 2.0f, 2.0f, 0.0f };
  const float backLightAmbt[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  const float backLightDiff[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
  const float backLightSpec[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  glLightfv(GL_LIGHT0, GL_POSITION, backLightPos);
  glLightfv(GL_LIGHT0, GL_AMBIENT,  backLightAmbt);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  backLightDiff);
  glLightfv(GL_LIGHT0, GL_SPECULAR, backLightSpec);

  // sun light -- needs the camera transformation
  glPushMatrix();
  glLoadIdentity();
  RENDERER.getViewFrustum().executeView();
  glLightfv(GL_LIGHT1, GL_POSITION, sunDir);
  glPopMatrix();

  const float sunFactor = 1.0f;
  const float sf = sunFactor;
  const float* la = ambient;
  const float* ld = diffuse;

  const float sunLightAmbt[4] = { la[0]*sf, la[1]*sf, la[2]*sf, la[3]*sf };
  const float sunLightDiff[4] = { ld[0]*sf, ld[1]*sf, ld[2]*sf, ld[3]*sf };
  const float sunLightSpec[4] = { la[0]*sf, la[1]*sf, la[2]*sf, la[3]*sf };
  glLightfv(GL_LIGHT1, GL_AMBIENT,  sunLightAmbt);
  glLightfv(GL_LIGHT1, GL_DIFFUSE,  sunLightDiff);
  glLightfv(GL_LIGHT1, GL_SPECULAR, sunLightSpec);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

  // Enable the GL lights
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
}


void OpenGLPassState::RevertScreenLighting()
{
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
  glDisable(GL_LIGHT1);
  glDisable(GL_LIGHT0);
}


/******************************************************************************/
/******************************************************************************/

void OpenGLPassState::ResetWorldMatrices()
{
  glMatrixMode(GL_TEXTURE); {
    ClearMatrixStack(GL_TEXTURE_STACK_DEPTH);
    glLoadIdentity();
  }
  glMatrixMode(GL_PROJECTION); {
    ClearMatrixStack(GL_PROJECTION_STACK_DEPTH);
//FIXME    glLoadMatrixd(camera->GetProjection());
  }
  glMatrixMode(GL_MODELVIEW); {
    ClearMatrixStack(GL_MODELVIEW_STACK_DEPTH);
//FIXME    glLoadMatrixd(camera->GetModelview());
  }
}


void OpenGLPassState::ResetWorldShadowMatrices()
{
  glMatrixMode(GL_TEXTURE); {
    ClearMatrixStack(GL_TEXTURE_STACK_DEPTH);
    glLoadIdentity();
  }
  glMatrixMode(GL_PROJECTION); {
    ClearMatrixStack(GL_PROJECTION_STACK_DEPTH);
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, -1.0);
  }
  glMatrixMode(GL_MODELVIEW); {
    ClearMatrixStack(GL_MODELVIEW_STACK_DEPTH);
//FIXME    glLoadMatrixf(shadowHandler->shadowMatrix.m);
  }
}


void OpenGLPassState::ResetScreenMatrices()
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
  SetupScreenMatrices();
}


void OpenGLPassState::ResetRadarMatrices()
{
  glMatrixMode(GL_TEXTURE); {
    ClearMatrixStack(GL_TEXTURE_STACK_DEPTH);
    glLoadIdentity();
  }
  glMatrixMode(GL_PROJECTION); {
    ClearMatrixStack(GL_PROJECTION_STACK_DEPTH);
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0e6, +1.0e6);
  }
  glMatrixMode(GL_MODELVIEW); {
    ClearMatrixStack(GL_MODELVIEW_STACK_DEPTH);
    glLoadIdentity();
//FIXME    if (minimap) {
//FIXME      glScalef(1.0f / (float)minimap->GetSizeX(),
//FIXME               1.0f / (float)minimap->GetSizeY(), 1.0f);
//FIXME    }
  }
}


/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/******************************************************************************/
