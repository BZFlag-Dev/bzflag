//
// TODO:
// - go back to counting matrix push/pops (just for modelview?)
//   would have to make sure that display lists are also handled
//   (GL_MODELVIEW_STACK_DEPTH could help current situation, but
//    requires the ARB_imaging extension)
// - use materials instead of raw calls (again, handle dlists)
//

#include "common.h"

// implementation header
#include "LuaOpenGL.h"

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
#include "OpenGLPassState.h"
#include "StateDatabase.h"
#include "SceneRenderer.h"
#include "BzfDisplay.h"
#include "BzfWindow.h"
#include "FontManager.h"

// bzflag headers
#include "../bzflag/playing.h"
#include "../bzflag/MainWindow.h"

// local headers
#include "LuaInclude.h"
#include "LuaHandle.h"
#include "LuaHashString.h"
#include "LuaShaders.h"
#include "LuaTextures.h"
//FIXME#include "LuaVBOs.h"
#include "LuaFBOs.h"
#include "LuaRBOs.h"
#include "LuaDLists.h"


#undef far // avoid collision with windef.h
#undef near


/******************************************************************************/
/******************************************************************************/


set<unsigned int> LuaOpenGL::occlusionQueries;

bool LuaOpenGL::haveGL20      = false;
bool LuaOpenGL::canUseShaders = false;

/******************************************************************************/
/******************************************************************************/

void LuaOpenGL::Init()
{
	haveGL20 = !!GLEW_VERSION_2_0;

	if (haveGL20 && !BZDB.isTrue("forbidLuaShaders")) {
		canUseShaders = true;
	}

	LuaTextureMgr::Init();
	LuaShaderMgr::Init();
	LuaDListMgr::Init();
}


void LuaOpenGL::Free()
{
	LuaDListMgr::Free();
	LuaShaderMgr::Free();
	LuaTextureMgr::Free();

	if (haveGL20) {
		set<unsigned int>::const_iterator it;
		for (it = occlusionQueries.begin(); it != occlusionQueries.end(); ++it) {
			glDeleteQueries(1, &(*it));
		}
	}
}


/******************************************************************************/
/******************************************************************************/

bool LuaOpenGL::PushEntries(lua_State* L)
{
	PUSH_LUA_CFUNC(L, HasExtension);
	PUSH_LUA_CFUNC(L, GetNumber);
	PUSH_LUA_CFUNC(L, GetString);

	PUSH_LUA_CFUNC(L, ConfigScreen);

	PUSH_LUA_CFUNC(L, ResetState);
	PUSH_LUA_CFUNC(L, ResetMatrices);
	PUSH_LUA_CFUNC(L, Clear);
	PUSH_LUA_CFUNC(L, Lighting);
	PUSH_LUA_CFUNC(L, ShadeModel);
	PUSH_LUA_CFUNC(L, Scissor);
	PUSH_LUA_CFUNC(L, Viewport);
	PUSH_LUA_CFUNC(L, ColorMask);
	PUSH_LUA_CFUNC(L, DepthMask);
	PUSH_LUA_CFUNC(L, DepthTest);
	if (GLEW_NV_depth_clamp) {
		PUSH_LUA_CFUNC(L, DepthClamp);
	}
	PUSH_LUA_CFUNC(L, Culling);
	PUSH_LUA_CFUNC(L, LogicOp);
	PUSH_LUA_CFUNC(L, Fog);
	PUSH_LUA_CFUNC(L, Smoothing);
	PUSH_LUA_CFUNC(L, AlphaTest);
	PUSH_LUA_CFUNC(L, LineStipple);
	PUSH_LUA_CFUNC(L, Blending);
	PUSH_LUA_CFUNC(L, BlendEquation);
	PUSH_LUA_CFUNC(L, BlendFunc);
	if (haveGL20) {
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
	if (haveGL20) {
		PUSH_LUA_CFUNC(L, StencilMaskSeparate);
		PUSH_LUA_CFUNC(L, StencilFuncSeparate);
		PUSH_LUA_CFUNC(L, StencilOpSeparate);
	}

	PUSH_LUA_CFUNC(L, LineWidth);
	PUSH_LUA_CFUNC(L, PointSize);
	if (haveGL20) {
		PUSH_LUA_CFUNC(L, PointSprite);
		PUSH_LUA_CFUNC(L, PointParameter);
	}

	PUSH_LUA_CFUNC(L, ActiveTexture);
	PUSH_LUA_CFUNC(L, TexEnv);
	PUSH_LUA_CFUNC(L, MultiTexEnv);
	PUSH_LUA_CFUNC(L, TexGen);
	PUSH_LUA_CFUNC(L, MultiTexGen);

	PUSH_LUA_CFUNC(L, BeginEnd);
	PUSH_LUA_CFUNC(L, Vertex);
	PUSH_LUA_CFUNC(L, Normal);
	PUSH_LUA_CFUNC(L, TexCoord);
	PUSH_LUA_CFUNC(L, MultiTexCoord);
	PUSH_LUA_CFUNC(L, SecondaryColor);
	PUSH_LUA_CFUNC(L, FogCoord);
	PUSH_LUA_CFUNC(L, EdgeFlag);

	PUSH_LUA_CFUNC(L, Rect);
	PUSH_LUA_CFUNC(L, TexRect);

	PUSH_LUA_CFUNC(L, Text);
	PUSH_LUA_CFUNC(L, GetTextWidth);

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
	PUSH_LUA_CFUNC(L, UnsafeState);

	PUSH_LUA_CFUNC(L, Flush);
	PUSH_LUA_CFUNC(L, Finish);

	PUSH_LUA_CFUNC(L, ReadPixels);
	PUSH_LUA_CFUNC(L, SaveImage);

	if (haveGL20) {
		PUSH_LUA_CFUNC(L, CreateQuery);
		PUSH_LUA_CFUNC(L, DeleteQuery);
		PUSH_LUA_CFUNC(L, RunQuery);
		PUSH_LUA_CFUNC(L, GetQuery);
	}

	PUSH_LUA_CFUNC(L, GetSun);

	LuaDListMgr::PushEntries(L);
	LuaTextureMgr::PushEntries(L);

	if (canUseShaders) {
		LuaShaderMgr::PushEntries(L);
	}

	if (GLEW_EXT_framebuffer_object) {
	 	LuaFBOMgr::PushEntries(L);
	 	LuaRBOMgr::PushEntries(L);
	}

//FIXME		LuaVBOs::PushEntries(L);

	PUSH_LUA_CFUNC(L, RenderMode);
	PUSH_LUA_CFUNC(L, SelectBuffer);
	PUSH_LUA_CFUNC(L, SelectBufferData);
	PUSH_LUA_CFUNC(L, InitNames);
	PUSH_LUA_CFUNC(L, PushName);
	PUSH_LUA_CFUNC(L, PopName);
	PUSH_LUA_CFUNC(L, LoadName);

	return true;
}


/******************************************************************************/
/******************************************************************************/

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


/******************************************************************************/

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
		return 0;
	}
	GLfloat values[64];
	glGetFloatv(pname, values);
	for (GLuint i = 0; i < count; i++) {
		lua_pushnumber(L, values[i]);
	}
	return count;
}


int LuaOpenGL::GetString(lua_State* L)
{
	const GLenum pname = (GLenum)luaL_checknumber(L, 1);
	lua_pushstring(L, (const char*)glGetString(pname));
	return 1;
}


int LuaOpenGL::ConfigScreen(lua_State* L)
{
//	CheckDrawingEnabled(L, __FUNCTION__);
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
	const float size = luaL_optfloat(L, 3, 12.0f);
	const float x    = luaL_optfloat(L, 4, 0.0f);
	const float y    = luaL_optfloat(L, 5, 0.0f);

	FontManager& FM = FontManager::instance();
	const int faceID = FM.getFaceID(face, true);

	bool right = false;
	bool center = false;
	bool outline = false;
	bool colorCodes = true;
	bool lightOut;

	if (lua_israwstring(L, 6)) {
	  const char* c = lua_tostring(L, 6);
	  while (*c != 0) {
	  	switch (*c) {
	  	  case 'c': { center = true;                    break; }
	  	  case 'r': { right = true;                     break; }
			  case 'n': { colorCodes = false;               break; }
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

	glPushMatrix();
	glTranslatef(xj, y, 0.0f);
	FM.drawString(0.0f, 0.0f, 0.0f, faceID, size, text); // FIXME -- setOpacity for fonts
	glPopMatrix();

	return 0;
}


int LuaOpenGL::GetTextWidth(lua_State* L)
{
	const char* text = luaL_checkstring(L, 1);
	const char* face = luaL_checkstring(L, 2);
	const float size = luaL_optfloat(L, 3, 12.0f);

	FontManager& FM = FontManager::instance();
	const int faceID = FM.getFaceID(face);
	lua_pushnumber(L, FM.getStringWidth(faceID, size, text, false));

	return 1;
}


/******************************************************************************/
/******************************************************************************/
//
//  GL evaluators
//

static int evalDepth = 0;


static int GetMap1TargetDataSize(GLenum target)
{
	switch (target) {
		case GL_MAP1_COLOR_4:         { return 4; }
		case GL_MAP1_INDEX:           { return 1; }
		case GL_MAP1_NORMAL:          { return 3; }
		case GL_MAP1_VERTEX_3:        { return 3; }
		case GL_MAP1_VERTEX_4:        { return 4; }
		case GL_MAP1_TEXTURE_COORD_1: { return 1; }
		case GL_MAP1_TEXTURE_COORD_2: { return 2; }
		case GL_MAP1_TEXTURE_COORD_3: { return 3; }
		case GL_MAP1_TEXTURE_COORD_4: { return 4; }
		default:                      { break; }
	}
	return 0;
}

static int GetMap2TargetDataSize(GLenum target)
{
	switch (target) {
		case GL_MAP2_COLOR_4:         { return 4; }
		case GL_MAP2_INDEX:           { return 1; }
		case GL_MAP2_NORMAL:          { return 3; }
		case GL_MAP2_VERTEX_3:        { return 3; }
		case GL_MAP2_VERTEX_4:        { return 4; }
		case GL_MAP2_TEXTURE_COORD_1: { return 1; }
		case GL_MAP2_TEXTURE_COORD_2: { return 2; }
		case GL_MAP2_TEXTURE_COORD_3: { return 3; }
		case GL_MAP2_TEXTURE_COORD_4: { return 4; }
		default:                      { break; }
	}
	return 0;
}



int LuaOpenGL::Eval(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);
	if (!lua_isfunction(L, 1)) {
		return 0;
	}

	if (evalDepth == 0) { glPushAttrib(GL_EVAL_BIT); }
	evalDepth++;
	const int error = lua_pcall(L, lua_gettop(L) - 1, 0, 0);
	evalDepth--;
	if (evalDepth == 0) { glPopAttrib(); }

	if (error != 0) {
		LuaLog("gl.Eval: error(%i) = %s", error, lua_tostring(L, -1));
		lua_error(L);
	}
	return 0;
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


/******************************************************************************/
/******************************************************************************/

int LuaOpenGL::BeginEnd(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	const int args = lua_gettop(L); // number of arguments
	if ((args < 2) || !lua_israwnumber(L, 1) || !lua_isfunction(L, 2)) {
		luaL_error(L, "Incorrect arguments to gl.BeginEnd(type, func, ...)");
	}
	const GLuint primMode = (GLuint)lua_tonumber(L, 1);

	// call the function
	glBegin(primMode);
	const int error = lua_pcall(L, (args - 2), 0, 0);
	glEnd();

	if (error != 0) {
		LuaLog("gl.BeginEnd: error(%i) = %s", error, lua_tostring(L, -1));
		lua_error(L);
	}
	return 0;
}


/******************************************************************************/

int LuaOpenGL::Vertex(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	const int args = lua_gettop(L); // number of arguments

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

	const int args = lua_gettop(L); // number of arguments

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

	const int args = lua_gettop(L); // number of arguments

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
	if ((texNum < 0) || (texNum >= MAX_TEXTURE_UNITS)) {
		luaL_error(L, "Bad texture unit passed to gl.MultiTexCoord()");
	}
	const GLenum texUnit = GL_TEXTURE0 + texNum;

	const int args = lua_gettop(L) - 1; // number of arguments

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

	const int args = lua_gettop(L); // number of arguments

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


/******************************************************************************/

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


/******************************************************************************/

static bool ParseColor(lua_State* L, float color[4], const char* funcName)
{
	const int args = lua_gettop(L); // number of arguments
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

	float color[4];
	ParseColor(L, color, __FUNCTION__);
	glColor4fv(color);

	return 0;
}


int LuaOpenGL::Material(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	const int args = lua_gettop(L); // number of arguments
	if ((args != 1) || !lua_istable(L, 1)) {
		luaL_error(L, "Incorrect arguments to gl.Material(table)");
	}

	float color[4];

	const int table = lua_gettop(L);
	for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
		if (!lua_israwstring(L, -2)) { // the key
			LuaLog("gl.Material: bad state type");
			return 0;;
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
			LuaLog("gl.Material: unknown material type: %s", key.c_str());
		}
	}
	return 0;
}


int LuaOpenGL::Ambient(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	float color[4];
	ParseColor(L, color, __FUNCTION__);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);

	return 0;
}


int LuaOpenGL::Diffuse(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	float color[4];
	ParseColor(L, color, __FUNCTION__);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);

	return 0;
}


int LuaOpenGL::Emission(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	float color[4];
	ParseColor(L, color, __FUNCTION__);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, color);

	return 0;
}


int LuaOpenGL::Specular(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	float color[4];
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


/******************************************************************************/

int LuaOpenGL::ResetState(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	const int args = lua_gettop(L); // number of arguments
	if (args != 0) {
		luaL_error(L, "gl.ResetState takes no arguments");
	}

	OpenGLPassState::ResetState();

	return 0;
}


int LuaOpenGL::ResetMatrices(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	const int args = lua_gettop(L); // number of arguments
	if (args != 0) {
		luaL_error(L, "gl.ResetMatrices takes no arguments");
	}

	OpenGLPassState::ResetMatrices();

	return 0;
}


int LuaOpenGL::Lighting(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	const int args = lua_gettop(L); // number of arguments
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


int LuaOpenGL::ShadeModel(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	const int args = lua_gettop(L); // number of arguments
	if ((args != 1) || !lua_israwnumber(L, 1)) {
		luaL_error(L, "Incorrect arguments to gl.ShadeModel()");
	}

	glShadeModel((GLenum)lua_tonumber(L, 1));

	return 0;
}


int LuaOpenGL::Scissor(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	const int args = lua_gettop(L); // number of arguments
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
//FIXME		glScissor(x + gu->viewPosX, y + gu->viewPosY, w, h);
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

	const int args = lua_gettop(L); // number of arguments
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

	const int args = lua_gettop(L); // number of arguments
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

	const int args = lua_gettop(L); // number of arguments
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
		glDepthFunc((GLenum)lua_tonumber(L, 1));
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

	const int args = lua_gettop(L); // number of arguments
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
		glCullFace((GLenum)lua_tonumber(L, 1));
	}
	else {
		luaL_error(L, "Incorrect arguments to gl.Culling()");
	}
	return 0;
}


int LuaOpenGL::LogicOp(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	const int args = lua_gettop(L); // number of arguments
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
		glLogicOp((GLenum)lua_tonumber(L, 1));
	}
	else {
		luaL_error(L, "Incorrect arguments to gl.LogicOp()");
	}
	return 0;
}


int LuaOpenGL::Fog(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	const int args = lua_gettop(L); // number of arguments
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

	const int args = lua_gettop(L); // number of arguments
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
			const GLenum hint = (GLenum)lua_tonumber(L, luaIndex);
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

	const int args = lua_gettop(L); // number of arguments
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
		glAlphaFunc((GLenum)lua_tonumber(L, 1), (GLfloat)lua_tonumber(L, 2));
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

	const int args = lua_gettop(L); // number of arguments
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


/******************************************************************************/

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


/******************************************************************************/

int LuaOpenGL::LineStipple(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

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
	const int args = lua_gettop(L); // number of arguments
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


int LuaOpenGL::ActiveTexture(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	const int args = lua_gettop(L); // number of arguments
	if ((args < 2) || !lua_israwnumber(L, 1) || !lua_isfunction(L, 2)) {
		luaL_error(L, "Incorrect arguments to gl.ActiveTexture(number, func, ...)");
	}
	const int texNum = lua_toint(L, 1);
	if ((texNum < 0) || (texNum >= MAX_TEXTURE_UNITS)) {
		luaL_error(L, "Bad texture unit passed to gl.ActiveTexture()");
		return 0;
	}

	// call the function
	glActiveTexture(GL_TEXTURE0 + texNum);
	const int error = lua_pcall(L, (args - 2), 0, 0);
	glActiveTexture(GL_TEXTURE0);

	if (error != 0) {
		LuaLog("gl.ActiveTexture: error(%i) = %s", error, lua_tostring(L, -1));
		lua_error(L);
	}
	return 0;
}


int LuaOpenGL::TexEnv(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	const GLenum target = (GLenum)luaL_checknumber(L, 1);
	const GLenum pname  = (GLenum)luaL_checknumber(L, 2);

	const int args = lua_gettop(L); // number of arguments
	if (args == 3) {
		const GLfloat value = (GLfloat)luaL_checknumber(L, 3);
		glTexEnvf(target, pname, value);
	}
	else if (args == 6) {
		GLfloat array[4];
		array[0] = luaL_optnumber(L, 3, 0.0f);
		array[1] = luaL_optnumber(L, 4, 0.0f);
		array[2] = luaL_optnumber(L, 5, 0.0f);
		array[3] = luaL_optnumber(L, 6, 0.0f);
		glTexEnvfv(target, pname, array);
	}
	else {
		luaL_error(L, "Incorrect arguments to gl.TexEnv()");
	}

	return 0;
}


int LuaOpenGL::MultiTexEnv(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);
	const int texNum    =    luaL_checkint(L, 1);
	const GLenum target = (GLenum)luaL_checknumber(L, 2);
	const GLenum pname  = (GLenum)luaL_checknumber(L, 3);

	if ((texNum < 0) || (texNum >= MAX_TEXTURE_UNITS)) {
		luaL_error(L, "Bad texture unit passed to gl.MultiTexEnv()");
	}

	const int args = lua_gettop(L); // number of arguments
	if (args == 4) {
		const GLfloat value = (GLfloat)luaL_checknumber(L, 4);
		glActiveTexture(GL_TEXTURE0 + texNum);
		glTexEnvf(target, pname, value);
		glActiveTexture(GL_TEXTURE0);
	}
	else if (args == 7) {
		GLfloat array[4];
		array[0] = luaL_optnumber(L, 4, 0.0f);
		array[1] = luaL_optnumber(L, 5, 0.0f);
		array[2] = luaL_optnumber(L, 6, 0.0f);
		array[3] = luaL_optnumber(L, 7, 0.0f);
		glActiveTexture(GL_TEXTURE0 + texNum);
		glTexEnvfv(target, pname, array);
		glActiveTexture(GL_TEXTURE0);
	}
	else {
		luaL_error(L, "Incorrect arguments to gl.MultiTexEnv()");
	}

	return 0;
}


static void SetTexGenState(GLenum target, bool state)
{
	if ((target >= GL_S) && (target <= GL_Q)) {
		const GLenum pname = GL_TEXTURE_GEN_S + (target - GL_S);
		if (state) {
			glEnable(pname);
		} else {
			glDisable(pname);
		}
	}
}


int LuaOpenGL::TexGen(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	const GLenum target = (GLenum)luaL_checknumber(L, 1);

	const int args = lua_gettop(L); // number of arguments
	if ((args == 2) && lua_isboolean(L, 2)) {
		const bool state = lua_tobool(L, 2);
		SetTexGenState(target, state);
		return 0;
	}

	const GLenum pname  = (GLenum)luaL_checknumber(L, 2);

	if (args == 3) {
		const GLfloat value = (GLfloat)luaL_checknumber(L, 3);
		glTexGenf(target, pname, value);
		SetTexGenState(target, true);
	}
	else if (args == 6) {
		GLfloat array[4];
		array[0] = luaL_optnumber(L, 3, 0.0f);
		array[1] = luaL_optnumber(L, 4, 0.0f);
		array[2] = luaL_optnumber(L, 5, 0.0f);
		array[3] = luaL_optnumber(L, 6, 0.0f);
		glTexGenfv(target, pname, array);
		SetTexGenState(target, true);
	}
	else {
		luaL_error(L, "Incorrect arguments to gl.TexGen()");
	}

	return 0;
}


int LuaOpenGL::MultiTexGen(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	const int texNum = luaL_checkint(L, 1);
	if ((texNum < 0) || (texNum >= MAX_TEXTURE_UNITS)) {
		luaL_error(L, "Bad texture unit passed to gl.MultiTexGen()");
	}

	const GLenum target = (GLenum)luaL_checknumber(L, 2);

	const int args = lua_gettop(L); // number of arguments
	if ((args == 3) && lua_isboolean(L, 3)) {
		const bool state = lua_tobool(L, 3);
		glActiveTexture(GL_TEXTURE0 + texNum);
		SetTexGenState(target, state);
		glActiveTexture(GL_TEXTURE0);
		return 0;
	}

	const GLenum pname  = (GLenum)luaL_checknumber(L, 3);

	if (args == 4) {
		const GLfloat value = (GLfloat)luaL_checknumber(L, 4);
		glActiveTexture(GL_TEXTURE0 + texNum);
		glTexGenf(target, pname, value);
		SetTexGenState(target, true);
		glActiveTexture(GL_TEXTURE0);
	}
	else if (args == 7) {
		GLfloat array[4];
		array[0] = luaL_optnumber(L, 4, 0.0f);
		array[1] = luaL_optnumber(L, 5, 0.0f);
		array[2] = luaL_optnumber(L, 6, 0.0f);
		array[3] = luaL_optnumber(L, 7, 0.0f);
		glActiveTexture(GL_TEXTURE0 + texNum);
		glTexGenfv(target, pname, array);
		SetTexGenState(target, true);
		glActiveTexture(GL_TEXTURE0);
	}
	else {
		luaL_error(L, "Incorrect arguments to gl.MultiTexGen()");
	}

	return 0;
}


/******************************************************************************/

int LuaOpenGL::Clear(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	const int args = lua_gettop(L); // number of arguments
	if ((args < 1) || !lua_israwnumber(L, 1)) {
		luaL_error(L, "Incorrect arguments to gl.Clear()");
	}

	const GLbitfield bits = (GLbitfield)lua_tonumber(L, 1);
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
			glClearStencil((GLint)lua_tonumber(L, 2));
		}
	}

	glClear(bits);

	return 0;
}


/******************************************************************************/

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


/******************************************************************************/

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

	const int args = lua_gettop(L); // number of arguments
	if (args == 3) {
		const GLenum  pname = (GLenum)luaL_checknumber(L, 2);
		const GLfloat param = (GLenum)luaL_checknumber(L, 3);
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


/******************************************************************************/

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
/* FIXME -- GetNamedMatrix()
	if (name == "shadow") {
		static double mat[16];
		for (int i =0; i <16; i++) {
			mat[i] = shadowHandler->shadowMatrix.m[i];
		}
		return mat;
	}
	else if (name == "caminv") {
		return camera->modelviewInverse;
	}
	else if (name == "billboard") {
		return camera->GetBillboard();
	}
*/
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

	const int args = lua_gettop(L); // number of arguments
	if (args != 0) {
		luaL_error(L, "gl.PushMatrix takes no arguments");
	}

	glPushMatrix();

	return 0;
}


int LuaOpenGL::PopMatrix(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	const int args = lua_gettop(L); // number of arguments
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
	int arg;
	for (arg = 1; lua_israwnumber(L, arg); arg++) {
		const GLenum mode = (GLenum)lua_toint(L, arg);
		matModes.push_back(mode);
	}

	if (!lua_isfunction(L, arg)) {
		luaL_error(L, "Incorrect arguments to gl.PushPopMatrix()");
	}

	if (arg == 1) {
		glPushMatrix();
	} else {
		for (int i = 0; i < (int)matModes.size(); i++) {
			glMatrixMode(matModes[i]);
			glPushMatrix();
		}
	}

	const int args = lua_gettop(L); // number of arguments
	const int error = lua_pcall(L, (args - arg), 0, 0);

	if (arg == 1) {
		glPopMatrix();
	} else {
		for (int i = 0; i < (int)matModes.size(); i++) {
			glMatrixMode(matModes[i]);
			glPopMatrix();
		}
	}

	if (error != 0) {
		LuaLog("gl.PushPopMatrix: error(%i) = %s", error, lua_tostring(L, -1));
		lua_error(L);
	}

	return 0;
}


int LuaOpenGL::GetMatrixData(lua_State* L)
{
	const int luaType = lua_type(L, 1);

	if (luaType == LUA_TNUMBER) {
		const GLenum type = (GLenum)lua_tonumber(L, 1);
		GLenum pname;
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
				return 0;
			}
			lua_pushnumber(L, matrix[index]);
			return 1;
		}

		for (int i = 0; i < 16; i++) {
			lua_pushnumber(L, matrix[i]);
		}

		return 16;
	}
	else if (luaType == LUA_TSTRING) {
		const float* matptr = GetNamedMatrix(lua_tostring(L, 1));
		if (matptr != NULL) {
			for (int i = 0; i < 16; i++) {
				lua_pushnumber(L, matptr[i]);
			}
		}
		else {
			luaL_error(L, "Incorrect arguments to gl.GetMatrixData(name)");
		}
		return 16;
	}

	return 0;
}

/******************************************************************************/

int LuaOpenGL::PushAttrib(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);
	glPushAttrib((GLbitfield)luaL_optnumber(L, 1, GL_ALL_ATTRIB_BITS));
	return 0;
}


int LuaOpenGL::PopAttrib(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);
	glPopAttrib();
	return 0;
}


int LuaOpenGL::UnsafeState(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);
	const GLenum state = (GLenum)luaL_checkint(L, 1);
	int funcLoc = 2;
	bool reverse = false;
	if (lua_isboolean(L, 2)) {
		funcLoc++;
		reverse = lua_tobool(L, 2);
	}
	if (!lua_isfunction(L, funcLoc)) {
		luaL_error(L, "expecting a function");
	}

	reverse ? glDisable(state) : glEnable(state);
	const int error = lua_pcall(L, lua_gettop(L) - funcLoc, 0, 0);
	reverse ? glEnable(state) : glDisable(state);

	if (error != 0) {
		LuaLog("gl.UnsafeState: error(%i) = %s", error, lua_tostring(L, -1));
		lua_pushnumber(L, 0);
	}
	return 0;
}


/******************************************************************************/

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


/******************************************************************************/

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
		lua_pushnumber(L, *data);
		data++;
	} else {
		lua_newtable(L);
		for (int e = 1; e <= fSize; e++) {
			lua_pushinteger(L, e);
			lua_pushnumber(L, *data);
			lua_rawset(L, -3);
			data++;
		}
	}
}


int LuaOpenGL::ReadPixels(lua_State* L)
{
	const GLint x = luaL_checkint(L, 1);
	const GLint y = luaL_checkint(L, 2);
	const GLint w = luaL_checkint(L, 3);
	const GLint h = luaL_checkint(L, 4);
	const GLenum format = luaL_optint(L, 5, GL_RGBA);
	if ((w <= 0) || (h <= 0)) {
		return 0;
	}

	int fSize = PixelFormatSize(format);
	if (fSize < 0) {
		fSize = 4; // good enough?
	}

	float* data = new float[(h * w) * fSize * sizeof(float)];
	glReadPixels(x, y, w, h, format, GL_FLOAT, data);

	int retCount = 0;

	const float* d = data;

	if ((w == 1) && (h == 1)) {
		for (int e = 0; e < fSize; e++) {
			lua_pushnumber(L, data[e]);
		}
		retCount = fSize;
	}
	else if ((w == 1) && (h > 1)) {
		lua_newtable(L);
		for (int i = 1; i <= h; i++) {
			lua_pushinteger(L, i);
			PushPixelData(L, fSize, d);
			lua_rawset(L, -3);
		}
		retCount = 1;
	}
	else if ((w > 1) && (h == 1)) {
		lua_newtable(L);
		for (int i = 1; i <= w; i++) {
			lua_pushinteger(L, i);
			PushPixelData(L, fSize, d);
			lua_rawset(L, -3);
		}
		retCount = 1;
	}
	else {
		lua_newtable(L);
		for (int xi = 1; xi <= w; xi++) {
			lua_pushinteger(L, xi);
			lua_newtable(L);
			for (int yi = 1; yi <= h; yi++) {
				lua_pushinteger(L, yi);
				PushPixelData(L, fSize, d);
				lua_rawset(L, -3);
			}
			lua_rawset(L, -3);
		}
		retCount = 1;
	}

	delete[] data;

	return retCount;
}


int LuaOpenGL::SaveImage(lua_State* L) // FIXME
{
	L = L;
	/*
	const string filename = luaL_checkstring(L, 1);

	const int x0 = luaL_checkint(L, 2) + gu->viewPosX;
	const int y0 = luaL_checkint(L, 3) + gu->viewPosY;
	const int x1 = luaL_checkint(L, 4) + gu->viewPosX;
	const int y1 = luaL_checkint(L, 5) + gu->viewPosY;

	bool alpha = false;
	bool yflip = false;
	const int table = 6;
	if (lua_istable(L, table)) {
		lua_getfield(L, table, "alpha");
		if (lua_isboolean(L, -1)) {
			alpha = lua_tobool(L, -1);
		}
		lua_pop(L, 1);
		lua_getfield(L, table, "yflip");
		if (lua_isboolean(L, -1)) {
			yflip = lua_tobool(L, -1);
		}
		lua_pop(L, 1);
	}

	const int xsize = (x1 - x0) + 1;
	const int ysize = (y1 - y0) + 1;
	if ((xsize <= 0) || (ysize <= 0)) {
		return 0;
	}
	const int memsize = xsize * ysize * 4;	

	unsigned char* img = new unsigned char[memsize];
	memset(img, 0, memsize);
	glReadPixels(x0, y0, xsize, ysize, GL_RGBA, GL_UNSIGNED_BYTE, img);

	CBitmap bitmap(img, xsize, ysize);
	if (!yflip) {
		bitmap.ReverseYAxis();
	}


	// FIXME Check file path permission here

	lua_pushboolean(L, bitmap.Save(filename, !alpha));
	delete[] img;
*/

	return 0;
}


/******************************************************************************/

int LuaOpenGL::CreateQuery(lua_State* L)
{
	GLuint q;
	glGenQueries(1, &q);
	if (q == 0) {
		return 0;
	}
	occlusionQueries.insert(q);
	lua_pushlightuserdata(L, (void*)q);
	return 1;
}


int LuaOpenGL::DeleteQuery(lua_State* L)
{
	if (lua_isnil(L, 1)) {
		return 0;
	}
	if (!lua_islightuserdata(L, 1)) {
		luaL_error(L, "invalid argument");
	}
	GLuint q = (unsigned long int)lua_topointer(L, 1);
	if (occlusionQueries.find(q) != occlusionQueries.end()) {
		occlusionQueries.erase(q);
		glDeleteQueries(1, &q);
	}
	return 0;
}


int LuaOpenGL::RunQuery(lua_State* L)
{
	static bool running = false;

	if (running) {
		luaL_error(L, "not re-entrant");
	}
	if (!lua_islightuserdata(L, 1)) {
		luaL_error(L, "expecting a query");
	}
	GLuint q = (unsigned long int)lua_topointer(L, 1);
	if (occlusionQueries.find(q) == occlusionQueries.end()) {
		return 0;
	}
	if (!lua_isfunction(L, 2)) {
		luaL_error(L, "expecting a function");
	}
	const int args = lua_gettop(L); // number of arguments

	running = true;
	glBeginQuery(GL_SAMPLES_PASSED, q);
	const int error = lua_pcall(L, (args - 2), 0, 0);
	glEndQuery(GL_SAMPLES_PASSED);
	running = false;

	if (error != 0) {
		LuaLog("gl.RunQuery: error(%i) = %s", error, lua_tostring(L, -1));
		lua_error(L);
	}

	return 0;
}


int LuaOpenGL::GetQuery(lua_State* L)
{
	if (!lua_islightuserdata(L, 1)) {
		luaL_error(L, "invalid argument");
	}
	GLuint q = (unsigned long int)lua_topointer(L, 1);
	if (occlusionQueries.find(q) == occlusionQueries.end()) {
		return 0;
	}

	GLuint count;
	glGetQueryObjectuiv(q, GL_QUERY_RESULT, &count);

	lua_pushinteger(L, count);

	return 1;
}


/******************************************************************************/

int LuaOpenGL::GetSun(lua_State* L)
{
	const string param = luaL_checkstring(L, 1);

	const float* data = NULL;

	if (param == "brightness") {
		lua_pushnumber(L, RENDERER.getSunBrightness());
		return 1;
	}
	else if (param == "dir") {
		data = RENDERER.getSunDirection();
	}
	else if (param == "ambient") {
		data = RENDERER.getAmbientColor();
	}
	else if (param == "diffuse") {
		data = RENDERER.getSunColor();
	}
	else if (param == "specular") {
		data = RENDERER.getSunColor();
	}

	if (data != NULL) {
		lua_pushnumber(L, data[0]);
		lua_pushnumber(L, data[1]);
		lua_pushnumber(L, data[2]);
		return 3;
	}

	return 0;
}


/******************************************************************************/
/******************************************************************************/

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


/******************************************************************************/

int LuaOpenGL::RenderMode(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);

	const GLenum mode = (GLenum)luaL_checkint(L, 1);
	if (!lua_isfunction(L, 2)) {
		luaL_error(L, "Incorrect arguments to gl.RenderMode(mode, func, ...)");
	}

	const int args = lua_gettop(L); // number of arguments

	// call the function
	glRenderMode(mode);
	const int error = lua_pcall(L, (args - 2), 0, 0);
	const GLint count2 = glRenderMode(GL_RENDER);

	if (error != 0) {
		LuaLog("gl.RenderMode: error(%i) = %s", error, lua_tostring(L, -1));
		lua_error(L);
	}

	lua_pushinteger(L, count2);
	return 1;
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
			return 0;
		}
		lua_pushinteger(L, selectBuffer[index]);
		return 1;
	}

	const unsigned int count = lua_toint(L, 2);
	if (!selectBuffer.ValidIndexRange(index, count)) {
		return 0;
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
	const GLuint name = (GLenum)luaL_checkint(L, 1);
	glPushName(name);
	return 0;
}


int LuaOpenGL::PopName(lua_State* L)
{
	CheckDrawingEnabled(L, __FUNCTION__);
	glPopName();
	return 0;
}


/******************************************************************************/
/******************************************************************************/
