
#include "common.h"

// interface header
#include "LuaGLPointers.h"

// system headers
#include <string.h>
#include <string>
#include <vector>
#include <map>
using std::string;
using std::vector;
using std::map;

// common headers
#include "bzfgl.h"
#include "OpenGLPassState.h"

// local headers
#include "LuaInclude.h"
#include "LuaOpenGL.h"
#include "LuaGLBuffers.h"
#include "LuaTextures.h"


static int minElements = 0; // the minimum element count of all enabled pointers
static map<GLenum, int> maxElements; // max element counts for enabled pointers

static map<GLenum, int> luaRefs; // refs to strings to avoid garbage collection
                                 // (could also use lua_gc(LUA_GCSTOP) if this
                                 // was being implemented as a wrapped function)

static lua_State* activeLua = NULL;


/******************************************************************************/
/******************************************************************************/

bool LuaGLPointers::PushEntries(lua_State* L)
{
	PUSH_LUA_CFUNC(L, VertexPointer);
	PUSH_LUA_CFUNC(L, NormalPointer);
	PUSH_LUA_CFUNC(L, TexCoordPointer);
	PUSH_LUA_CFUNC(L, MultiTexCoordPointer);
	PUSH_LUA_CFUNC(L, ColorPointer);
	PUSH_LUA_CFUNC(L, EdgeFlagPointer);

	PUSH_LUA_CFUNC(L, ArrayElement);
	PUSH_LUA_CFUNC(L, DrawArrays);
	PUSH_LUA_CFUNC(L, DrawElements);
	PUSH_LUA_CFUNC(L, DrawRangeElements);
	if (GLEW_ARB_draw_instanced) {
		PUSH_LUA_CFUNC(L, DrawArraysInstanced);
		PUSH_LUA_CFUNC(L, DrawElementsInstanced);
	}

	PUSH_LUA_CFUNC(L, GetPointerState);

	return true;
}


/******************************************************************************/
/******************************************************************************/

void LuaGLPointers::CheckActiveState(lua_State* L, const char* funcName)
{
	LuaOpenGL::CheckDrawingEnabled(L, funcName);
	if (L != activeLua) {
		luaL_error(L, "invalid lua state");
	}
}


/******************************************************************************/
/******************************************************************************/

bool LuaGLPointers::Enable(lua_State* L)
{
	if (activeLua != NULL) {
		return false;
	}

	activeLua = L;

	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	// NOTE: BZFlag does not use the following
	// glDisableClientState(GL_COLOR_ARRAY);
	// glDisableClientState(GL_EDGE_FLAG_ARRAY);

	return true;
}


bool LuaGLPointers::Reset(lua_State* L)
{
	if (L != activeLua) {
		return false;
	}

	glPopClientAttrib();

	map<GLenum, int>::const_iterator it;
	for (it = luaRefs.begin(); it != luaRefs.end(); ++it) {
		luaL_unref(L, LUA_REGISTRYINDEX, it->second);
	}
	luaRefs.clear();

	maxElements.clear();
	minElements = 0;

	activeLua = NULL;

	return true;
}


/******************************************************************************/
/******************************************************************************/

static void UpdateMinElement(int maxElem)
{
	if (maxElements.empty()) {
		minElements = 0;
		return;
	}
	if (minElements > maxElem) {
		minElements = maxElem;
		return;
	}
	minElements = 1234567890;
	map<GLenum, int>::const_iterator it;
	for (it = maxElements.begin(); it != maxElements.end(); ++it) {
		if (minElements > it->second) {
			minElements = it->second;
		}
	}
}


/******************************************************************************/
/******************************************************************************/

static bool FreeLuaRef(lua_State* L, GLenum type)
{
	map<GLenum, int>::iterator it = luaRefs.find(type);
	if (it != luaRefs.end()) {
		luaL_unref(L, LUA_REGISTRYINDEX, it->second);
		luaRefs.erase(it);
		return true;
	}
	return false;
}


static bool CreateLuaRef(lua_State* L, int index, GLenum type)
{
	FreeLuaRef(L, type);
	lua_checkstack(L, 1);
	lua_pushvalue(L, index);
	const int ref = luaL_ref(L, LUA_REGISTRYINDEX);
	luaRefs[type] = ref;
	return true;
}


/******************************************************************************/
/******************************************************************************/

static const void* ParsePointer(lua_State* L, int index, GLsizei count,
                                GLenum* dataType, int& maxElem, bool& useVBO,
                                GLenum arrayType)
{
	const char* data = NULL;
	size_t size;
	size_t offset = 0;
	GLuint bufID = 0;

	if (dataType != NULL) {
		*dataType = (GLenum)luaL_checkint(L, index);
		index++;
	}

	if (lua_israwstring(L, index)) {
		data = lua_tolstring(L, index, &size);
		offset = luaL_optint(L, index + 1, 0) ;
	}
	else {
		const LuaGLBuffer* buf = LuaGLBufferMgr::TestLuaGLBuffer(L, index);
		if (buf == NULL) {
			luaL_error(L, "invalid data source");
		}
		offset = luaL_optint(L, index + 1, 0);

		size = buf->size;
		bufID = buf->id;

		if (bufID == 0) {
			maxElem = 0;
			return NULL;
		}
	}

	if (offset > size) {
		luaL_error(L, "offset exceeds data size");
	}
	data += offset;
	size -= offset;

	const int typeSize = LuaGLBufferMgr::GetTypeSize(*dataType);
	if (typeSize <= 0) {
		maxElem = 0;
	}
	maxElem = size / (count * typeSize);
	if (maxElem <= 0) {
		return NULL;
	}

	if (bufID == 0) {
		useVBO = false;
	} else {
		useVBO = true;
		glBindBuffer(GL_ARRAY_BUFFER, bufID);
	}

	CreateLuaRef(L, index, arrayType);
	maxElements[arrayType] = maxElem;
	UpdateMinElement(maxElem);

	return (const void*)data;
}


/******************************************************************************/
/******************************************************************************/

int LuaGLPointers::VertexPointer(lua_State* L)
{
	CheckActiveState(L, __FUNCTION__);

	if (lua_isboolean(L, 1) && !lua_tobool(L, 1)) {
	  glDisableClientState(GL_VERTEX_ARRAY);
		maxElements.erase(GL_VERTEX_ARRAY);
		FreeLuaRef(L, GL_VERTEX_ARRAY);		
		return 0;
	}

	const GLsizei stride = (GLenum)luaL_checkint(L, 1);
	const GLint   width  = (GLenum)luaL_checkint(L, 2);
	if ((width < 3) || (width > 4)) {
		return 0;
	}

	GLenum type;
	int maxElem;
	bool useVBO;
	const GLvoid* ptr = ParsePointer(L, 3, width, &type, maxElem, useVBO,
	                                 GL_VERTEX_ARRAY);
	if (maxElem <= 0) {
		return 0;
	}
		
	glVertexPointer(width, type, stride, ptr);
	glEnableClientState(GL_VERTEX_ARRAY);
	if (useVBO) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	return 0;
}


int LuaGLPointers::NormalPointer(lua_State* L)
{
	CheckActiveState(L, __FUNCTION__);

	if (lua_isboolean(L, 1) && !lua_tobool(L, 1)) {
	  glDisableClientState(GL_NORMAL_ARRAY);
		maxElements.erase(GL_NORMAL_ARRAY);
		FreeLuaRef(L, GL_NORMAL_ARRAY);		
		return 0;
	}

	const GLsizei stride = (GLenum)luaL_checkint(L, 1);

	GLenum type;
	int maxElem;
	bool useVBO;
	const GLvoid* ptr = ParsePointer(L, 2, 3, &type, maxElem, useVBO,
	                                 GL_NORMAL_ARRAY);
	if (maxElem <= 0) {
		return 0;
	}

	glNormalPointer(type, stride, ptr);
	glEnableClientState(GL_NORMAL_ARRAY);
	if (useVBO) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	return 0;
}


static int HandleTexCoordPointer(lua_State* L, int index, GLenum texUnit)
{
	if (lua_isboolean(L, index) && !lua_tobool(L, index)) {
	  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		maxElements.erase(texUnit);
		FreeLuaRef(L, texUnit);
		return 0;
	}

	const GLsizei stride = (GLenum)luaL_checkint(L, index);
	const GLint   width  = (GLenum)luaL_checkint(L, index + 1);
	if ((width < 1) || (width > 4)) {
		return 0;
	}

	GLenum type;
	int maxElem;
	bool useVBO;
	const GLvoid* ptr = ParsePointer(L, index + 2, width, &type, maxElem, useVBO,
	                                 texUnit);
	if (maxElem <= 0) {
		return 0;
	}
		
	glTexCoordPointer(width, type, stride, ptr);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if (useVBO) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	return 0;
}


int LuaGLPointers::TexCoordPointer(lua_State* L)
{
	CheckActiveState(L, __FUNCTION__);

	const GLenum texUnit = LuaTextureMgr::GetActiveTexture();
	return HandleTexCoordPointer(L, 1, texUnit);
}


int LuaGLPointers::MultiTexCoordPointer(lua_State* L)
{
	CheckActiveState(L, __FUNCTION__);

	const int texNum = luaL_checkint(L, 1);
	if ((texNum < 0) || (texNum >= MAX_LUA_TEXTURE_UNITS)) {
		luaL_error(L, "Bad texture unit passed to gl.MultiTexCoord()");
	}
	const GLenum texUnit = GL_TEXTURE0 + texNum;

	glClientActiveTexture(texUnit);
	const int retval = HandleTexCoordPointer(L, 2, texUnit);
	glClientActiveTexture(GL_TEXTURE0);

	return retval;
}


int LuaGLPointers::ColorPointer(lua_State* L)
{
	CheckActiveState(L, __FUNCTION__);

	if (lua_isboolean(L, 1) && !lua_tobool(L, 1)) {
	  glDisableClientState(GL_COLOR_ARRAY);
		maxElements.erase(GL_COLOR_ARRAY);
		FreeLuaRef(L, GL_COLOR_ARRAY);
		return 0;
	}

	const GLsizei stride = (GLenum)luaL_checkint(L, 1);
	const GLint   width  = (GLenum)luaL_checkint(L, 2);
	if ((width < 3) || (width > 4)) {
		return 0;
	}

	GLenum type;
	int maxElem;
	bool useVBO;
	const GLvoid* ptr = ParsePointer(L, 4, width, &type, maxElem, useVBO,
	                                 GL_COLOR_ARRAY);
	if (maxElem <= 0) {
		return 0;
	}
		
	glColorPointer(width, type, stride, ptr);
	glEnableClientState(GL_COLOR_ARRAY);
	if (useVBO) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	return 0;
}


int LuaGLPointers::EdgeFlagPointer(lua_State* L)
{
	CheckActiveState(L, __FUNCTION__);

	if (lua_isboolean(L, 1) && !lua_tobool(L, 1)) {
	  glDisableClientState(GL_EDGE_FLAG_ARRAY);
		maxElements.erase(GL_EDGE_FLAG_ARRAY);
		FreeLuaRef(L, GL_EDGE_FLAG_ARRAY);
		return 0;
	}

	const GLsizei stride = (GLenum)luaL_checkint(L, 1);

	int maxElem;
	bool useVBO;
	const GLvoid* ptr = ParsePointer(L, 2, 1, NULL, maxElem, useVBO,
	                                 GL_EDGE_FLAG_ARRAY);
	if (maxElem <= 0) {
		return 0;
	}
		
	glEdgeFlagPointer(stride, ptr);
	glEnableClientState(GL_EDGE_FLAG_ARRAY);
	if (useVBO) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	return 0;
}



/******************************************************************************/
/******************************************************************************/

static const void* ParseIndices(lua_State* L, int index, size_t count,
                                GLenum& type, bool& useVBO, int& maxElem)
{
	// GLBuffer index data
	const LuaGLBuffer* buf = LuaGLBufferMgr::TestLuaGLBuffer(L, index);
	if (buf != NULL) {
		if ((buf->target != GL_ELEMENT_ARRAY_BUFFER) ||
		    (buf->indexMax == 0)  ||
		    (buf->indexType == 0) ||
		    (buf->indexTypeSize == 0)) {
			luaL_error(L, "invalid GLBuffer target for indices");
		}

		const int offset = luaL_optint(L, index + 1, 0);
		if ((offset % buf->indexTypeSize) != 0) {
			luaL_error(L, "invalid GLBuffer index offset: %i vs %i",
			           offset, buf->indexTypeSize);
		}
		if (offset > buf->size) {
			luaL_error(L, "indices offset is larger than the buffer data");
		}
		if (((buf->size - offset) / buf->indexTypeSize) < count) {
			luaL_error(L, "index data size is too small for the element count");
		}

		useVBO = true;

		type = buf->indexType;

		maxElem = buf->indexMax;

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf->id);

		return (const void*)((char*)offset);
	}

	// string index data
	type = (GLenum)luaL_checkint(L, index);
	const int typeSize = LuaGLBufferMgr::GetIndexTypeSize(type);
	if (typeSize < 0) {
		luaL_error(L, "unknown index type: 0x%0x", type);
	}
	
	size_t size;
	const char* data = luaL_checklstring(L, index + 1, &size);
	const size_t offset = luaL_optint(L, index + 2, 0);

	if (offset > size) {
		luaL_error(L, "indices offset is larger than the data: %i vs %i",
							 offset, size);
	}
	data += offset;
	size -= offset;

	if ((size / typeSize) < count) {
		luaL_error(L, "index data size is too small for the element count: %i vs %i",
		           (size / typeSize), count);
	}

	maxElem = LuaGLBufferMgr::CalcMaxElement(type, size, data);
	if (maxElem < 0) {
		luaL_error(L, "error parsing index data");
	}

	return data;		
}


static inline void CheckMaxElement(lua_State* L, int maxElem)
{
	if ((maxElem < 0) || (maxElem >= minElements)) {
		luaL_error(L, "max index vs. data buffers mismatch: %i vs %i",
		           maxElem, minElements);
	}
}


/******************************************************************************/
/******************************************************************************/

int LuaGLPointers::ArrayElement(lua_State* L)
{
	CheckActiveState(L, __FUNCTION__);

	const GLint element = (GLint)luaL_checkint(L, 1);

	CheckMaxElement(L, element);

	glArrayElement(element);

	return 0;
}


int LuaGLPointers::DrawArrays(lua_State* L)
{
	CheckActiveState(L, __FUNCTION__);

	const GLenum  mode  = (GLenum) luaL_checkint(L, 1);
	const GLsizei count = (GLsizei)luaL_checkint(L, 2);
	const GLint   first = (GLint)  luaL_optint(L, 3, 0);

	if (first < 0) {
		luaL_error(L, "the first element can not be negative");
	}

	CheckMaxElement(L, count + first - 1);

	glDrawArrays(mode, first, count);

	return 0;
}


int LuaGLPointers::DrawElements(lua_State* L)
{
	CheckActiveState(L, __FUNCTION__);

	const GLenum  mode  = (GLenum) luaL_checkint(L, 1);
	const GLsizei count = (GLsizei)luaL_checkint(L, 2);

	GLenum type;
	int maxElem = 0;
	bool useVBO = false;
	const GLvoid* indices = ParseIndices(L, 3, count, type, useVBO, maxElem);

	if (type == 0) {
		luaL_error(L, "unknown index data type");
	}
	CheckMaxElement(L, maxElem);

	glDrawElements(mode, count, type, indices);

	if (useVBO) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	
	return 0;
}


int LuaGLPointers::DrawRangeElements(lua_State* L)
{
	CheckActiveState(L, __FUNCTION__);

	const GLenum  mode  = (GLenum) luaL_checkint(L, 1);
	const GLuint  start = (GLuint) luaL_checkint(L, 2);
	const GLuint  end   = (GLuint) luaL_checkint(L, 3);
	const GLsizei count = (GLsizei)luaL_checkint(L, 4);

	GLenum type;
	int maxElem = 0;
	bool useVBO = false;
	const GLvoid* indices = ParseIndices(L, 5, count, type, useVBO, maxElem);

	if (type == 0) {
		luaL_error(L, "unknown index data type");
	}
	CheckMaxElement(L, maxElem);

	glDrawRangeElements(mode, start, end, count, type, indices);

	if (useVBO) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	return 0;
}


int LuaGLPointers::DrawArraysInstanced(lua_State* L)
{
	CheckActiveState(L, __FUNCTION__);
	if (OpenGLPassState::CreatingList()) {
		luaL_error(L, "can not be used in a display list");
	}
	const GLsizei prims = (GLsizei)luaL_checkint(L, 1);
	const GLenum  mode  = (GLenum) luaL_checkint(L, 2);
	const GLsizei count = (GLsizei)luaL_checkint(L, 3);
	const GLuint  first = (GLuint) luaL_optint(L, 4, 0);

	CheckMaxElement(L, count + first - 1);
	
	glDrawArraysInstancedARB(mode, first, count, prims);

	return 0;
}


int LuaGLPointers::DrawElementsInstanced(lua_State* L)
{
	CheckActiveState(L, __FUNCTION__);
	if (OpenGLPassState::CreatingList()) {
		luaL_error(L, "can not be used in a display list");
	}
	const GLsizei prims = (GLsizei)luaL_checkint(L, 1);
	const GLenum  mode  = (GLenum) luaL_checkint(L, 2);
	const GLsizei count = (GLsizei)luaL_checkint(L, 3);

	GLenum type;
	int maxElem = 0;
	bool useVBO = false;
	const GLvoid* indices = ParseIndices(L, 4, count, type, useVBO, maxElem);

	if (type == 0) {
		luaL_error(L, "unknown index data type");
	}
	CheckMaxElement(L, maxElem);

	glDrawElementsInstancedARB(mode, count, type, indices, prims);

	if (useVBO) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	return 0;
}


/******************************************************************************/
/******************************************************************************/

int LuaGLPointers::GetPointerState(lua_State* L)
{
	CheckActiveState(L, __FUNCTION__);

	lua_createtable(L, 0, maxElements.size());
	map<GLenum, int>::const_iterator it;
	for (it = maxElements.begin(); it != maxElements.end(); ++it) {
		const GLenum type = it->first;
		lua_pushinteger(L, type);
		lua_newtable(L); {
			lua_pushliteral(L, "elements");
			lua_pushinteger(L, it->second);
			lua_rawset(L, -3);

			lua_pushliteral(L, "data");
			map<GLenum, int>::const_iterator rit = luaRefs.find(type);
			if (rit != luaRefs.end()) {
				lua_rawgeti(L, LUA_REGISTRYINDEX, rit->second);
			} else {
				lua_pushboolean(L, false);
			}
			lua_rawset(L, -3);
		}
		lua_rawset(L, -3);
	}

	return 1;
}


/******************************************************************************/
/******************************************************************************/

