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

//
// TODO:
// - go back to counting matrix push/pops (just for modelview?)
//   would have to make sure that display lists are also handled
//   (GL_MODELVIEW_STACK_DEPTH could help current situation, but
//    requires the ARB_imaging extension)
// - use materials instead of raw calls (again, handle dlists)
//

#include "common.h"

// interface header
#include "LuaOpenGL.h"

// system headers
//#include <string.h>
#include <string>
#include <vector>
#include <set>
using std::string;
using std::vector;
using std::set;

// common headers
#include "FontManager.h"
#include "OpenGLGState.h"
#include "OpenGLPassState.h"
#include "SceneRenderer.h"
#include "StateDatabase.h"
#include "CacheManager.h"
#include "TextUtils.h"

// local headers
#include "LuaHeader.h"
#include "LuaHandle.h"
#include "LuaShaders.h"
#include "LuaTextures.h"
#include "LuaFBOs.h"
#include "LuaRBOs.h"
#include "LuaGLBuffers.h"
#include "LuaGLPointers.h"
#include "LuaGLQueries.h"
#include "LuaDLists.h"


#undef far // avoid collision with windef.h
#undef near


const float LuaOpenGL::defaultTextSize = 12.0f;


//============================================================================//
//============================================================================//

void LuaOpenGL::Init()
{
}


void LuaOpenGL::Free()
{
}


//============================================================================//
//============================================================================//

static set<string> forbidden;


static void ParseForbidden()
{
  static set<string> knownExts;
  if (knownExts.empty()) {
    knownExts.insert("FBO");
    knownExts.insert("Buffer");
    knownExts.insert("Query");
    knownExts.insert("Shader");
    knownExts.insert("DepthClamp");
    knownExts.insert("PointSprite");
    knownExts.insert("SeparateBlend");
    knownExts.insert("SeparateStencil");
  }

  forbidden.clear();

  const string extList = BZDB.get("forbidLuaGL");

  if (extList == "*") {
    forbidden = knownExts;
  }
  else {
    const vector<string> args = TextUtils::tokenize(extList, ",");
    for (size_t i = 0; i < args.size(); i++) {
      const string& ext = args[i];
      if (knownExts.find(ext) != knownExts.end()) {
	forbidden.insert(ext);
      } else {
	LuaLog(1, "LuaGL: unknown extension \"%s\"\n", ext.c_str());
      }
    }
  }

  set<string>::const_iterator it;
  for (it = forbidden.begin(); it != forbidden.end(); ++it) {
    LuaLog(1, "LuaGL: forbidding extension \"%s\"\n", (*it).c_str());
  }
}


static bool IsAllowed(const string& ext)
{
  return (forbidden.find(ext) == forbidden.end());
}


//============================================================================//
//============================================================================//

bool LuaOpenGL::PushEntries(lua_State* L)
{
  ParseForbidden();

  PUSH_LUA_CFUNC(L, HasExtension);
  PUSH_LUA_CFUNC(L, GetNumber);
  PUSH_LUA_CFUNC(L, GetString);
  PUSH_LUA_CFUNC(L, GetError);

  PUSH_LUA_CFUNC(L, ConfigScreen);

  PUSH_LUA_CFUNC(L, ResetState);
  PUSH_LUA_CFUNC(L, ResetMatrices);
  PUSH_LUA_CFUNC(L, Clear);

  PUSH_LUA_CFUNC(L, Hint);
  PUSH_LUA_CFUNC(L, Lighting);
  PUSH_LUA_CFUNC(L, LightModel);
  PUSH_LUA_CFUNC(L, ShadeModel);
  PUSH_LUA_CFUNC(L, Scissor);
  PUSH_LUA_CFUNC(L, Viewport);
  PUSH_LUA_CFUNC(L, ColorMask);
  PUSH_LUA_CFUNC(L, DepthMask);
  PUSH_LUA_CFUNC(L, DepthTest);
  if (GLEW_NV_depth_clamp && IsAllowed("DepthClamp")) {
    PUSH_LUA_CFUNC(L, DepthClamp);
  }
  PUSH_LUA_CFUNC(L, Culling);
  PUSH_LUA_CFUNC(L, FrontFace);
  PUSH_LUA_CFUNC(L, LogicOp);
  PUSH_LUA_CFUNC(L, Fog);
  PUSH_LUA_CFUNC(L, Smoothing);
  PUSH_LUA_CFUNC(L, AlphaTest);
  PUSH_LUA_CFUNC(L, LineStipple);
  PUSH_LUA_CFUNC(L, PolygonStipple);
  PUSH_LUA_CFUNC(L, Normalize);
  PUSH_LUA_CFUNC(L, Blending);
  PUSH_LUA_CFUNC(L, BlendEquation);
  PUSH_LUA_CFUNC(L, BlendFunc);
  if (GLEW_VERSION_2_0 && IsAllowed("SeparateBlend")) {
    PUSH_LUA_CFUNC(L, BlendEquationSeparate);
    PUSH_LUA_CFUNC(L, BlendFuncSeparate);
  }

  PUSH_LUA_CFUNC(L, Color);
  PUSH_LUA_CFUNC(L, Material);
  PUSH_LUA_CFUNC(L, Ambient);
  PUSH_LUA_CFUNC(L, Diffuse);
  PUSH_LUA_CFUNC(L, Emission);
  PUSH_LUA_CFUNC(L, Specular);
  PUSH_LUA_CFUNC(L, Shininess);

  PUSH_LUA_CFUNC(L, PolygonMode);
  PUSH_LUA_CFUNC(L, PolygonOffset);

  PUSH_LUA_CFUNC(L, StencilTest);
  PUSH_LUA_CFUNC(L, StencilMask);
  PUSH_LUA_CFUNC(L, StencilFunc);
  PUSH_LUA_CFUNC(L, StencilOp);
  if (GLEW_VERSION_2_0 && IsAllowed("SeparateStencil")) {
    PUSH_LUA_CFUNC(L, StencilMaskSeparate);
    PUSH_LUA_CFUNC(L, StencilFuncSeparate);
    PUSH_LUA_CFUNC(L, StencilOpSeparate);
  }

  PUSH_LUA_CFUNC(L, LineWidth);
  PUSH_LUA_CFUNC(L, PointSize);
  if (GLEW_VERSION_2_0 && IsAllowed("PointSprite")) {
    PUSH_LUA_CFUNC(L, PointSprite);
    PUSH_LUA_CFUNC(L, PointParameter);
  }

  PUSH_LUA_CFUNC(L, BeginEnd);
  PUSH_LUA_CFUNC(L, Vertex);
  PUSH_LUA_CFUNC(L, Normal);
  PUSH_LUA_CFUNC(L, TexCoord);
  PUSH_LUA_CFUNC(L, MultiTexCoord);
  PUSH_LUA_CFUNC(L, SecondaryColor);
  PUSH_LUA_CFUNC(L, FogCoord);
  PUSH_LUA_CFUNC(L, EdgeFlag);
  if (glVertexAttrib1f  &&
      glVertexAttrib2fv &&
      glVertexAttrib3fv &&
      glVertexAttrib4fv) {
    PUSH_LUA_CFUNC(L, VertexAttrib);
  }

  PUSH_LUA_CFUNC(L, Rect);
  PUSH_LUA_CFUNC(L, TexRect);

  PUSH_LUA_CFUNC(L, Text);
  PUSH_LUA_CFUNC(L, GetTextWidth);
  PUSH_LUA_CFUNC(L, GetTextHeight);

  PUSH_LUA_CFUNC(L, Map1);
  PUSH_LUA_CFUNC(L, Map2);
  PUSH_LUA_CFUNC(L, MapGrid1);
  PUSH_LUA_CFUNC(L, MapGrid2);
  PUSH_LUA_CFUNC(L, Eval);
  PUSH_LUA_CFUNC(L, EvalEnable);
  PUSH_LUA_CFUNC(L, EvalDisable);
  PUSH_LUA_CFUNC(L, EvalMesh1);
  PUSH_LUA_CFUNC(L, EvalMesh2);
  PUSH_LUA_CFUNC(L, EvalCoord1);
  PUSH_LUA_CFUNC(L, EvalCoord2);
  PUSH_LUA_CFUNC(L, EvalPoint1);
  PUSH_LUA_CFUNC(L, EvalPoint2);

  PUSH_LUA_CFUNC(L, Light);
  PUSH_LUA_CFUNC(L, ClipPlane);

  PUSH_LUA_CFUNC(L, MatrixMode);
  PUSH_LUA_CFUNC(L, LoadIdentity);
  PUSH_LUA_CFUNC(L, LoadMatrix);
  PUSH_LUA_CFUNC(L, MultMatrix);
  PUSH_LUA_CFUNC(L, Translate);
  PUSH_LUA_CFUNC(L, Scale);
  PUSH_LUA_CFUNC(L, Rotate);
  PUSH_LUA_CFUNC(L, Ortho);
  PUSH_LUA_CFUNC(L, Frustum);
  PUSH_LUA_CFUNC(L, PushMatrix);
  PUSH_LUA_CFUNC(L, PopMatrix);
  PUSH_LUA_CFUNC(L, PushPopMatrix);
  PUSH_LUA_CFUNC(L, Billboard);
  PUSH_LUA_CFUNC(L, GetMatrixData);

  PUSH_LUA_CFUNC(L, PushAttrib);
  PUSH_LUA_CFUNC(L, PopAttrib);
  PUSH_LUA_CFUNC(L, PushPopAttrib);
  PUSH_LUA_CFUNC(L, UnsafeState);

  PUSH_LUA_CFUNC(L, Flush);
  PUSH_LUA_CFUNC(L, Finish);

  PUSH_LUA_CFUNC(L, RenderMode);
  PUSH_LUA_CFUNC(L, SelectBuffer);
  PUSH_LUA_CFUNC(L, SelectBufferData);
  PUSH_LUA_CFUNC(L, InitNames);
  PUSH_LUA_CFUNC(L, PushName);
  PUSH_LUA_CFUNC(L, PopName);
  PUSH_LUA_CFUNC(L, PushPopName);
  PUSH_LUA_CFUNC(L, LoadName);

  PUSH_LUA_CFUNC(L, ReadPixels);

  if (GLEW_VERSION_2_0 && IsAllowed("Query")) {
    LuaGLQueryMgr::PushEntries(L);
  }

  LuaDListMgr::PushEntries(L);
  LuaTextureMgr::PushEntries(L);

  if (IsAllowed("Shader")) {
    LuaShaderMgr::PushEntries(L);
  }

  // should be GL_ARB_framebuffer_object, but glew 1.5.1 has
  // a typo from the following:  glFramebufferTextureLayer
  // (it should be fixed in the next version of glew)
  if (GLEW_VERSION_2_0 && glGenFramebuffers && glGenRenderbuffers &&
      IsAllowed("FBO")) {
     LuaFBOMgr::PushEntries(L);
     LuaRBOMgr::PushEntries(L);
  }

  LuaGLPointers::PushEntries(L);

  if (GLEW_VERSION_1_5 && IsAllowed("Buffer")) {
    LuaGLBufferMgr::PushEntries(L);
  }

  return true;
}


//============================================================================//
//============================================================================//

void LuaOpenGL::CheckDrawingEnabled(lua_State* L, const char* caller)
{
  if (!OpenGLPassState::IsDrawingEnabled()) {
    luaL_error(L, "%s(): OpenGL calls can only be used in Draw() "
		  "call-ins, or while creating display lists", caller);
  }
}


static int ParseFloatArray(lua_State* L, float* array, int size)
{
  if (!lua_istable(L, -1)) {
    return -1;
  }
  const int table = lua_gettop(L);
  for (int i = 0; i < size; i++) {
    lua_rawgeti(L, table, (i + 1));
    if (lua_israwnumber(L, -1)) {
      array[i] = lua_tofloat(L, -1);
      lua_pop(L, 1);
    } else {
      lua_pop(L, 1);
      return i;
    }
  }
  return size;
}


//============================================================================//

int LuaOpenGL::HasExtension(lua_State* L)
{
  const char* extName = luaL_checkstring(L, 1);
  lua_pushboolean(L, glewIsSupported(extName));
  return 1;
}


int LuaOpenGL::GetNumber(lua_State* L)
{
  const GLenum pname = (GLenum)luaL_checknumber(L, 1);
  const GLuint count = (GLuint)luaL_optnumber(L, 2, 1);
  if (count > 64) {
    return luaL_pushnil(L);
  }
  lua_checkstack(L, count);
  GLfloat values[64];
  glGetFloatv(pname, values);
  for (GLuint i = 0; i < count; i++) {
    lua_pushfloat(L, values[i]);
  }
  return count;
}


int LuaOpenGL::GetString(lua_State* L)
{
  const GLenum pname = (GLenum)luaL_checknumber(L, 1);
  lua_pushstring(L, (const char*)glGetString(pname));
  return 1;
}


int LuaOpenGL::GetError(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  lua_pushinteger(L, glGetError());
  return 1;
}


int LuaOpenGL::ConfigScreen(lua_State* L)
{
//  CheckDrawingEnabled(L, __FUNCTION__);
  const float screenWidth    = luaL_checkfloat(L, 1);
  const float screenDistance = luaL_checkfloat(L, 2);
  OpenGLPassState::ConfigScreen(screenWidth, screenDistance);
  return 0;
}


int LuaOpenGL::Text(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const char* text = luaL_checkstring(L, 1);
  const char* face = luaL_checkstring(L, 2);
  const float size = luaL_optfloat(L, 3, defaultTextSize);
  const float x    = luaL_optfloat(L, 4, 0.0f);
  const float y    = luaL_optfloat(L, 5, 0.0f);
  const char* opts = luaL_optstring(L, 6, "");
  float alpha = 1.0f;
  bool useAlpha = false;
  if (lua_israwnumber(L, 7)) {
    alpha = lua_tonumber(L, 7);
    useAlpha = true;
  }

  int faceID = -1;
  FontManager& FM = FontManager::instance();
  if (!CacheManager::isCacheFileType(face)) {
    faceID = FM.getFaceID(face);
  }
  else {
    // try the URL cache
    const string localName = CACHEMGR.getLocalName(face);
    faceID = FM.lookupFileID(localName);
    if (faceID < 0) {
      faceID = FM.load(localName.c_str());
    }
  }
  // backup plan
  if (faceID < 0) {
    faceID = FM.getFaceID("junkName"); // get the default face
  }

  bool right = false;
  bool center = false;
  bool outline = false;
  bool colorCodes = true;
  bool rawBlending = false;
  bool lightOut;

  if (opts != NULL) {
    const char* c = opts;
    while (*c != 0) {
      switch (*c) {
	case 'c': { center = true;		    break; }
	case 'r': { right = true;		     break; }
	case 'n': { colorCodes = false;	       break; }
	case '*': { rawBlending = true;	       break; }
	case 'o': { outline = true; lightOut = false; break; }
	case 'O': { outline = true; lightOut = true;  break; }
      }
      c++;
    }
  }

  float xj = x; // justified x position
  if (right) {
    xj -= FM.getStringWidth(faceID, size, text, false);
  } else if (center) {
    xj -= FM.getStringWidth(faceID, size, text, false) * 0.5f;
  }

  const int ftglAttribDepth = 2;

  if (!OpenGLPassState::TryAttribStackChange(+ftglAttribDepth)) {
    luaL_error(L, "attrib stack overflow");
  }

  float oldAlpha = 1.0f;
  if (useAlpha) {
    oldAlpha = FM.getOpacity();
    FM.setOpacity(alpha);
  }

  glPushMatrix();
  glTranslatef(xj, y, 0.0f);
  if (!rawBlending) {
    FM.drawString(0.0f, 0.0f, 0.0f, faceID, size, text);
  } else {
    FM.setRawBlending(true);
    FM.drawString(0.0f, 0.0f, 0.0f, faceID, size, text);
    FM.setRawBlending(false);
  }
  glPopMatrix();

  if (useAlpha) {
    FM.setOpacity(oldAlpha);
  }

  if (!OpenGLPassState::TryAttribStackChange(-ftglAttribDepth)) {
    luaL_error(L, "attrib stack underflow");
  }

  return 0;
}


int LuaOpenGL::GetTextWidth(lua_State* L)
{
  const char* text = luaL_checkstring(L, 1);
  const char* face = luaL_checkstring(L, 2);
  const float size = luaL_optfloat(L, 3, defaultTextSize);

  FontManager& FM = FontManager::instance();
  const int faceID = FM.getFaceID(face);
  lua_pushfloat(L, FM.getStringWidth(faceID, size, text, false));

  return 1;
}


int LuaOpenGL::GetTextHeight(lua_State* L)
{
//  const char* text = luaL_checkstring(L, 1); unused ...
  const char* face = luaL_checkstring(L, 2);
  const float size = luaL_optfloat(L, 3, defaultTextSize);

  FontManager& FM = FontManager::instance();
  const int faceID = FM.getFaceID(face);
  lua_pushfloat(L, FM.getStringHeight(faceID, size));

  return 1;
}


//============================================================================//
//============================================================================//
//
//  GL evaluators
//

static int evalDepth = 0;


static int GetMap1TargetDataSize(GLenum target)
{
  switch (target) {
    case GL_MAP1_COLOR_4:	 { return 4; }
    case GL_MAP1_INDEX:	   { return 1; }
    case GL_MAP1_NORMAL:	  { return 3; }
    case GL_MAP1_VERTEX_3:	{ return 3; }
    case GL_MAP1_VERTEX_4:	{ return 4; }
    case GL_MAP1_TEXTURE_COORD_1: { return 1; }
    case GL_MAP1_TEXTURE_COORD_2: { return 2; }
    case GL_MAP1_TEXTURE_COORD_3: { return 3; }
    case GL_MAP1_TEXTURE_COORD_4: { return 4; }
    default:		      { break; }
  }
  return 0;
}

static int GetMap2TargetDataSize(GLenum target)
{
  switch (target) {
    case GL_MAP2_COLOR_4:	 { return 4; }
    case GL_MAP2_INDEX:	   { return 1; }
    case GL_MAP2_NORMAL:	  { return 3; }
    case GL_MAP2_VERTEX_3:	{ return 3; }
    case GL_MAP2_VERTEX_4:	{ return 4; }
    case GL_MAP2_TEXTURE_COORD_1: { return 1; }
    case GL_MAP2_TEXTURE_COORD_2: { return 2; }
    case GL_MAP2_TEXTURE_COORD_3: { return 3; }
    case GL_MAP2_TEXTURE_COORD_4: { return 4; }
    default:		      { break; }
  }
  return 0;
}



int LuaOpenGL::Eval(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int funcIndex = 1;
  luaL_checktype(L, funcIndex, LUA_TFUNCTION);

  if (evalDepth == 0) {
    if (!OpenGLPassState::PushAttrib(GL_EVAL_BIT)) {
      luaL_error(L, "attrib stack overflow");
    }
  }

  evalDepth++;
  const int top = lua_gettop(L);
  const int error = lua_pcall(L, top - funcIndex, LUA_MULTRET, 0);
  evalDepth--;

  if (evalDepth == 0) {
    if (!OpenGLPassState::PopAttrib()) {
      luaL_error(L, "attrib stack underflow");
    }
  }

  if (error != 0) {
    LuaLog(1, "gl.Eval: error(%i) = %s", error, lua_tostring(L, -1));
    lua_error(L);
  }

  return lua_gettop(L) - (funcIndex - 1);
}


int LuaOpenGL::EvalEnable(lua_State* L)
{
  if (evalDepth <= 0) {
    luaL_error(L, "EvalState can only be used in Eval() blocks");
  }
  const GLenum target = (GLenum)luaL_checkint(L, 1);
  if ((GetMap1TargetDataSize(target) > 0) ||
      (GetMap2TargetDataSize(target) > 0) ||
      (target == GL_AUTO_NORMAL)) {
    glEnable(target);
  }
  return 0;
}


int LuaOpenGL::EvalDisable(lua_State* L)
{
  if (evalDepth <= 0) {
    luaL_error(L, "EvalState can only be used in Eval() blocks");
  }
  const GLenum target = (GLenum)luaL_checkint(L, 1);
  if ((GetMap1TargetDataSize(target) > 0) ||
      (GetMap2TargetDataSize(target) > 0) ||
      (target == GL_AUTO_NORMAL)) {
    glDisable(target);
  }
  return 0;
}


int LuaOpenGL::Map1(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  if (lua_gettop(L) != 6) { // NOTE: required for ParseFloatArray()
    return 0;
  }
  const GLenum  target = (GLenum)luaL_checkint(L, 1);
  const GLfloat u1     = luaL_checkfloat(L, 2);
  const GLfloat u2     = luaL_checkfloat(L, 3);
  const GLint   stride = luaL_checkint(L, 4);
  const GLint   order  = luaL_checkint(L, 5);

  const int dataSize = GetMap1TargetDataSize(target);
  if (dataSize <= 0) {
    return 0;
  }
  if ((order <= 0) || (stride != dataSize)) {
    return 0;
  }
  const int fullSize = (order * dataSize);
  float* points = new float[fullSize];
  if (ParseFloatArray(L, points, fullSize) == fullSize) {
    glMap1f(target, u1, u2, stride, order, points);
  }
  delete[] points;
  return 0;
}


int LuaOpenGL::Map2(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  if (lua_gettop(L) != 10) { // NOTE: required for ParseFloatArray()
    return 0;
  }

  const GLenum  target  = (GLenum)luaL_checkint(L, 1);
  const GLfloat u1      = luaL_checkfloat(L, 2);
  const GLfloat u2      = luaL_checkfloat(L, 3);
  const GLint   ustride = luaL_checkint(L, 4);
  const GLint   uorder  = luaL_checkint(L, 5);
  const GLfloat v1      = luaL_checkfloat(L, 6);
  const GLfloat v2      = luaL_checkfloat(L, 7);
  const GLint   vstride = luaL_checkint(L, 8);
  const GLint   vorder  = luaL_checkint(L, 9);

  const int dataSize = GetMap2TargetDataSize(target);
  if (dataSize <= 0) {
    return 0;
  }
  if ((uorder  <= 0) || (vorder  <= 0) ||
      (ustride != dataSize) || (vstride != (dataSize * uorder))) {
    return 0;
  }
  const int fullSize = (uorder * vorder * dataSize);
  float* points = new float[fullSize];
  if (ParseFloatArray(L, points, fullSize) == fullSize) {
    glMap2f(target, u1, u2, ustride, uorder,
		    v1, v2, vstride, vorder, points);
  }
  delete[] points;
  return 0;
}


int LuaOpenGL::MapGrid1(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLint   un = luaL_checkint(L, 1);
  const GLfloat u1 = luaL_checkfloat(L, 2);
  const GLfloat u2 = luaL_checkfloat(L, 3);
  glMapGrid1f(un, u1, u2);
  return 0;
}


int LuaOpenGL::MapGrid2(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLint   un = luaL_checkint(L, 1);
  const GLfloat u1 = luaL_checkfloat(L, 2);
  const GLfloat u2 = luaL_checkfloat(L, 3);
  const GLint   vn = luaL_checkint(L, 4);
  const GLfloat v1 = luaL_checkfloat(L, 5);
  const GLfloat v2 = luaL_checkfloat(L, 6);
  glMapGrid2f(un, u1, u2, vn, v1, v2);
  return 0;
}


int LuaOpenGL::EvalMesh1(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLenum mode = (GLenum)luaL_checkint(L, 1);
  const GLint  i1   = luaL_checkint(L, 2);
  const GLint  i2   = luaL_checkint(L, 3);
  glEvalMesh1(mode, i1, i2);
  return 0;
}


int LuaOpenGL::EvalMesh2(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLenum mode = (GLenum)luaL_checkint(L, 1);
  const GLint  i1   = luaL_checkint(L, 2);
  const GLint  i2   = luaL_checkint(L, 3);
  const GLint  j1   = luaL_checkint(L, 4);
  const GLint  j2   = luaL_checkint(L, 5);
  glEvalMesh2(mode, i1, i2, j1, j2);
  return 0;
}


int LuaOpenGL::EvalCoord1(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLfloat u = luaL_checkfloat(L, 1);
  glEvalCoord1f(u);
  return 0;
}


int LuaOpenGL::EvalCoord2(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLfloat u = luaL_checkfloat(L, 1);
  const GLfloat v = luaL_checkfloat(L, 2);
  glEvalCoord2f(u, v);
  return 0;
}


int LuaOpenGL::EvalPoint1(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLint i = luaL_checkint(L, 1);
  glEvalPoint1(i);
  return 0;
}


int LuaOpenGL::EvalPoint2(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLint i = luaL_checkint(L, 1);
  const GLint j = luaL_checkint(L, 1);
  glEvalPoint2(i, j);
  return 0;
}


//============================================================================//
//============================================================================//

int LuaOpenGL::BeginEnd(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const GLuint primMode = (GLuint)luaL_checkint(L, 1);

  const int funcIndex = 2;
  luaL_checktype(L, funcIndex, LUA_TFUNCTION);

  // call the function
  glBegin(primMode);
  const int top = lua_gettop(L);
  const int error = lua_pcall(L, top - funcIndex, LUA_MULTRET, 0);
  glEnd();

  if (error != 0) {
    LuaLog(1, "gl.BeginEnd: error(%i) = %s", error, lua_tostring(L, -1));
    lua_error(L);
  }

  return lua_gettop(L) - (funcIndex - 1);
}


//============================================================================//

int LuaOpenGL::Vertex(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);

  if (args == 1) {
    if (!lua_istable(L, 1)) {
      luaL_error(L, "Bad data passed to gl.Vertex()");
    }
    lua_rawgeti(L, 1, 1);
    if (!lua_israwnumber(L, -1)) {
      luaL_error(L, "Bad data passed to gl.Vertex()");
    }
    const float x = lua_tofloat(L, -1);
    lua_rawgeti(L, 1, 2);
    if (!lua_israwnumber(L, -1)) {
      luaL_error(L, "Bad data passed to gl.Vertex()");
    }
    const float y = lua_tofloat(L, -1);
    lua_rawgeti(L, 1, 3);
    if (!lua_israwnumber(L, -1)) {
      glVertex2f(x, y);
      return 0;
    }
    const float z = lua_tofloat(L, -1);
    lua_rawgeti(L, 1, 4);
    if (!lua_israwnumber(L, -1)) {
      glVertex3f(x, y, z);
      return 0;
    }
    const float w = lua_tofloat(L, -1);
    glVertex4f(x, y, z, w);
    return 0;
  }

  if (args == 3) {
    if (!lua_israwnumber(L, 1) ||
	!lua_israwnumber(L, 2) ||
	!lua_israwnumber(L, 3)) {
      luaL_error(L, "Bad data passed to gl.Vertex()");
    }
    const float x = lua_tofloat(L, 1);
    const float y = lua_tofloat(L, 2);
    const float z = lua_tofloat(L, 3);
    glVertex3f(x, y, z);
  }
  else if (args == 2) {
    if (!lua_israwnumber(L, 1) || !lua_israwnumber(L, 2)) {
      luaL_error(L, "Bad data passed to gl.Vertex()");
    }
    const float x = lua_tofloat(L, 1);
    const float y = lua_tofloat(L, 2);
    glVertex2f(x, y);
  }
  else if (args == 4) {
    if (!lua_israwnumber(L, 1) || !lua_israwnumber(L, 2) ||
	!lua_israwnumber(L, 3) || !lua_israwnumber(L, 4)) {
      luaL_error(L, "Bad data passed to gl.Vertex()");
    }
    const float x = lua_tofloat(L, 1);
    const float y = lua_tofloat(L, 2);
    const float z = lua_tofloat(L, 3);
    const float w = lua_tofloat(L, 4);
    glVertex4f(x, y, z, w);
  }
  else {
    luaL_error(L, "Incorrect arguments to gl.Vertex()");
  }

  return 0;
}


int LuaOpenGL::Normal(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);

  if (args == 1) {
    if (!lua_istable(L, 1)) {
      luaL_error(L, "Bad data passed to gl.Normal()");
    }
    lua_rawgeti(L, 1, 1);
    if (!lua_israwnumber(L, -1)) {
      luaL_error(L, "Bad data passed to gl.Normal()");
    }
    const float x = lua_tofloat(L, -1);
    lua_rawgeti(L, 1, 2);
    if (!lua_israwnumber(L, -1)) {
      luaL_error(L, "Bad data passed to gl.Normal()");
    }
    const float y = lua_tofloat(L, -1);
    lua_rawgeti(L, 1, 3);
    if (!lua_israwnumber(L, -1)) {
      luaL_error(L, "Bad data passed to gl.Normal()");
    }
    const float z = lua_tofloat(L, -1);
    glNormal3f(x, y, z);
    return 0;
  }

  if (args < 3) {
    luaL_error(L, "Incorrect arguments to gl.Normal()");
  }
  const float x = lua_tofloat(L, 1);
  const float y = lua_tofloat(L, 2);
  const float z = lua_tofloat(L, 3);
  glNormal3f(x, y, z);
  return 0;
}


int LuaOpenGL::TexCoord(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);

  if (args == 1) {
    if (lua_israwnumber(L, 1)) {
      const float x = lua_tofloat(L, 1);
      glTexCoord1f(x);
      return 0;
    }
    if (!lua_istable(L, 1)) {
      luaL_error(L, "Bad 1data passed to gl.TexCoord()");
    }
    lua_rawgeti(L, 1, 1);
    if (!lua_israwnumber(L, -1)) {
      luaL_error(L, "Bad 2data passed to gl.TexCoord()");
    }
    const float x = lua_tofloat(L, -1);
    lua_rawgeti(L, 1, 2);
    if (!lua_israwnumber(L, -1)) {
      glTexCoord1f(x);
      return 0;
    }
    const float y = lua_tofloat(L, -1);
    lua_rawgeti(L, 1, 3);
    if (!lua_israwnumber(L, -1)) {
      glTexCoord2f(x, y);
      return 0;
    }
    const float z = lua_tofloat(L, -1);
    lua_rawgeti(L, 1, 4);
    if (!lua_israwnumber(L, -1)) {
      glTexCoord3f(x, y, z);
      return 0;
    }
    const float w = lua_tofloat(L, -1);
    glTexCoord4f(x, y, z, w);
    return 0;
  }

  if (args == 2) {
    if (!lua_israwnumber(L, 1) || !lua_israwnumber(L, 2)) {
      luaL_error(L, "Bad data passed to gl.TexCoord()");
    }
    const float x = lua_tofloat(L, 1);
    const float y = lua_tofloat(L, 2);
    glTexCoord2f(x, y);
  }
  else if (args == 3) {
    if (!lua_israwnumber(L, 1) ||
	!lua_israwnumber(L, 2) ||
	!lua_israwnumber(L, 3)) {
      luaL_error(L, "Bad data passed to gl.TexCoord()");
    }
    const float x = lua_tofloat(L, 1);
    const float y = lua_tofloat(L, 2);
    const float z = lua_tofloat(L, 3);
    glTexCoord3f(x, y, z);
  }
  else if (args == 4) {
    if (!lua_israwnumber(L, 1) || !lua_israwnumber(L, 2) ||
	!lua_israwnumber(L, 3) || !lua_israwnumber(L, 4)) {
      luaL_error(L, "Bad data passed to gl.TexCoord()");
    }
    const float x = lua_tofloat(L, 1);
    const float y = lua_tofloat(L, 2);
    const float z = lua_tofloat(L, 3);
    const float w = lua_tofloat(L, 4);
    glTexCoord4f(x, y, z, w);
  }
  else {
    luaL_error(L, "Incorrect arguments to gl.TexCoord()");
  }
  return 0;
}


int LuaOpenGL::MultiTexCoord(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int texNum = luaL_checkint(L, 1);
  if ((texNum < 0) || (texNum >= MAX_LUA_TEXTURE_UNITS)) {
    luaL_error(L, "Bad texture unit passed to gl.MultiTexCoord()");
  }
  const GLenum texUnit = GL_TEXTURE0 + texNum;

  const int args = lua_gettop(L) - 1;

  if (args == 1) {
    if (lua_israwnumber(L, 2)) {
      const float x = lua_tofloat(L, 2);
      glMultiTexCoord1f(texUnit, x);
      return 0;
    }
    if (!lua_istable(L, 2)) {
      luaL_error(L, "Bad data passed to gl.MultiTexCoord()");
    }
    lua_rawgeti(L, 2, 1);
    if (!lua_israwnumber(L, -1)) {
      luaL_error(L, "Bad data passed to gl.MultiTexCoord()");
    }
    const float x = lua_tofloat(L, -1);
    lua_rawgeti(L, 2, 2);
    if (!lua_israwnumber(L, -1)) {
      glMultiTexCoord1f(texUnit, x);
      return 0;
    }
    const float y = lua_tofloat(L, -1);
    lua_rawgeti(L, 2, 3);
    if (!lua_israwnumber(L, -1)) {
      glMultiTexCoord2f(texUnit, x, y);
      return 0;
    }
    const float z = lua_tofloat(L, -1);
    lua_rawgeti(L, 2, 4);
    if (!lua_israwnumber(L, -1)) {
      glMultiTexCoord3f(texUnit, x, y, z);
      return 0;
    }
    const float w = lua_tofloat(L, -1);
    glMultiTexCoord4f(texUnit, x, y, z, w);
    return 0;
  }

  if (args == 2) {
    if (!lua_israwnumber(L, 2) || !lua_israwnumber(L, 3)) {
      luaL_error(L, "Bad data passed to gl.MultiTexCoord()");
    }
    const float x = lua_tofloat(L, 2);
    const float y = lua_tofloat(L, 3);
    glMultiTexCoord2f(texUnit, x, y);
  }
  else if (args == 3) {
    if (!lua_israwnumber(L, 2) ||
	!lua_israwnumber(L, 3) ||
	!lua_israwnumber(L, 4)) {
      luaL_error(L, "Bad data passed to gl.MultiTexCoord()");
    }
    const float x = lua_tofloat(L, 2);
    const float y = lua_tofloat(L, 3);
    const float z = lua_tofloat(L, 4);
    glMultiTexCoord3f(texUnit, x, y, z);
  }
  else if (args == 4) {
    if (!lua_israwnumber(L, 2) || !lua_israwnumber(L, 3) ||
	!lua_israwnumber(L, 4) || !lua_israwnumber(L, 5)) {
      luaL_error(L, "Bad data passed to gl.MultiTexCoord()");
    }
    const float x = lua_tofloat(L, 2);
    const float y = lua_tofloat(L, 3);
    const float z = lua_tofloat(L, 4);
    const float w = lua_tofloat(L, 5);
    glMultiTexCoord4f(texUnit, x, y, z, w);
  }
  else {
    luaL_error(L, "Incorrect arguments to gl.MultiTexCoord()");
  }
  return 0;
}


int LuaOpenGL::SecondaryColor(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);

  if (args == 1) {
    if (!lua_istable(L, 1)) {
      luaL_error(L, "Bad data passed to gl.SecondaryColor()");
    }
    lua_rawgeti(L, 1, 1);
    if (!lua_israwnumber(L, -1)) {
      luaL_error(L, "Bad data passed to gl.SecondaryColor()");
    }
    const float x = lua_tofloat(L, -1);
    lua_rawgeti(L, 1, 2);
    if (!lua_israwnumber(L, -1)) {
      luaL_error(L, "Bad data passed to gl.SecondaryColor()");
    }
    const float y = lua_tofloat(L, -1);
    lua_rawgeti(L, 1, 3);
    if (!lua_israwnumber(L, -1)) {
      luaL_error(L, "Bad data passed to gl.SecondaryColor()");
    }
    const float z = lua_tofloat(L, -1);
    glSecondaryColor3f(x, y, z);
    return 0;
  }

  if (args < 3) {
    luaL_error(L, "Incorrect arguments to gl.SecondaryColor()");
  }
  const float x = lua_tofloat(L, 1);
  const float y = lua_tofloat(L, 2);
  const float z = lua_tofloat(L, 3);
  glSecondaryColor3f(x, y, z);
  return 0;
}


int LuaOpenGL::FogCoord(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const float value = luaL_checkfloat(L, 1);
  glFogCoordf(value);
  return 0;
}


int LuaOpenGL::EdgeFlag(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  if (lua_isboolean(L, 1)) {
    glEdgeFlag(lua_tobool(L, 1));
  }
  return 0;
}


int LuaOpenGL::VertexAttrib(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const GLuint index = luaL_checkint(L, 1);
  const int elemSize = lua_gettop(L) - 1;
  switch (elemSize) {
    case 1: {
      const float value = luaL_checkfloat(L, 2);
      glVertexAttrib1f(index, value);
      break;
    }
    case 2: {
      const fvec2 value(luaL_checkfloat(L, 2),
			luaL_checkfloat(L, 3));
      glVertexAttrib2fv(index, value);
      break;
    }
    case 3: {
      const fvec3 value(luaL_checkfloat(L, 2),
			luaL_checkfloat(L, 3),
			luaL_checkfloat(L, 4));
      glVertexAttrib3fv(index, value);
      break;
    }
    case 4: {
      const fvec4 value(luaL_checkfloat(L, 2),
			luaL_checkfloat(L, 3),
			luaL_checkfloat(L, 4),
			luaL_checkfloat(L, 5));
      glVertexAttrib4fv(index, value);
      break;
    }
    default: {
      luaL_error(L, "bad VertexAttrib size");
    }
  }
  return 0;
}


//============================================================================//

int LuaOpenGL::Rect(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const float x1 = luaL_checkfloat(L, 1);
  const float y1 = luaL_checkfloat(L, 2);
  const float x2 = luaL_checkfloat(L, 3);
  const float y2 = luaL_checkfloat(L, 4);

  glRectf(x1, y1, x2, y2);
  return 0;
}


int LuaOpenGL::TexRect(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const float x1 = luaL_checkfloat(L, 1);
  const float y1 = luaL_checkfloat(L, 2);
  const float x2 = luaL_checkfloat(L, 3);
  const float y2 = luaL_checkfloat(L, 4);

  const int args = lua_gettop(L);

  if (args <= 6) {
    float s0 = 0.0f;
    float t0 = 0.0f;
    float s1 = 1.0f;
    float t1 = 1.0f;
    if ((args >= 5) && lua_isboolean(L, 5) && lua_tobool(L, 5)) {
      // flip s-coords
      s0 = 1.0f;
      s1 = 0.0f;
    }
    if ((args >= 6) && lua_isboolean(L, 6) && lua_tobool(L, 6)) {
      // flip t-coords
      t0 = 1.0f;
      t1 = 0.0f;
    }
    glBegin(GL_QUADS); {
      glTexCoord2f(s0, t0); glVertex2f(x1, y1);
      glTexCoord2f(s1, t0); glVertex2f(x2, y1);
      glTexCoord2f(s1, t1); glVertex2f(x2, y2);
      glTexCoord2f(s0, t1); glVertex2f(x1, y2);
    }
    glEnd();
    return 0;
  }

  const float s0 = luaL_checkfloat(L, 5);
  const float t0 = luaL_checkfloat(L, 6);
  const float s1 = luaL_checkfloat(L, 7);
  const float t1 = luaL_checkfloat(L, 8);
  glBegin(GL_QUADS); {
    glTexCoord2f(s0, t0); glVertex2f(x1, y1);
    glTexCoord2f(s1, t0); glVertex2f(x2, y1);
    glTexCoord2f(s1, t1); glVertex2f(x2, y2);
    glTexCoord2f(s0, t1); glVertex2f(x1, y2);
  }
  glEnd();

  return 0;
}


//============================================================================//

static bool ParseColor(lua_State* L, fvec4& color, const char* funcName)
{
  const int args = lua_gettop(L);
  if (args < 1) {
    luaL_error(L, "Incorrect arguments to %s", funcName);
  }

  if (args == 1) {
    if (!lua_istable(L, 1)) {
      luaL_error(L, "Incorrect arguments to %s", funcName);
    }
    const int count = ParseFloatArray(L, color, 4);
    if (count < 3) {
      luaL_error(L, "Incorrect arguments to %s", funcName);
    }
    if (count == 3) {
      color[3] = 1.0f;
    }
  }
  else if (args >= 3) {
    color[0] = (GLfloat)luaL_checkfloat(L, 1);
    color[1] = (GLfloat)luaL_checkfloat(L, 2);
    color[2] = (GLfloat)luaL_checkfloat(L, 3);
    if (args < 4) {
      color[3] = 1.0f;
    } else {
      color[3] = (GLfloat)luaL_checkfloat(L, 4);
    }
  }
  else {
    luaL_error(L, "Incorrect arguments to %s", funcName);
  }

  return true;
}


int LuaOpenGL::Color(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  fvec4 color;
  ParseColor(L, color, __FUNCTION__);
  glColor4fv(color);

  return 0;
}


int LuaOpenGL::Material(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if ((args != 1) || !lua_istable(L, 1)) {
    luaL_error(L, "Incorrect arguments to gl.Material(table)");
  }

  fvec4 color;

  const int table = lua_gettop(L);
  for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
    if (!lua_israwstring(L, -2)) { // the key
      LuaLog(1, "gl.Material: bad state type");
      return 0;
    }
    const string key = lua_tostring(L, -2);

    if (key == "shininess") {
      if (lua_israwnumber(L, -1)) {
	const GLfloat specExp = (GLfloat)lua_tonumber(L, -1);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, specExp);
      }
      continue;
    }

    const int count = ParseFloatArray(L, color, 4);
    if (count == 3) {
      color[3] = 1.0f;
    }

    if (key == "ambidiff") {
      if (count >= 3) {
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
      }
    }
    else if (key == "ambient") {
      if (count >= 3) {
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);
      }
    }
    else if (key == "diffuse") {
      if (count >= 3) {
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
      }
    }
    else if (key == "specular") {
      if (count >= 3) {
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
      }
    }
    else if (key == "emission") {
      if (count >= 3) {
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, color);
      }
    }
    else {
      LuaLog(1, "gl.Material: unknown material type: %s", key.c_str());
    }
  }
  return 0;
}


int LuaOpenGL::Ambient(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  fvec4 color;
  ParseColor(L, color, __FUNCTION__);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);

  return 0;
}


int LuaOpenGL::Diffuse(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  fvec4 color;
  ParseColor(L, color, __FUNCTION__);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);

  return 0;
}


int LuaOpenGL::Emission(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  fvec4 color;
  ParseColor(L, color, __FUNCTION__);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, color);

  return 0;
}


int LuaOpenGL::Specular(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  fvec4 color;
  ParseColor(L, color, __FUNCTION__);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);

  return 0;
}


int LuaOpenGL::Shininess(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const float shininess = luaL_checkfloat(L, 1);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

  return 0;
}


//============================================================================//

int LuaOpenGL::ResetState(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if (args != 0) {
    luaL_error(L, "gl.ResetState takes no arguments");
  }

  OpenGLPassState::ResetState();

  return 0;
}


int LuaOpenGL::ResetMatrices(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if (args != 0) {
    luaL_error(L, "gl.ResetMatrices takes no arguments");
  }

  OpenGLPassState::ResetMatrices();

  return 0;
}


int LuaOpenGL::Lighting(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if ((args != 1) || !lua_isboolean(L, 1)) {
    luaL_error(L, "Incorrect arguments to gl.Lighting()");
  }
  if (lua_tobool(L, 1)) {
    glEnable(GL_LIGHTING);
  } else {
    glDisable(GL_LIGHTING);
  }
  return 0;
}


int LuaOpenGL::LightModel(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const GLenum pname = (GLenum)luaL_checkint(L, 1);

  const int args = lua_gettop(L);
  if (args > 17) {
    luaL_error(L, "too many parameters");
  }

  GLfloat params[16] = { 0.0f };
  for (int p = 2; p <= args; p++) {
    params[p - 2] = luaL_checkfloat(L, p);
  }

  glLightModelfv(pname, params);

  return 0;
}


int LuaOpenGL::ShadeModel(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if ((args != 1) || !lua_israwnumber(L, 1)) {
    luaL_error(L, "Incorrect arguments to gl.ShadeModel()");
  }

  glShadeModel((GLenum)lua_tointeger(L, 1));

  return 0;
}


int LuaOpenGL::Scissor(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if (args == 1) {
    if (!lua_isboolean(L, 1)) {
      luaL_error(L, "Incorrect arguments to gl.Scissor()");
    }
    if (lua_tobool(L, 1)) {
      glEnable(GL_SCISSOR_TEST);
    } else {
      glDisable(GL_SCISSOR_TEST);
    }
  }
  else if (args == 4) {
    glEnable(GL_SCISSOR_TEST);
    const GLint   x =   (GLint)luaL_checkint(L, 1);
    const GLint   y =   (GLint)luaL_checkint(L, 2);
    const GLsizei w = (GLsizei)luaL_checkint(L, 3);
    const GLsizei h = (GLsizei)luaL_checkint(L, 4);
    glScissor(x, y, w, h);
  }
  else {
    luaL_error(L, "Incorrect arguments to gl.Scissor()");
  }

  return 0;
}


int LuaOpenGL::Viewport(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int x = luaL_checkint(L, 1);
  const int y = luaL_checkint(L, 1);
  const int w = luaL_checkint(L, 1);
  const int h = luaL_checkint(L, 1);

  glViewport(x, y, w, h);

  return 0;
}


int LuaOpenGL::ColorMask(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if ((args == 1) && lua_isboolean(L, 1)) {
    if (!lua_isboolean(L, 1)) {
      luaL_error(L, "Incorrect arguments to gl.ColorMask()");
    }
    if (lua_tobool(L, 1)) {
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    } else {
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    }
  }
  else if ((args == 4) &&
	   lua_isboolean(L, 1) && lua_isboolean(L, 1) &&
	   lua_isboolean(L, 3) && lua_isboolean(L, 4)) {
    glColorMask(lua_tobool(L, 1), lua_tobool(L, 2),
		lua_tobool(L, 3), lua_tobool(L, 4));
  }
  else {
    luaL_error(L, "Incorrect arguments to gl.ColorMask()");
  }
  return 0;
}


int LuaOpenGL::DepthMask(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if ((args != 1) || !lua_isboolean(L, 1)) {
    luaL_error(L, "Incorrect arguments to gl.DepthMask()");
  }
  if (lua_tobool(L, 1)) {
    glDepthMask(GL_TRUE);
  } else {
    glDepthMask(GL_FALSE);
  }
  return 0;
}


int LuaOpenGL::DepthTest(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if (args != 1) {
    luaL_error(L, "Incorrect arguments to gl.DepthTest()");
  }

  if (lua_isboolean(L, 1)) {
    if (lua_tobool(L, 1)) {
      glEnable(GL_DEPTH_TEST);
    } else {
      glDisable(GL_DEPTH_TEST);
    }
  }
  else if (lua_israwnumber(L, 1)) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc((GLenum)lua_tointeger(L, 1));
  }
  else {
    luaL_error(L, "Incorrect arguments to gl.DepthTest()");
  }
  return 0;
}


int LuaOpenGL::DepthClamp(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  luaL_checktype(L, 1, LUA_TBOOLEAN);
  if (lua_tobool(L, 1)) {
    glEnable(GL_DEPTH_CLAMP_NV);
  } else {
    glDisable(GL_DEPTH_CLAMP_NV);
  }
  return 0;
}


int LuaOpenGL::Culling(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if (args != 1) {
    luaL_error(L, "Incorrect arguments to gl.Culling()");
  }

  if (lua_isboolean(L, 1)) {
    if (lua_tobool(L, 1)) {
      glEnable(GL_CULL_FACE);
    } else {
      glDisable(GL_CULL_FACE);
    }
  }
  else if (lua_israwnumber(L, 1)) {
    glEnable(GL_CULL_FACE);
    glCullFace((GLenum)lua_tointeger(L, 1));
  }
  else {
    luaL_error(L, "Incorrect arguments to gl.Culling()");
  }
  return 0;
}


int LuaOpenGL::FrontFace(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const GLenum face = (GLenum)luaL_checkint(L, 1);
  glFrontFace(face);

  return 0;
}


int LuaOpenGL::LogicOp(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if (args != 1) {
    luaL_error(L, "Incorrect arguments to gl.LogicOp()");
  }

  if (lua_isboolean(L, 1)) {
    if (lua_tobool(L, 1)) {
      glEnable(GL_COLOR_LOGIC_OP);
    } else {
      glDisable(GL_COLOR_LOGIC_OP);
    }
  }
  else if (lua_israwnumber(L, 1)) {
    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp((GLenum)lua_tointeger(L, 1));
  }
  else {
    luaL_error(L, "Incorrect arguments to gl.LogicOp()");
  }
  return 0;
}


int LuaOpenGL::Fog(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if ((args != 1) || !lua_isboolean(L, 1)) {
    luaL_error(L, "Incorrect arguments to gl.Fog()");
  }

  if (lua_tobool(L, 1)) {
    glEnable(GL_FOG);
  } else {
    glDisable(GL_FOG);
  }
  return 0;
}


int LuaOpenGL::Blending(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if (args == 1) {
    if (lua_isboolean(L, 1)) {
      if (lua_tobool(L, 1)) {
	glEnable(GL_BLEND);
      } else {
	glDisable(GL_BLEND);
      }
    }
    else if (lua_israwstring(L, 1)) {
      const string mode = lua_tostring(L, 1);
      if (mode == "add") {
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);
      }
      else if (mode == "alpha_add") {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);
      }
      else if ((mode == "alpha") ||
	       (mode == "reset")) {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
      }
      else if (mode == "color") {
	glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
	glEnable(GL_BLEND);
      }
      else if (mode == "modulate") {
	glBlendFunc(GL_DST_COLOR, GL_ZERO);
	glEnable(GL_BLEND);
      }
      else if (mode == "disable") {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
      }
    }
    else {
      luaL_error(L, "Incorrect argument to gl.Blending()");
    }
  }
  else if (args == 2) {
    const GLenum src = (GLenum)luaL_checkint(L, 1);
    const GLenum dst = (GLenum)luaL_checkint(L, 2);
    glBlendFunc(src, dst);
    glEnable(GL_BLEND);
  }
  else {
    luaL_error(L, "Incorrect arguments to gl.Blending()");
  }
  return 0;
}


int LuaOpenGL::BlendEquation(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLenum mode = (GLenum)luaL_checkint(L, 1);
  glBlendEquation(mode);
  return 0;
}


int LuaOpenGL::BlendFunc(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLenum src = (GLenum)luaL_checkint(L, 1);
  const GLenum dst = (GLenum)luaL_checkint(L, 2);
  glBlendFunc(src, dst);
  return 0;
}


int LuaOpenGL::BlendEquationSeparate(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLenum modeRGB   = (GLenum)luaL_checkint(L, 1);
  const GLenum modeAlpha = (GLenum)luaL_checkint(L, 2);
  glBlendEquationSeparate(modeRGB, modeAlpha);
  return 0;
}


int LuaOpenGL::BlendFuncSeparate(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLenum srcRGB   = (GLenum)luaL_checkint(L, 1);
  const GLenum dstRGB   = (GLenum)luaL_checkint(L, 2);
  const GLenum srcAlpha = (GLenum)luaL_checkint(L, 3);
  const GLenum dstAlpha = (GLenum)luaL_checkint(L, 4);
  glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
  return 0;
}


int LuaOpenGL::Smoothing(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const static struct {
    const char* name;
    GLenum hintEnum;
    GLenum enableEnum;
  } smoothTypes[3] = {
    { "point",   GL_POINT_SMOOTH_HINT,   GL_POINT_SMOOTH   },
    { "line",    GL_LINE_SMOOTH_HINT,    GL_LINE_SMOOTH    },
    { "polygon", GL_POLYGON_SMOOTH_HINT, GL_POLYGON_SMOOTH }
  };

  for (int i = 0; i < 3; i++) {
    const GLenum hintEnum   = smoothTypes[i].hintEnum;
    const GLenum enableEnum = smoothTypes[i].enableEnum;
    const int luaIndex = (i + 1);
    const int type = lua_type(L, luaIndex);
    if (type == LUA_TBOOLEAN) {
      if (lua_tobool(L, luaIndex)) {
	glEnable(enableEnum);
      } else {
	glDisable(enableEnum);
      }
    }
    else if (type == LUA_TNUMBER) {
      const GLenum hint = (GLenum)lua_tointeger(L, luaIndex);
      if ((hint == GL_FASTEST) || (hint == GL_NICEST) || (hint == GL_DONT_CARE)) {
	glHint(hintEnum, hint);
	glEnable(enableEnum);
      } else {
	luaL_error(L, "Bad %s hint in gl.Smoothing()", smoothTypes[i].name);
      }
    }
  }
  return 0;
}


int LuaOpenGL::AlphaTest(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if (args == 1) {
    if (!lua_isboolean(L, 1)) {
      luaL_error(L, "Incorrect arguments to gl.AlphaTest()");
    }
    if (lua_tobool(L, 1)) {
      glEnable(GL_ALPHA_TEST);
    } else {
      glDisable(GL_ALPHA_TEST);
    }
  }
  else if (args == 2) {
    if (!lua_israwnumber(L, 1) || !lua_israwnumber(L, 2)) {
      luaL_error(L, "Incorrect arguments to gl.AlphaTest()");
    }
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc((GLenum)lua_tointeger(L, 1), (GLfloat)lua_tonumber(L, 2));
  }
  else {
    luaL_error(L, "Incorrect arguments to gl.AlphaTest()");
  }
  return 0;
}


int LuaOpenGL::PolygonMode(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLenum face = (GLenum)luaL_checkint(L, 1);
  const GLenum mode = (GLenum)luaL_checkint(L, 2);
  glPolygonMode(face, mode);
  return 0;
}


int LuaOpenGL::PolygonOffset(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if (args == 1) {
    if (!lua_isboolean(L, 1)) {
      luaL_error(L, "Incorrect arguments to gl.PolygonOffset()");
    }
    if (lua_tobool(L, 1)) {
      glEnable(GL_POLYGON_OFFSET_FILL);
      glEnable(GL_POLYGON_OFFSET_LINE);
      glEnable(GL_POLYGON_OFFSET_POINT);
    } else {
      glDisable(GL_POLYGON_OFFSET_FILL);
      glDisable(GL_POLYGON_OFFSET_LINE);
      glDisable(GL_POLYGON_OFFSET_POINT);
    }
  }
  else if (args == 2) {
    const float factor = luaL_checkfloat(L, 1);
    const float units  = luaL_checkfloat(L, 2);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glEnable(GL_POLYGON_OFFSET_LINE);
    glEnable(GL_POLYGON_OFFSET_POINT);
    glPolygonOffset(factor, units);
  }
  else {
    luaL_error(L, "Incorrect arguments to gl.PolygonOffset()");
  }
  return 0;
}


//============================================================================//

int LuaOpenGL::StencilTest(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  luaL_checktype(L, 1, LUA_TBOOLEAN);
  if (lua_tobool(L, 1)) {
    glEnable(GL_STENCIL_TEST);
  } else {
    glDisable(GL_STENCIL_TEST);
  }
  return 0;
}


int LuaOpenGL::StencilMask(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLuint mask = luaL_checkint(L, 1);
  glStencilMask(mask);
  return 0;
}


int LuaOpenGL::StencilFunc(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLenum func = luaL_checkint(L, 1);
  const GLint  ref  = luaL_checkint(L, 2);
  const GLuint mask = luaL_checkint(L, 3);
  glStencilFunc(func, ref, mask);
  return 0;
}


int LuaOpenGL::StencilOp(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLenum fail  = luaL_checkint(L, 1);
  const GLenum zfail = luaL_checkint(L, 2);
  const GLenum zpass = luaL_checkint(L, 3);
  glStencilOp(fail, zfail, zpass);
  return 0;
}


int LuaOpenGL::StencilMaskSeparate(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLenum face = luaL_checkint(L, 1);
  const GLuint mask = luaL_checkint(L, 2);
  glStencilMaskSeparate(face, mask);
  return 0;
}


int LuaOpenGL::StencilFuncSeparate(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLenum face = luaL_checkint(L, 1);
  const GLenum func = luaL_checkint(L, 2);
  const GLint  ref  = luaL_checkint(L, 3);
  const GLuint mask = luaL_checkint(L, 4);
  glStencilFuncSeparate(face, func, ref, mask);
  return 0;
}


int LuaOpenGL::StencilOpSeparate(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLenum face  = luaL_checkint(L, 1);
  const GLenum fail  = luaL_checkint(L, 2);
  const GLenum zfail = luaL_checkint(L, 3);
  const GLenum zpass = luaL_checkint(L, 4);
  glStencilOpSeparate(face, fail, zfail, zpass);
  return 0;
}


//============================================================================//

int LuaOpenGL::LineStipple(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  if (lua_isboolean(L, 1)) {
    if (!lua_tobool(L, 1)) {
      glDisable(GL_LINE_STIPPLE);
    }
    return 0;
  }

  GLint factor     =    (GLint)luaL_checkint(L, 1);
  GLushort pattern = (GLushort)luaL_checkint(L, 2);

  if (lua_israwnumber(L, 3)) {
    int shift = lua_toint(L, 3);
    while (shift < 0) { shift += 16; }
    shift = (shift % 16);
    unsigned int pat = pattern & 0xFFFF;
    pat = pat | (pat << 16);
    pattern = pat >> shift;
  }

  glEnable(GL_LINE_STIPPLE);
  glLineStipple(factor, pattern);

  return 0;
}


int LuaOpenGL::PolygonStipple(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  if (lua_isboolean(L, 1)) {
    if (!lua_tobool(L, 1)) {
      glDisable(GL_POLYGON_STIPPLE);
    }
    return 0;
  }

  const float alpha = luaL_checkfloat(L, 1);
  const int numStipples = OpenGLGState::getOpaqueStippleIndex() + 1;
  const int stipple = (int)(alpha * numStipples);

  OpenGLGState::setStippleIndex(stipple);

  glEnable(GL_POLYGON_STIPPLE);

  return 0;
}


int LuaOpenGL::Normalize(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  luaL_checktype(L, 1, LUA_TBOOLEAN);
  if (lua_tobool(L, 1)) {
    glEnable(GL_NORMALIZE);
  } else {
    glDisable(GL_NORMALIZE);
  }

  return 0;
}


int LuaOpenGL::LineWidth(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const float width = luaL_checkfloat(L, 1);
  glLineWidth(width);
  return 0;
}


int LuaOpenGL::PointSize(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const float size = luaL_checkfloat(L, 1);
  glPointSize(size);
  return 0;
}


int LuaOpenGL::PointSprite(lua_State* L)
{
  const int args = lua_gettop(L);
  if ((args < 1) || !lua_isboolean(L, 1)) {
    luaL_error(L, "Incorrect arguments to gl.PointSprite()");
  }
  if (lua_tobool(L, 1)) {
    glEnable(GL_POINT_SPRITE);
  } else {
    glDisable(GL_POINT_SPRITE);
  }
  if ((args >= 2) && lua_isboolean(L, 2)) {
    if (lua_tobool(L, 2)) {
      glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    } else {
      glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_FALSE);
    }
  }
  if ((args >= 3) && lua_isboolean(L, 3)) {
    if (lua_tobool(L, 3)) {
      glTexEnvi(GL_POINT_SPRITE, GL_POINT_SPRITE_COORD_ORIGIN, GL_UPPER_LEFT);
    } else {
      glTexEnvi(GL_POINT_SPRITE, GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
    }
  }
  return 0;
}


int LuaOpenGL::PointParameter(lua_State* L)
{
  GLfloat atten[3];
  atten[0] = (GLfloat)luaL_checknumber(L, 1);
  atten[1] = (GLfloat)luaL_checknumber(L, 2);
  atten[2] = (GLfloat)luaL_checknumber(L, 3);
  glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, atten);

  const int args = lua_gettop(L);
  if (args >= 4) {
    const float sizeMin = luaL_checkfloat(L, 4);
    glPointParameterf(GL_POINT_SIZE_MIN, sizeMin);
  }
  if (args >= 5) {
    const float sizeMax = luaL_checkfloat(L, 5);
    glPointParameterf(GL_POINT_SIZE_MAX, sizeMax);
  }
  if (args >= 6) {
    const float sizeFade = luaL_checkfloat(L, 6);
    glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, sizeFade);
  }

  return 0;
}


//============================================================================//

int LuaOpenGL::Clear(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if ((args < 1) || !lua_israwnumber(L, 1)) {
    luaL_error(L, "Incorrect arguments to gl.Clear()");
  }

  const GLbitfield bits = (GLbitfield)lua_tointeger(L, 1);
  if (args == 5) {
    if (!lua_israwnumber(L, 2) || !lua_israwnumber(L, 3) ||
	!lua_israwnumber(L, 4) || !lua_israwnumber(L, 5)) {
      luaL_error(L, "Incorrect arguments to Clear()");
    }
    if (bits == GL_COLOR_BUFFER_BIT) {
      glClearColor((GLfloat)lua_tonumber(L, 2), (GLfloat)lua_tonumber(L, 3),
		   (GLfloat)lua_tonumber(L, 4), (GLfloat)lua_tonumber(L, 5));
    }
    else if (bits == GL_ACCUM_BUFFER_BIT) {
      glClearAccum((GLfloat)lua_tonumber(L, 2), (GLfloat)lua_tonumber(L, 3),
		   (GLfloat)lua_tonumber(L, 4), (GLfloat)lua_tonumber(L, 5));
    }
  }
  else if (args == 2) {
    if (!lua_israwnumber(L, 2)) {
      luaL_error(L, "Incorrect arguments to gl.Clear()");
    }
    if (bits == GL_DEPTH_BUFFER_BIT) {
      glClearDepth((GLfloat)lua_tonumber(L, 2));
    }
    else if (bits == GL_STENCIL_BUFFER_BIT) {
      glClearStencil((GLint)lua_tointeger(L, 2));
    }
  }

  glClear(bits);

  return 0;
}


//============================================================================//

int LuaOpenGL::Hint(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLenum target = (GLenum)luaL_checkint(L, 1);
  const GLenum mode   = (GLenum)luaL_checkint(L, 2);
  glHint(target, mode);
  return 0;
}


//============================================================================//

int LuaOpenGL::Translate(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const float x = luaL_checkfloat(L, 1);
  const float y = luaL_checkfloat(L, 2);
  const float z = luaL_checkfloat(L, 3);
  glTranslatef(x, y, z);
  return 0;
}


int LuaOpenGL::Scale(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const float x = luaL_checkfloat(L, 1);
  const float y = luaL_checkfloat(L, 2);
  const float z = luaL_checkfloat(L, 3);
  glScalef(x, y, z);
  return 0;
}


int LuaOpenGL::Rotate(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const float r = luaL_checkfloat(L, 1);
  const float x = luaL_checkfloat(L, 2);
  const float y = luaL_checkfloat(L, 3);
  const float z = luaL_checkfloat(L, 4);
  glRotatef(r, x, y, z);
  return 0;
}


int LuaOpenGL::Ortho(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const float left   = luaL_checkfloat(L, 1);
  const float right  = luaL_checkfloat(L, 2);
  const float bottom = luaL_checkfloat(L, 3);
  const float top    = luaL_checkfloat(L, 4);
  const float near   = luaL_checkfloat(L, 5);
  const float far    = luaL_checkfloat(L, 6);
  glOrtho(left, right, bottom, top, near, far);
  return 0;
}


int LuaOpenGL::Frustum(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const float left   = luaL_checkfloat(L, 1);
  const float right  = luaL_checkfloat(L, 2);
  const float bottom = luaL_checkfloat(L, 3);
  const float top    = luaL_checkfloat(L, 4);
  const float near   = luaL_checkfloat(L, 5);
  const float far    = luaL_checkfloat(L, 6);
  glFrustum(left, right, bottom, top, near, far);
  return 0;
}


int LuaOpenGL::Billboard(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  RENDERER.getViewFrustum().executeBillboard();
  return 0;
}


//============================================================================//

int LuaOpenGL::Light(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const GLenum light = GL_LIGHT0 + (GLint)luaL_checknumber(L, 1);
  if ((light < GL_LIGHT0) || (light > GL_LIGHT7)) {
    luaL_error(L, "Bad light number in gl.Light");
  }

  if (lua_isboolean(L, 2)) {
    if (lua_tobool(L, 2)) {
      glEnable(light);
    } else {
      glDisable(light);
    }
    return 0;
  }

  const int args = lua_gettop(L);
  if (args == 3) {
    const GLenum  pname = (GLenum)luaL_checknumber(L, 2);
    const GLfloat param = (GLfloat)luaL_checkfloat(L, 3);
    glLightf(light, pname, param);
  }
  else if (args == 5) {
    GLfloat array[4]; // NOTE: 4 instead of 3  (to be safe)
    const GLenum pname = (GLenum)luaL_checknumber(L, 2);
    array[0] = (GLfloat)luaL_checknumber(L, 3);
    array[1] = (GLfloat)luaL_checknumber(L, 4);
    array[2] = (GLfloat)luaL_checknumber(L, 5);
    array[3] = 0.0f;
    glLightfv(light, pname, array);
  }
  else if (args == 6) {
    GLfloat array[4];
    const GLenum pname = (GLenum)luaL_checknumber(L, 2);
    array[0] = (GLfloat)luaL_checknumber(L, 3);
    array[1] = (GLfloat)luaL_checknumber(L, 4);
    array[2] = (GLfloat)luaL_checknumber(L, 5);
    array[3] = (GLfloat)luaL_checknumber(L, 6);
    glLightfv(light, pname, array);
  }
  else {
    luaL_error(L, "Incorrect arguments to gl.Light");
  }

  return 0;
}


int LuaOpenGL::ClipPlane(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const GLenum plane = (GLenum)luaL_checkint(L, 1) + GL_CLIP_PLANE0;
  if ((plane < GL_CLIP_PLANE0) || (plane > GL_CLIP_PLANE5)) {
    luaL_error(L, "gl.ClipPlane: bad plane number");
  }
  if (lua_isboolean(L, 2)) {
    if (lua_tobool(L, 2)) {
      glEnable(plane);
    } else {
      glDisable(plane);
    }
    return 0;
  }
  GLdouble equation[4];
  equation[0] = (double)luaL_checknumber(L, 2);
  equation[1] = (double)luaL_checknumber(L, 3);
  equation[2] = (double)luaL_checknumber(L, 4);
  equation[3] = (double)luaL_checknumber(L, 5);
  glClipPlane(plane, equation);
  glEnable(plane);
  return 0;
}


//============================================================================//

int LuaOpenGL::MatrixMode(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  glMatrixMode((GLenum)luaL_checkint(L, 1));
  return 0;
}


int LuaOpenGL::LoadIdentity(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  glLoadIdentity();
  return 0;
}


//============================================================================//

static float Calculate4x4Cofactor(const float m[4][4], int ei, int ej)
{
  int ai, bi, ci;
  switch (ei) {
    case 0:  { ai = 1; bi = 2; ci = 3; break; }
    case 1:  { ai = 0; bi = 2; ci = 3; break; }
    case 2:  { ai = 0; bi = 1; ci = 3; break; }
    case 3:  { ai = 0; bi = 1; ci = 2; break; }
    default: { ai = 1; bi = 2; ci = 3; break; } // for warnings
  }
  int aj, bj, cj;
  switch (ej) {
    case 0:  { aj = 1; bj = 2; cj = 3; break; }
    case 1:  { aj = 0; bj = 2; cj = 3; break; }
    case 2:  { aj = 0; bj = 1; cj = 3; break; }
    case 3:  { aj = 0; bj = 1; cj = 2; break; }
    default: { aj = 1; bj = 2; cj = 3; break; } // for warnings
  }

  const float val =
      (m[ai][aj] * ((m[bi][bj] * m[ci][cj]) - (m[ci][bj] * m[bi][cj])))
    - (m[ai][bj] * ((m[bi][aj] * m[ci][cj]) - (m[ci][aj] * m[bi][cj])))
    + (m[ai][cj] * ((m[bi][aj] * m[ci][bj]) - (m[ci][aj] * m[bi][bj])));

  if (((ei + ej) % 2) == 0) {
    return +val;
  } else {
    return -val;
  }
}


static bool CalculateInverse4x4(const float m[4][4], float inv[4][4])
{
  float cofac[4][4];
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      cofac[i][j] = Calculate4x4Cofactor(m, i, j);
    }
  }

  const float det = (m[0][0] * cofac[0][0]) +
		    (m[0][1] * cofac[0][1]) +
		    (m[0][2] * cofac[0][2]) +
		    (m[0][3] * cofac[0][3]);

  if (det <= 1.0e-9f) {
    // singular matrix, set to identity?
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
	inv[i][j] = (i == j) ? 1.0f : 0.0f;
      }
    }
    return false;
  }

  const float scale = 1.0f / det;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      inv[i][j] = cofac[j][i] * scale; // (adjoint / determinant)
				       // (note the transposition in 'cofac')
    }
  }

  return true;
}


static const float* GetNamedMatrix(const string& name)
{
  if (name == "camera") {
    const ViewFrustum& vf = RENDERER.getViewFrustum();
    return vf.getViewMatrix();
  }
  else if (name == "camprj") {
    const ViewFrustum& vf = RENDERER.getViewFrustum();
    return vf.getProjectionMatrix();
  }
  else if (name == "caminv") {
    static float invMatrix[4][4];
    const ViewFrustum& vf = RENDERER.getViewFrustum();
    CalculateInverse4x4((const float (*)[4])vf.getViewMatrix(), invMatrix);
    return (const float*)invMatrix;
  }
  else if (name == "billboard") {
    const ViewFrustum& vf = RENDERER.getViewFrustum();
    return vf.getBillboardMatrix();
  }
  return NULL;
}


int LuaOpenGL::LoadMatrix(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  GLfloat matrix[16];

  const int luaType = lua_type(L, 1);
  if (luaType == LUA_TSTRING) {
    const float* matptr = GetNamedMatrix(lua_tostring(L, 1));
    if (matptr != NULL) {
      glLoadMatrixf(matptr);
    } else {
      luaL_error(L, "Incorrect arguments to gl.LoadMatrix()");
    }
    return 0;
  }
  else if (luaType == LUA_TTABLE) {
    if (ParseFloatArray(L, matrix, 16) != 16) {
      luaL_error(L, "gl.LoadMatrix requires all 16 values");
    }
  }
  else {
    for (int i = 1; i <= 16; i++) {
      matrix[i] = (GLfloat)luaL_checknumber(L, i);
    }
  }
  glLoadMatrixf(matrix);
  return 0;
}


int LuaOpenGL::MultMatrix(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  GLfloat matrix[16];

  const int luaType = lua_type(L, 1);
  if (luaType == LUA_TSTRING) {
    const float* matptr = GetNamedMatrix(lua_tostring(L, 1));
    if (matptr != NULL) {
      glMultMatrixf(matptr);
    } else {
      luaL_error(L, "Incorrect arguments to gl.MultMatrix()");
    }
    return 0;
  }
  else if (luaType == LUA_TTABLE) {
    if (ParseFloatArray(L, matrix, 16) != 16) {
      luaL_error(L, "gl.MultMatrix requires all 16 values");
    }
  }
  else {
    for (int i = 1; i <= 16; i++) {
      matrix[i] = (GLfloat)luaL_checknumber(L, i);
    }
  }
  glMultMatrixf(matrix);
  return 0;
}


int LuaOpenGL::PushMatrix(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if (args != 0) {
    luaL_error(L, "gl.PushMatrix takes no arguments");
  }

  glPushMatrix();

  return 0;
}


int LuaOpenGL::PopMatrix(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const int args = lua_gettop(L);
  if (args != 0) {
    luaL_error(L, "gl.PopMatrix takes no arguments");
  }

  glPopMatrix();

  return 0;
}


int LuaOpenGL::PushPopMatrix(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  vector<GLenum> matModes;
  int funcIndex;
  for (funcIndex = 1; lua_israwnumber(L, funcIndex); funcIndex++) {
    const GLenum mode = (GLenum)lua_toint(L, funcIndex);
    matModes.push_back(mode);
  }

  luaL_checktype(L, funcIndex, LUA_TFUNCTION);

  if (funcIndex == 1) {
    glPushMatrix();
  }
  else {
    for (int i = 0; i < (int)matModes.size(); i++) {
      glMatrixMode(matModes[i]);
      glPushMatrix();
    }
  }

  const int top = lua_gettop(L);
  const int error = lua_pcall(L, top - funcIndex, LUA_MULTRET, 0);

  if (funcIndex == 1) {
    glPopMatrix();
  }
  else {
    for (int i = 0; i < (int)matModes.size(); i++) {
      glMatrixMode(matModes[i]);
      glPopMatrix();
    }
  }

  if (error != 0) {
    LuaLog(1, "gl.PushPopMatrix: error(%i) = %s", error, lua_tostring(L, -1));
    lua_error(L);
  }

  return lua_gettop(L) - (funcIndex - 1);
}


int LuaOpenGL::GetMatrixData(lua_State* L)
{
  const int luaType = lua_type(L, 1);

  if (luaType == LUA_TNUMBER) {
    const GLenum type = (GLenum)lua_tointeger(L, 1);
    GLenum pname = 0; // avoid warnings
    switch (type) {
      case GL_PROJECTION: { pname = GL_PROJECTION_MATRIX; break; }
      case GL_MODELVIEW:  { pname = GL_MODELVIEW_MATRIX;  break; }
      case GL_TEXTURE:    { pname = GL_TEXTURE_MATRIX;    break; }
      default: {
	luaL_error(L, "Incorrect arguments to gl.GetMatrixData(id)");
      }
    }
    GLfloat matrix[16];
    glGetFloatv(pname, matrix);

    if (lua_israwnumber(L, 2)) {
      const int index = lua_toint(L, 2);
      if ((index < 0) || (index >= 16)) {
	return luaL_pushnil(L);
      }
      lua_pushfloat(L, matrix[index]);
      return 1;
    }

    for (int i = 0; i < 16; i++) {
      lua_pushfloat(L, matrix[i]);
    }
    return 16;
  }
  else if (luaType == LUA_TSTRING) {
    const float* matptr = GetNamedMatrix(lua_tostring(L, 1));
    if (matptr != NULL) {
      for (int i = 0; i < 16; i++) {
	lua_pushfloat(L, matptr[i]);
      }
    }
    else {
      luaL_error(L, "Incorrect arguments to gl.GetMatrixData(name)");
    }
    return 16;
  }

  return luaL_pushnil(L);
}


//============================================================================//

int LuaOpenGL::PushAttrib(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  if (OpenGLPassState::CreatingList()) { // FIXME -- remove later
    luaL_error(L, "cannot use gl.PushAttrib() in gl.CreateList()");
  }
  const GLbitfield bits = luaL_optint(L, 1, GL_ALL_ATTRIB_BITS);
  if (!OpenGLPassState::PushAttrib(bits)) {
    luaL_error(L, "attrib stack overflow");
  }
  return 0;
}


int LuaOpenGL::PopAttrib(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  if (OpenGLPassState::CreatingList()) { // FIXME -- remove later
    luaL_error(L, "cannot use gl.PopAttrib() in gl.CreateList()");
  }
  if (!OpenGLPassState::PopAttrib()) {
    luaL_error(L, "attrib stack underflow");
  }
  return 0;
}


int LuaOpenGL::PushPopAttrib(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const GLenum bits = (GLbitfield)luaL_checkint(L, 1);

  const int funcIndex = 2;
  luaL_checktype(L, funcIndex, LUA_TFUNCTION);

  if (!OpenGLPassState::PushAttrib(bits)) {
    luaL_error(L, "attrib stack overflow");
  }

  const int top = lua_gettop(L);
  const int error = lua_pcall(L, top - funcIndex, LUA_MULTRET, 0);

  if (!OpenGLPassState::PopAttrib()) {
    luaL_error(L, "attrib stack overflow");
  }

  if (error != 0) {
    LuaLog(1, "gl.PushPopAttrib: error(%i) = %s", error, lua_tostring(L, -1));
    lua_error(L);
  }

  return lua_gettop(L) - (funcIndex - 1);
}


int LuaOpenGL::UnsafeState(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLenum state = (GLenum)luaL_checkint(L, 1);
  int funcIndex = 2;
  bool reverse = false;
  if (lua_isboolean(L, 2)) {
    funcIndex++;
    reverse = lua_tobool(L, 2);
  }
  luaL_checktype(L, funcIndex, LUA_TFUNCTION);

  bool revert = reverse;
  GLboolean currVal;
  glGetError();
  glGetBooleanv(state, &currVal);
  const GLenum errgl = glGetError();
  if (errgl == GL_INVALID_OPERATION) {
    luaL_error(L, "can not be executed in a glBegin/glEnd pair");
  }
  else if (glGetError() == GL_NO_ERROR) {
    revert = (currVal != GL_FALSE);
  }

  reverse ? glDisable(state) : glEnable(state);
  const int top = lua_gettop(L);
  const int error = lua_pcall(L, top - funcIndex, LUA_MULTRET, 0);
  revert  ? glEnable(state) : glDisable(state);

  if (error != 0) {
    LuaLog(1, "gl.UnsafeState: error(%i) = %s", error, lua_tostring(L, -1));
    lua_error(L);
  }

  return lua_gettop(L) - (funcIndex - 1);
}


//============================================================================//

int LuaOpenGL::Flush(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  glFlush();
  return 0;
}


int LuaOpenGL::Finish(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  glFinish();
  return 0;
}


//============================================================================//
//
//  ReadPixels
//

static int PixelFormatSize(GLenum f)
{
  switch (f) {
    case GL_COLOR_INDEX:
    case GL_STENCIL_INDEX:
    case GL_DEPTH_COMPONENT:
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_LUMINANCE: {
      return 1;
    }
    case GL_LUMINANCE_ALPHA: {
      return 2;
    }
    case GL_RGB:
    case GL_BGR: {
      return 3;
    }
    case GL_RGBA:
    case GL_BGRA: {
      return 4;
    }
  }
  return -1;
}


static void PushPixelData(lua_State* L, int fSize, const float*& data)
{
  if (fSize == 1) {
    lua_pushfloat(L, *data);
    data++;
  }
  else {
    lua_newtable(L);
    for (int e = 1; e <= fSize; e++) {
      lua_pushfloat(L, *data);
      lua_rawseti(L, -2, e);
      data++;
    }
  }
}


int LuaOpenGL::ReadPixels(lua_State* L)
{
  bool rawData = false;
  GLenum type = GL_FLOAT;

  const GLint x = luaL_checkint(L, 1);
  const GLint y = luaL_checkint(L, 2);
  const GLint w = luaL_checkint(L, 3);
  const GLint h = luaL_checkint(L, 4);
  const GLenum format = luaL_optint(L, 5, GL_RGBA);
  if (lua_israwnumber(L, 6)) {
    rawData = true;
    type = lua_toint(L, 6);
  }

  if ((w <= 0) || (h <= 0)) {
    return luaL_pushnil(L);
  }

  const int formatSize = PixelFormatSize(format);
  if (formatSize < 0) {
    luaL_error(L, "unknown format");
  }

  const int pixelSize = LuaTextureMgr::GetPixelSize(format, type);
  if (pixelSize < 0) {
    luaL_error(L, "unknown format/type");
  }

  const size_t dataSize = (w * h * pixelSize);
  char* data = new char[dataSize];

  glReadPixels(x, y, w, h, format, type, data);

  if (rawData) {
    lua_pushlstring(L, data, dataSize);
    delete[] data;
    return 1;
  }

  const float* d = (const float*)data;

  int retCount = 0;

  if ((w == 1) && (h == 1)) {
    for (int e = 0; e < formatSize; e++) {
      lua_pushfloat(L, d[e]);
    }
    retCount = formatSize;
  }
  else if ((w == 1) && (h > 1)) {
    lua_newtable(L);
    for (int i = 1; i <= h; i++) {
      PushPixelData(L, formatSize, d);
      lua_rawseti(L, -2, i);
    }
    retCount = 1;
  }
  else if ((w > 1) && (h == 1)) {
    lua_newtable(L);
    for (int i = 1; i <= w; i++) {
      PushPixelData(L, formatSize, d);
      lua_rawseti(L, -2, i);
    }
    retCount = 1;
  }
  else {
    lua_newtable(L);
    for (int xi = 1; xi <= w; xi++) {
      lua_newtable(L);
      for (int yi = 1; yi <= h; yi++) {
	PushPixelData(L, formatSize, d);
	lua_rawseti(L, -2, yi);
      }
      lua_rawseti(L, -2, xi);
    }
    retCount = 1;
  }

  delete[] data;

  return retCount;
}


//============================================================================//
//============================================================================//

class SelectBuffer {
  public:
    static const GLsizei maxSize = (1 << 24); // float integer range
    static const GLsizei defSize = (256 * 1024);

    SelectBuffer() : size(0), buffer(NULL) {}
    ~SelectBuffer() { delete[] buffer; }

    inline GLuint* GetBuffer() const { return buffer; }

    inline bool ValidIndex(int index) const {
      return ((index >= 0) && (index < size));
    }
    inline bool ValidIndexRange(int index, unsigned int count) const {
      return ((index >= 0) && ((index + (int)count) < size));
    }

    inline GLuint operator[](int index) const {
      return ValidIndex(index) ? buffer[index] : 0;
    }

    inline GLsizei Resize(GLsizei c) {
      c = (c < maxSize) ? c : maxSize;
      if (c != size) {
	delete[] buffer;
	buffer = new GLuint[c];
      }
      size = c;
      return size;
    }

  private:
    GLsizei size;
    GLuint* buffer;
};

static SelectBuffer selectBuffer;


//============================================================================//

int LuaOpenGL::RenderMode(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const GLenum mode = (GLenum)luaL_checkint(L, 1);
  if (!lua_isfunction(L, 2)) {
    luaL_error(L, "Incorrect arguments to gl.RenderMode(mode, func, ...)");
  }


  // call the function
  glRenderMode(mode);
  const int top = lua_gettop(L);
  const int error = lua_pcall(L, top - 2, LUA_MULTRET, 0);
  const GLint oldmode = glRenderMode(GL_RENDER);

  if (error != 0) {
    LuaLog(1, "gl.RenderMode: error(%i) = %s", error, lua_tostring(L, -1));
    lua_error(L);
  }

  lua_pushinteger(L, oldmode);
  lua_replace(L, 1);
  return lua_gettop(L);
}


int LuaOpenGL::SelectBuffer(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  GLsizei selCount = (GLsizei)luaL_optint(L, 1, SelectBuffer::defSize);
  selCount = selectBuffer.Resize(selCount);
  glSelectBuffer(selCount, selectBuffer.GetBuffer());
  lua_pushinteger(L, selCount);
  return 1;
}


int LuaOpenGL::SelectBufferData(lua_State* L)
{
  const int index = luaL_checkint(L, 1);

  if (!lua_israwnumber(L, 2)) {
    if (!selectBuffer.ValidIndex(index)) {
      return luaL_pushnil(L);
    }
    lua_pushinteger(L, selectBuffer[index]);
    return 1;
  }

  const unsigned int count = lua_toint(L, 2);
  if (!selectBuffer.ValidIndexRange(index, count)) {
    return luaL_pushnil(L);
  }
  if (!lua_checkstack(L, count)) {
    luaL_error(L, "could not allocate stack space");
  }
  for (int i = 0; i < (int)count; i++) {
    lua_pushinteger(L, selectBuffer[index + i]);
  }

  return count;
}


int LuaOpenGL::InitNames(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  glInitNames();
  return 0;
}


int LuaOpenGL::LoadName(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLuint name = (GLenum)luaL_checkint(L, 1);
  glLoadName(name);
  return 0;
}


int LuaOpenGL::PushName(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  const GLuint name = (GLuint)luaL_checkint(L, 1);
  glPushName(name);
  return 0;
}


int LuaOpenGL::PopName(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);
  glPopName();
  return 0;
}


int LuaOpenGL::PushPopName(lua_State* L)
{
  CheckDrawingEnabled(L, __FUNCTION__);

  const GLuint name = (GLuint)luaL_checkint(L, 1);
  if (!lua_isfunction(L, 2)) {
    luaL_error(L, "argument 2 should be a function");
  }

  glPushName(name);

  const int top = lua_gettop(L);
  const int error = lua_pcall(L, top - 2, LUA_MULTRET, 0);

  glPopName();

  if (error != 0) {
    LuaLog(1, "gl.PushPopName: error(%i) = %s", error, lua_tostring(L, -1));
    lua_error(L);
  }

  return lua_gettop(L) - 1;
}


//============================================================================//
//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
