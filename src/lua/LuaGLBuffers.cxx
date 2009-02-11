
#include "common.h"

// implementation header
#include "LuaGLBuffers.h"

// system headers
#include <string>
#include <set>
using std::string;
using std::set;

// common headers
#include "bzfgl.h"
#include "OpenGLGState.h"

// local headers
#include "LuaInclude.h"
#include "LuaHashString.h"
#include "LuaUtils.h"


LuaGLBufferMgr luaGLBufferMgr;

const char* LuaGLBufferMgr::metaName = "GLBuffer";

const int LuaGLBufferMgr::maxDataSize = (64 * 1024 * 1024);


/******************************************************************************/
/******************************************************************************/

LuaGLBuffer::LuaGLBuffer(const LuaGLBufferData& bufData)
: LuaGLBufferData(bufData)
{
	OpenGLGState::registerContextInitializer(StaticFreeContext,
	                                         StaticInitContext, this);
}


LuaGLBuffer::~LuaGLBuffer()
{
	FreeContext();
	OpenGLGState::unregisterContextInitializer(StaticFreeContext,
	                                           StaticInitContext, this);
}


void LuaGLBuffer::Delete()
{
	FreeContext();
}


void LuaGLBuffer::InitContext()
{
}


void LuaGLBuffer::FreeContext()
{
	if (id == 0) {
		return;
	}
	glDeleteBuffers(1, &id);
	id = 0;
}


void LuaGLBuffer::StaticInitContext(void* data)
{
	((LuaGLBuffer*)data)->InitContext();
}


void LuaGLBuffer::StaticFreeContext(void* data)
{
	((LuaGLBuffer*)data)->FreeContext();
}


/******************************************************************************/
/******************************************************************************/

LuaGLBufferMgr::LuaGLBufferMgr()
{
}


LuaGLBufferMgr::~LuaGLBufferMgr()
{
}


/******************************************************************************/
/******************************************************************************/

bool LuaGLBufferMgr::PushEntries(lua_State* L)
{
	CreateMetatable(L);

	PUSH_LUA_CFUNC(L, CreateBuffer);
	PUSH_LUA_CFUNC(L, DeleteBuffer);
	PUSH_LUA_CFUNC(L, BufferData);
	PUSH_LUA_CFUNC(L, BufferSubData);
	PUSH_LUA_CFUNC(L, GetBufferSubData);

	return true;
}


/******************************************************************************/
/******************************************************************************/

const LuaGLBuffer* LuaGLBufferMgr::TestLuaGLBuffer(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		return NULL;
	}
	return (LuaGLBuffer*)lua_touserdata(L, index);
}


const LuaGLBuffer* LuaGLBufferMgr::CheckLuaGLBuffer(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		luaL_argerror(L, index, "expected GLBuffer");
	}
	return (LuaGLBuffer*)lua_touserdata(L, index);
}


LuaGLBuffer* LuaGLBufferMgr::GetLuaGLBuffer(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		luaL_argerror(L, index, "expected GLBuffer");
	}
	return (LuaGLBuffer*)lua_touserdata(L, index);
}


/******************************************************************************/
/******************************************************************************/

int LuaGLBufferMgr::GetTypeSize(GLenum type)
{
	switch (type) {
		case GL_BYTE:           { return sizeof(GLbyte);   }
		case GL_UNSIGNED_BYTE:  { return sizeof(GLubyte);  }
		case GL_SHORT:          { return sizeof(GLshort);  }
		case GL_UNSIGNED_SHORT: { return sizeof(GLushort); }
		case GL_HALF_FLOAT:     { return sizeof(GLhalf);   }
		case GL_INT:            { return sizeof(GLint);    }
		case GL_UNSIGNED_INT:   { return sizeof(GLuint);   }
		case GL_FLOAT:          { return sizeof(GLfloat);  }
		case GL_DOUBLE:         { return sizeof(GLdouble); }
	}
	return -1;
}


int LuaGLBufferMgr::GetIndexTypeSize(GLenum type)
{
	switch (type) {
		case GL_UNSIGNED_INT:   { return sizeof(GLuint);   }
		case GL_UNSIGNED_BYTE:  { return sizeof(GLubyte);  }
		case GL_UNSIGNED_SHORT: { return sizeof(GLushort); }
	}
	return -1;
}


/******************************************************************************/
/******************************************************************************/

template<typename T>
int CalcMaxElem(int bytes, const void* indices)
{
	int maxElem = 0;
	const size_t typeSize = sizeof(T);
	const size_t count = (bytes / typeSize);
	for (size_t i = 0; i < count; i++) {
		const int value = (int)(*(((T*)indices) + i));
		if (value < 0) {
			return -1;
		}
		if (maxElem < value) {
			maxElem = value;
		}
	}
	return maxElem;
}


int LuaGLBufferMgr::CalcMaxElement(GLenum type, int bytes, const void* indices)
{
	if ((bytes <= 0) || (indices == NULL)) {
		return -1;
	}
	switch (type) {
		case GL_UNSIGNED_INT:   { return CalcMaxElem<GLuint>  (bytes, indices); }
		case GL_UNSIGNED_BYTE:  { return CalcMaxElem<GLubyte> (bytes, indices); }
		case GL_UNSIGNED_SHORT: { return CalcMaxElem<GLushort>(bytes, indices); }
	}
	return -1;
}


/******************************************************************************/
/******************************************************************************/

bool LuaGLBufferMgr::CreateMetatable(lua_State* L)
{
	luaL_newmetatable(L, metaName);
	HSTR_PUSH_CFUNC(L,  "__gc",        MetaGC);
	HSTR_PUSH_CFUNC(L,  "__index",     MetaIndex);
	HSTR_PUSH_CFUNC(L,  "__newindex",  MetaNewindex);
	HSTR_PUSH_STRING(L, "__metatable", "no access");
	lua_pop(L, 1);
	return true;
}


int LuaGLBufferMgr::MetaGC(lua_State* L)
{
	LuaGLBuffer* buf = GetLuaGLBuffer(L, 1);
	buf->~LuaGLBuffer();
	return 0;
}


int LuaGLBufferMgr::MetaIndex(lua_State* L)
{
	const LuaGLBuffer* buf = CheckLuaGLBuffer(L, 1);
	if (!lua_israwstring(L, 2)) {
		return 0;
	}
	const string key = luaL_checkstring(L, 2);
	if (key == "valid") {
		lua_pushboolean(L, glIsBuffer(buf->id));
	}
	else if (key == "target")    { lua_pushinteger(L, buf->target);    }
	else if (key == "usage")     { lua_pushinteger(L, buf->usage);     }
	else if (key == "size")      { lua_pushinteger(L, buf->size);      }
	else if (key == "indexType") { lua_pushinteger(L, buf->indexType); }
	else {
		return 0;
	}
	return 1;
}


int LuaGLBufferMgr::MetaNewindex(lua_State* /*L*/)
{
	return 0;
}


/******************************************************************************/
/******************************************************************************/

const void* LuaGLBufferMgr::ParseArgs(lua_State* L, int index,
                                      LuaGLBufferData& bufData)
{
	const char* data = NULL;
	size_t size = 0;
	bufData.size = 0;

	if (lua_israwstring(L, index)) {
		data = lua_tolstring(L, index, &size);
	}
	else if (lua_israwnumber(L, index)) {
		size = (size_t)lua_toint(L, index);
	}

	const size_t offset  = luaL_optint(L, index + 1, 0);
	const size_t maxSize = luaL_optint(L, index + 2, size);

	bufData.target    = luaL_optint(L, index + 3, bufData.target);
	bufData.usage     = luaL_optint(L, index + 4, bufData.usage);
	bufData.indexType = luaL_optint(L, index + 5, 0);

	if (data != NULL) {
		// offset adjustment
		if (offset > size) {
			luaL_error(L, "offset exceeds data size");
		}
		data += offset;
		size -= offset;

		// size adjustment
		if (maxSize > size) {
			luaL_error(L, "requested size exceeds data size");
		}
		size = maxSize;
	}

	bufData.size = (GLsizei)size;

	IndexCheck(L, bufData, data);

	return data;
}


const void* LuaGLBufferMgr::ParseTable(lua_State* L, int index,
                                       LuaGLBufferData& bufData)
{
	const char* data = NULL;

	size_t offset = 0;
	size_t size = 0;
	size_t maxSize = 0;
	bool haveMaxSize = false;

	for (lua_pushnil(L); lua_next(L, index) != 0; lua_pop(L, 1)) {
		if (lua_israwstring(L, -2)) {
			const string key = lua_tostring(L, -2);
			if (key == "data") {
				data = luaL_checklstring(L, -1, &size);
			}
			else if (key == "offset") {
				offset = (size_t)luaL_checkint(L, -1);
			}
			else if (key == "size") {
				maxSize = (size_t)luaL_checkint(L, -1);
				haveMaxSize = true;
			}
			else if (key == "target") {
				bufData.target = (GLenum)luaL_checkint(L, -1);
			}
			else if (key == "usage") {
				bufData.usage = (GLenum)luaL_checkint(L, -1);
			}
			else if (key == "indexType") {
				bufData.indexType = (GLenum)luaL_checkint(L, -1);
			}
		}
	}

	if (data != NULL) {
		if (offset > size) {
			luaL_error(L, "offset exceeds data size");
		}
		data += offset;
		size -= offset;

		if (haveMaxSize) {
			if (maxSize > size) {
				luaL_error(L, "requested size exceeds data size");
			}
			else if (maxSize < size) {
				size = maxSize;
			}
		}
	}
  
	bufData.size = size;	

	IndexCheck(L, bufData, data);

	return data;
}


void LuaGLBufferMgr::IndexCheck(lua_State* L,
                                LuaGLBufferData& bufData, const void* data)
{
	bufData.indexMax      = 0;
	bufData.indexTypeSize = 0;

	if (bufData.target != GL_ELEMENT_ARRAY_BUFFER) {
		bufData.indexType = 0;
		return;
	}

	if (data == NULL) {
		bufData.indexType = 0;
		return;
	}
		
	const int typeSize = GetIndexTypeSize(bufData.indexType);
	if (typeSize < 0) {
		luaL_error(L, "unknown index buffer data type");
	}
	bufData.indexTypeSize = (GLuint)typeSize;

	const int maxElem = CalcMaxElement(bufData.indexType, bufData.size, data);
	if (maxElem < 0) {
		bufData.indexTypeSize = 0;
		luaL_error(L, "invalid index buffer data");
	}
	bufData.indexMax = (GLuint)maxElem;
}


void LuaGLBufferMgr::SetBufferData(const LuaGLBufferData& bufData,
                                   const void* data)
{
	glBindBuffer(bufData.target, bufData.id);
	glBufferData(bufData.target, bufData.size, data, bufData.usage);
	glBindBuffer(bufData.target, 0);
}


/******************************************************************************/
/******************************************************************************/

int LuaGLBufferMgr::CreateBuffer(lua_State* L)
{
	LuaGLBufferData bufData;

	const char* data = NULL;
	if (lua_israwstring(L, 1)) {
		data = (const char*)ParseArgs(L, 1, bufData);
	}
	else if (lua_istable(L, 1)) {
		data = (const char*)ParseTable(L, 1, bufData);
	}
	else {
		luaL_argerror(L, 2, "expected number or table");
	}

	glGenBuffers(1, &bufData.id);
	if (bufData.id == 0) {
		return 0;
	}

	glGetError();
	SetBufferData(bufData, data);
	const GLenum errgl = glGetError();
	if (errgl != GL_NO_ERROR) {
		glDeleteBuffers(1, &bufData.id);
		lua_pushnil(L);
		lua_pushinteger(L, errgl);
		return 2;
	}

	void* udData = lua_newuserdata(L, sizeof(LuaGLBuffer));
	new(udData) LuaGLBuffer(bufData);

	lua_setuserdataextra(L, -1, (void*)metaName);
	luaL_getmetatable(L, metaName);
	lua_setmetatable(L, -2);

	return 1;
}


int LuaGLBufferMgr::DeleteBuffer(lua_State* L)
{
	if (OpenGLGState::isExecutingInitFuncs()) {
		luaL_error(L, "gl.DeleteBuffer can not be used in GLReload");
	}
	if (lua_isnil(L, 1)) {
		return 0;
	}
	LuaGLBuffer* buf = GetLuaGLBuffer(L, 1);
	buf->Delete();
	return 0;
}


int LuaGLBufferMgr::BufferData(lua_State* L)
{
	LuaGLBuffer* buf = GetLuaGLBuffer(L, 1);
	if (!buf->IsValid()) {
		lua_pushboolean(L, false);
		return 1;
	}
	LuaGLBufferData bufData = *buf;

	const char* data = NULL;
	if (lua_israwstring(L, 2)) {
		data = (const char*)ParseArgs(L, 2, bufData);
	}
	else if (lua_istable(L, 2)) {
		data = (const char*)ParseTable(L, 2, bufData);
	}
	else {
		luaL_argerror(L, 2, "expected number or table");
	}

	glGetError();
	SetBufferData(bufData, data);
	const GLenum errgl = glGetError();
	if (errgl != GL_NO_ERROR) {
		lua_pushboolean(L, false);
		lua_pushinteger(L, errgl);
		return 2;
	}

	*buf = bufData; // success, update the buffer info

	lua_pushboolean(L, true);
	return 1;
}


int LuaGLBufferMgr::BufferSubData(lua_State* L)
{
	LuaGLBuffer* buf = GetLuaGLBuffer(L, 1);
	if (!buf->IsValid()) {
		lua_pushboolean(L, false);
		return 1;
	}

	size_t size = 0;
	const GLintptr glOffset   = (GLintptr)luaL_checkint(L, 2);
	const char*    data       = luaL_checklstring(L, 3, &size);
	const size_t   dataOffset = (size_t)  luaL_optint(L, 4, 0);
	const size_t   dataSize   = (size_t)  luaL_optint(L, 5, size);
	const GLenum   target     = (GLenum)  luaL_optint(L, 6, buf->target);

	if (dataOffset > size) {
		luaL_error(L, "offset exceeds data size");
	}
	data += dataOffset;
	size -= dataOffset;

	if (size < dataSize) {
		luaL_error(L, "desired size exceeds data size");
	}
	size = dataSize;

	if (buf->target == GL_ELEMENT_ARRAY_BUFFER) {
		// extra checks for index buffers
		if (target != GL_ELEMENT_ARRAY_BUFFER) {
			luaL_error(L, "index buffer target type can not be changed");
		}
		if (((glOffset % buf->indexTypeSize) != 0) ||
		    ((size     % buf->indexTypeSize) != 0)) {
			luaL_error(L, "index buffer subdata must be aligned");
		}
	}

	if (buf->target == GL_ELEMENT_ARRAY_BUFFER) {
		const int maxElem = CalcMaxElement(buf->indexType, size, data);
		if (maxElem < 0) {
			luaL_error(L, "invalid index data");
		}
		if (buf->indexMax < (GLuint)maxElem) {
			buf->indexMax = (GLuint)maxElem;
		}
	}

	glGetError();
	glBindBuffer(target, buf->id);
	glBufferSubData(target, glOffset, size, data);
	glBindBuffer(target, 0);
	const GLenum errgl = glGetError();
	if (errgl != GL_NO_ERROR) {
		lua_pushboolean(L, false);
		lua_pushinteger(L, errgl);
		return 2;
	}

	lua_pushboolean(L, true);
	return 1;
}


int LuaGLBufferMgr::GetBufferSubData(lua_State* L)
{
	const LuaGLBuffer* buf = CheckLuaGLBuffer(L, 1);
	if (!buf->IsValid()) {
		lua_pushboolean(L, false);
		return 1;
	}

	const GLintptr offset = (GLintptr)luaL_optint(L, 2, 0);
	const GLsizei  size   = (GLsizei) luaL_optint(L, 3, 1);
	const GLenum   target = (GLenum)  luaL_optint(L, 4, buf->target);

	if (size <= 0) {
		return 0;
	}
	if (size > maxDataSize) {
		luaL_error(L, "maximum gl.GetBufferSubData is %d", maxDataSize);
	}

	char* data = new char[size];

	glGetError();
	glBindBuffer(target, buf->id);
	glGetBufferSubData(target, offset, size, data);
	glBindBuffer(target, 0);
	const GLenum errgl = glGetError();

	if (errgl != GL_NO_ERROR) {
		lua_pushboolean(L, false);
		lua_pushinteger(L, errgl);
		delete[] data;
		return 2;
	}

	lua_pushlstring(L, data, size);
	delete[] data;
	return 1;
}


/******************************************************************************/
/******************************************************************************/
