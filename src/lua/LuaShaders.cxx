
#include "common.h"

// implementation header
#include "LuaShaders.h"

// system headers
#include <string>
#include <vector>
#include <set>
using std::string;
using std::vector;
using std::set;

// common headers
#include "TextUtils.h"
#include "bzfgl.h"
#include "bzfio.h"
#include "OpenGLGState.h"
#include "SceneRenderer.h"

// local headers
#include "LuaInclude.h"
#include "LuaHashString.h"
#include "LuaHandle.h"
#include "LuaOpenGL.h"


static string errorLog; // FIXME


const char* LuaShaderMgr::metaName = "Shader";

LuaShaderMgr luaShaderMgr;

int LuaShaderMgr::activeShaderDepth = 0;


/******************************************************************************/
/******************************************************************************/
//
//  LuaShader
//

LuaShader::LuaShader(GLuint _progID, const vector<Object>& _objects)
: progID(_progID)
, objects(_objects)
{
	OpenGLGState::registerContextInitializer(StaticFreeContext,
	                                         StaticInitContext, this);
}


LuaShader::~LuaShader()
{
	OpenGLGState::unregisterContextInitializer(StaticFreeContext,
	                                           StaticInitContext, this);
	FreeContext();
}


void LuaShader::InitContext()
{
}


void LuaShader::FreeContext()
{
	for (int o = 0; o < (int)objects.size(); o++) {
		Object& obj = objects[o];
		glDetachShader(progID, obj.id);
		glDeleteShader(obj.id);
	}
	objects.clear();
	glDeleteProgram(progID);
	progID = 0;
}


void LuaShader::StaticInitContext(void* data)
{
	((LuaShader*)data)->InitContext();
}


void LuaShader::StaticFreeContext(void* data)
{
	((LuaShader*)data)->FreeContext();
}


/******************************************************************************/
/******************************************************************************/
//
//  LuaShaderMgr
//

bool LuaShaderMgr::PushEntries(lua_State* L)
{
	CreateMetatable(L);

	PUSH_LUA_CFUNC(L, CreateShader);
	PUSH_LUA_CFUNC(L, DeleteShader);
	PUSH_LUA_CFUNC(L, UseShader);
	PUSH_LUA_CFUNC(L, ActiveShader);

	PUSH_LUA_CFUNC(L, GetActiveUniforms);
	PUSH_LUA_CFUNC(L, GetUniformLocation);
	PUSH_LUA_CFUNC(L, Uniform);
	PUSH_LUA_CFUNC(L, UniformInt);
	PUSH_LUA_CFUNC(L, UniformMatrix);

	PUSH_LUA_CFUNC(L, SetShaderParameter);

	PUSH_LUA_CFUNC(L, GetShaderLog);

	return true;
}


/******************************************************************************/
/******************************************************************************/

bool LuaShaderMgr::CreateMetatable(lua_State* L)
{
  luaL_newmetatable(L, metaName);
  HSTR_PUSH_CFUNC(L, "__gc",    MetaGC);
  HSTR_PUSH_CFUNC(L, "__index", MetaIndex);
  lua_pop(L, 1);  
  return true;
}


int LuaShaderMgr::MetaGC(lua_State* L)
{
	LuaShader*& shader = CheckLuaShader(L, 1);
	delete shader;
	shader = NULL;
	return 0;
}


int LuaShaderMgr::MetaIndex(lua_State* L)
{
	LuaShader* shader = CheckLuaShader(L, 1);
	if (shader == NULL) {
		return 0;
	}

	if (!lua_israwstring(L, 2)) {
		return 0;
	}

	const string key = lua_tostring(L, 2);
	if (key == "valid") {
		lua_pushboolean(L, shader->IsValid());
	}

	return 1;
}


/******************************************************************************/
/******************************************************************************/

void LuaShaderMgr::Init()
{
	OpenGLGState::registerContextInitializer(StaticFreeContext,
	                                         StaticInitContext, NULL);
}


void LuaShaderMgr::Free()
{
	OpenGLGState::unregisterContextInitializer(StaticFreeContext,
	                                           StaticInitContext, NULL);
}


inline LuaShader*& LuaShaderMgr::CheckLuaShader(lua_State* L, int index)
{
	return *((LuaShader**)luaL_checkudata(L, index, metaName));
}


/******************************************************************************/
/******************************************************************************/

int LuaShaderMgr::GetShaderLog(lua_State* L)
{
	lua_pushstring(L, errorLog.c_str());
	return 1;
}


/******************************************************************************/
/******************************************************************************/

static int ParseIntArray(lua_State* L, int* array, int size)
{
	if (!lua_istable(L, -1)) {
		return -1;
	}
	const int table = lua_gettop(L);
	for (int i = 0; i < size; i++) {
		lua_rawgeti(L, table, (i + 1));
		if (lua_israwnumber(L, -1)) {
			array[i] = lua_toint(L, -1);
			lua_pop(L, 1);
		} else {
			lua_pop(L, 1);
			return i;
		}
	}
	return size;
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


static bool ParseUniformTable(lua_State* L, int index, GLuint progID)
{
	lua_getfield(L, index, "uniform");
	if (lua_istable(L, -1)) {
		const int table = lua_gettop(L);
		for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
			if (lua_israwstring(L, -2)) {
				const string name = lua_tostring(L, -2);
				const GLint loc = glGetUniformLocation(progID, name.c_str());
				if (loc >= 0) {
					if (lua_israwnumber(L, -1)) {
						const float value = lua_tofloat(L, -1);
						glUniform1f(loc, value);
					}
					else if (lua_istable(L, -1)) {
						float a[4];
						const int count = ParseFloatArray(L, a, 4);
						switch (count) {
							case 1: { glUniform1f(loc, a[0]);                   break; }
							case 2: { glUniform2f(loc, a[0], a[1]);             break; }
							case 3: { glUniform3f(loc, a[0], a[1], a[2]);       break; }
							case 4: { glUniform4f(loc, a[0], a[1], a[2], a[3]); break; }
						}
					}
				}
			}				
		}
	}
	lua_pop(L, 1);
	return true;
}


static bool ParseUniformIntTable(lua_State* L, int index, GLuint progID)
{
	lua_getfield(L, index, "uniformInt");
	if (lua_istable(L, -1)) {
		const int table = lua_gettop(L);
		for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
			if (lua_israwstring(L, -2)) {
				const string name = lua_tostring(L, -2);
				const GLint loc = glGetUniformLocation(progID, name.c_str());
				if (loc >= 0) {
					if (lua_israwnumber(L, -1)) {
						const int value = lua_toint(L, -1);
						glUniform1i(loc, value);
					}
					else if (lua_istable(L, -1)) {
						int a[4];
						const int count = ParseIntArray(L, a, 4);
						switch (count) {
							case 1: { glUniform1i(loc, a[0]);                   break; }
							case 2: { glUniform2i(loc, a[0], a[1]);             break; }
							case 3: { glUniform3i(loc, a[0], a[1], a[2]);       break; }
							case 4: { glUniform4i(loc, a[0], a[1], a[2], a[3]); break; }
						}
					}
				}
			}				
		}
	}
	lua_pop(L, 1);
	return true;
}


static bool ParseUniformMatrixTable(lua_State* L, int index, GLuint progID)
{
	lua_getfield(L, index, "uniformMatrix");
	if (lua_istable(L, -1)) {
		const int table = lua_gettop(L);
		for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
			if (lua_israwstring(L, -2)) {
				const string name = lua_tostring(L, -2);
				const GLint loc = glGetUniformLocation(progID, name.c_str());
				if (loc >= 0) {
					if (lua_istable(L, -1)) {
						float a[16];
						const int count = ParseFloatArray(L, a, 16);
						switch (count) {
							case (2 * 2): { glUniformMatrix2fv(loc, 1, GL_FALSE, a); break; }
							case (3 * 3): { glUniformMatrix3fv(loc, 1, GL_FALSE, a); break; }
							case (4 * 4): { glUniformMatrix4fv(loc, 1, GL_FALSE, a); break; }
						}
					}
				}
			}				
		}
	}
	lua_pop(L, 1);
	return true;
}


static bool ParseUniformSetupTables(lua_State* L, int index, GLuint progID)
{
	GLint currentProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

	glUseProgram(progID);

	const bool success = ParseUniformTable(L,       index, progID) &&
	                     ParseUniformIntTable(L,    index, progID) &&
	                     ParseUniformMatrixTable(L, index, progID);

	glUseProgram(currentProgram);

	return success;
}


/******************************************************************************/
/******************************************************************************/

static GLuint CompileObject(const vector<string>& sources, GLenum type,
                            bool& success)
{
	if (sources.empty()) {
		success = true;
		return 0;
	}

	GLuint obj = glCreateShader(type);
	if (obj == 0) {
		errorLog = "Could not create shader object";
		return 0;
	}

	const int count = (int)sources.size();
	
	const GLchar** texts = new const GLchar*[count];
	for (int i = 0; i < count; i++) {
		texts[i] = sources[i].c_str();
	}
	
	glShaderSource(obj, count, texts, NULL);

	delete[] texts;

	glCompileShader(obj);

	GLint result;
	glGetShaderiv(obj, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE) {
		GLchar log[4096];
		GLsizei logSize = sizeof(log);
		glGetShaderInfoLog(obj, logSize, &logSize, log);
		
		errorLog = log;
		if (errorLog.empty()) {
			errorLog = "Empty error message:  code = "
			           + IntToString(result) + " (0x"
			           + IntToString(result, "%04X") + ")";
		}
		
		glDeleteShader(obj);

		success = false;
		return 0;
	}

	success = true;
	return obj;
}


static bool ParseSources(lua_State* L, int table,
                         const char* type, vector<string>& srcs)
{
	lua_getfield(L, table, type);
	
	if (lua_israwstring(L, -1)) {
		const string src = lua_tostring(L, -1);
		if (!src.empty()) {
			srcs.push_back(src);
		}
	}
	else if (lua_istable(L, -1)) {
		const int table2 = lua_gettop(L);
		for (lua_pushnil(L); lua_next(L, table2) != 0; lua_pop(L, 1)) {
			if (!lua_israwnumber(L, -2) || !lua_israwstring(L, -1)) {
				continue;
			}
			const string src = lua_tostring(L, -1);
			if (!src.empty()) {
				srcs.push_back(src);
			}
		}
	}
	else if (!lua_isnil(L, -1)) {
		errorLog = "Invalid " + string(type) + " shader source";
		lua_pop(L, 1);
		return false;
	}

	lua_pop(L, 1);
	return true;
}


static bool ApplyGeometryParameters(lua_State* L, int table, GLuint prog)
{
	if (!glProgramParameteriEXT) {
		return true;
	}

	struct { const char* name; GLenum param; } parameters[] = {
		{ "geoInputType",   GL_GEOMETRY_INPUT_TYPE_EXT },
		{ "geoOutputType",  GL_GEOMETRY_OUTPUT_TYPE_EXT },
		{ "geoOutputVerts", GL_GEOMETRY_VERTICES_OUT_EXT }
	};

	const int count = sizeof(parameters) / sizeof(parameters[0]);
	for (int i = 0; i < count; i++) {
		lua_getfield(L, table, parameters[i].name);
		if (lua_israwnumber(L, -1)) {
			const GLint type = lua_toint(L, -1);
			glProgramParameteriEXT(prog, parameters[i].param, type);
		}
		lua_pop(L, 1);
	}

	return true;
}


int LuaShaderMgr::CreateShader(lua_State* L)
{
	const int args = lua_gettop(L);
	if ((args != 1) || !lua_istable(L, 1)) {
		luaL_error(L, "Incorrect arguments to gl.CreateShader()");
	}

	vector<string> vertSrcs;
	vector<string> geomSrcs;
	vector<string> fragSrcs;
	if (!ParseSources(L, 1, "vertex",   vertSrcs) ||
	    !ParseSources(L, 1, "geometry", geomSrcs) ||
	    !ParseSources(L, 1, "fragment", fragSrcs)) {
		return 0;
	}
	if (vertSrcs.empty() &&
	    fragSrcs.empty() &&
	    geomSrcs.empty()) {
		return 0;
	}

	bool success;
	const GLuint vertObj = CompileObject(vertSrcs, GL_VERTEX_SHADER, success);
	if (!success) {
		return 0;
	}
	const GLuint geomObj = CompileObject(geomSrcs, GL_GEOMETRY_SHADER_EXT, success);
	if (!success) {
		glDeleteShader(vertObj);
		return 0;
	}
	const GLuint fragObj = CompileObject(fragSrcs, GL_FRAGMENT_SHADER, success);
	if (!success) {
		glDeleteShader(vertObj);
		glDeleteShader(geomObj);
		return 0;
	}

	const GLuint progID = glCreateProgram();
	vector<LuaShader::Object> objects;

	if (vertObj != 0) {
		glAttachShader(progID, vertObj);
		objects.push_back(LuaShader::Object(vertObj, GL_VERTEX_SHADER));
	}
	if (geomObj != 0) {
		glAttachShader(progID, geomObj);
		objects.push_back(LuaShader::Object(geomObj, GL_GEOMETRY_SHADER_EXT));
		ApplyGeometryParameters(L, 1, progID); // done before linking
	}
	if (fragObj != 0) {
		glAttachShader(progID, fragObj);
		objects.push_back(LuaShader::Object(fragObj, GL_FRAGMENT_SHADER));
	}

	glLinkProgram(progID);

	GLint result;
	glGetProgramiv(progID, GL_LINK_STATUS, &result);
	if (result != GL_TRUE) {
		GLchar log[4096];
		GLsizei logSize = sizeof(log);
		glGetProgramInfoLog(progID, logSize, &logSize, log);
		errorLog = log;
		for (size_t i = 0; i < objects.size(); i++) {
			glDetachShader(progID, objects[i].id);
			glDeleteShader(objects[i].id);
		}
		return 0;
	}

	// Allows setting up uniforms when drawing is disabled
	// (much more convenient for sampler uniforms, and static
	//  configuration values)
	ParseUniformSetupTables(L, 1, progID);

	LuaShader** shaderPtr =
		(LuaShader**)lua_newuserdata(L, sizeof(LuaShader*));
	luaL_getmetatable(L, metaName);
	lua_setmetatable(L, -2);

	*shaderPtr = new LuaShader(progID, objects);

	return 1;
}


int LuaShaderMgr::DeleteShader(lua_State* L)
{
	if (lua_isnil(L, 1)) {
		return 0;
	}
	LuaShader*& shader = CheckLuaShader(L, 1);
	delete shader;
	shader = NULL;
	return 0;
}


int LuaShaderMgr::UseShader(lua_State* L)
{
	LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);

	if (lua_isboolean(L, 1) && !lua_tobool(L, 1)) {
		glUseProgram(0);
		lua_pushboolean(L, true);
		return 1;
	}
		
	LuaShader* shader = CheckLuaShader(L, 1);
	if ((shader == NULL) || !shader->IsValid()) {
		lua_pushboolean(L, false);
		return 1;
	}

	glUseProgram(shader->GetProgID());
	lua_pushboolean(L, true);

	return 1;
}


int LuaShaderMgr::ActiveShader(lua_State* L)
{
	GLuint progID = 0;
	if (!lua_isnil(L, 1)) {
		LuaShader* shader = CheckLuaShader(L, 1);
		if ((shader == NULL) || !shader->IsValid()) {
			return 0;
		}
	}
	luaL_checktype(L, 2, LUA_TFUNCTION);

	GLint currentProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

	glUseProgram(progID);
	activeShaderDepth++;
	const int error = lua_pcall(L, lua_gettop(L) - 2, 0, 0);
	activeShaderDepth--;
	glUseProgram(currentProgram);

	if (error != 0) {
		LuaLog("gl.ActiveShader: error(%i) = %s", error, lua_tostring(L, -1));
		lua_error(L);
	}

	return 0;
}


/******************************************************************************/
/******************************************************************************/

static const char* UniformTypeString(GLenum type)
{
#define UNIFORM_STRING_CASE(x)                        \
  case (GL_ ## x): {                                  \
    static const string str = TextUtils::tolower(#x); \
    return str.c_str();                               \
	}

	switch (type) {
		UNIFORM_STRING_CASE(FLOAT)
		UNIFORM_STRING_CASE(FLOAT_VEC2)
		UNIFORM_STRING_CASE(FLOAT_VEC3)
		UNIFORM_STRING_CASE(FLOAT_VEC4)
		UNIFORM_STRING_CASE(FLOAT_MAT2)
		UNIFORM_STRING_CASE(FLOAT_MAT3)
		UNIFORM_STRING_CASE(FLOAT_MAT4)
		UNIFORM_STRING_CASE(SAMPLER_1D)
		UNIFORM_STRING_CASE(SAMPLER_2D)
		UNIFORM_STRING_CASE(SAMPLER_3D)
		UNIFORM_STRING_CASE(SAMPLER_CUBE)
		UNIFORM_STRING_CASE(SAMPLER_1D_SHADOW)
		UNIFORM_STRING_CASE(SAMPLER_2D_SHADOW)
		UNIFORM_STRING_CASE(INT)
		UNIFORM_STRING_CASE(INT_VEC2)
		UNIFORM_STRING_CASE(INT_VEC3)
		UNIFORM_STRING_CASE(INT_VEC4)
		UNIFORM_STRING_CASE(BOOL)
		UNIFORM_STRING_CASE(BOOL_VEC2)
		UNIFORM_STRING_CASE(BOOL_VEC3)
		UNIFORM_STRING_CASE(BOOL_VEC4)
	  default: {
	  	return "unknown_type";
		}
	}
}


int LuaShaderMgr::GetActiveUniforms(lua_State* L)
{
	LuaShader* shader = CheckLuaShader(L, 1);
	if ((shader == NULL) || !shader->IsValid()) {
		return 0;
	}
	const GLuint progID = shader->GetProgID();
	
	GLint uniformCount;
	glGetProgramiv(progID, GL_ACTIVE_UNIFORMS, &uniformCount);

	lua_newtable(L);

	for (GLint i = 0; i < uniformCount; i++) {
		GLsizei length;
		GLint size;
		GLenum type;
		GLchar name[1024];
		glGetActiveUniform(progID, i, sizeof(name), &length, &size, &type, name);

		lua_pushnumber(L, i + 1);
		lua_newtable(L); {
			HSTR_PUSH_STRING(L, "name",   name);
			HSTR_PUSH_STRING(L, "type",   UniformTypeString(type));
			HSTR_PUSH_NUMBER(L, "length", length);
			HSTR_PUSH_NUMBER(L, "size",   size);
		}
		lua_rawset(L, -3);
	}
	
	return 1;
}


int LuaShaderMgr::GetUniformLocation(lua_State* L)
{
	LuaShader* shader = CheckLuaShader(L, 1);
	if ((shader == NULL) || !shader->IsValid()) {
		return 0;
	}
	const string name = luaL_checkstring(L, 2);
	const GLuint progID = shader->GetProgID();
	const GLint location = glGetUniformLocation(progID, name.c_str());
	lua_pushnumber(L, location);
	return 1;
}


/******************************************************************************/
/******************************************************************************/

int LuaShaderMgr::Uniform(lua_State* L)
{
	if (activeShaderDepth <= 0) {
		LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);
	}
	const GLuint location = (GLuint)luaL_checknumber(L, 1);

	const int values = lua_gettop(L) - 1;
	switch (values) {
		case 1: {
			glUniform1f(location,
									luaL_checkfloat(L, 2));
			break;
		}
		case 2: {
			glUniform2f(location,
									luaL_checkfloat(L, 2),
									luaL_checkfloat(L, 3));
			break;
		}
		case 3: {
			glUniform3f(location,
									luaL_checkfloat(L, 2),
									luaL_checkfloat(L, 3),
									luaL_checkfloat(L, 4));
			break;
		}
		case 4: {
			glUniform4f(location,
									luaL_checkfloat(L, 2),
									luaL_checkfloat(L, 3),
									luaL_checkfloat(L, 4),
									luaL_checkfloat(L, 5));
			break;
		}
		default: {
			luaL_error(L, "Incorrect arguments to gl.Uniform()");
		}
	}
	return 0;
}


int LuaShaderMgr::UniformInt(lua_State* L)
{
	if (activeShaderDepth <= 0) {
		LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);
	}
	const GLuint location = (GLuint)luaL_checknumber(L, 1);

	const int values = lua_gettop(L) - 1;
	switch (values) {
		case 1: {
			glUniform1i(location,
									luaL_checkint(L, 2));
			break;
		}
		case 2: {
			glUniform2i(location,
									luaL_checkint(L, 2),
									luaL_checkint(L, 3));
			break;
		}
		case 3: {
			glUniform3i(location,
									luaL_checkint(L, 2),
									luaL_checkint(L, 3),
									luaL_checkint(L, 4));
			break;
		}
		case 4: {
			glUniform4i(location,
									luaL_checkint(L, 2),
									luaL_checkint(L, 3),
									luaL_checkint(L, 4),
									luaL_checkint(L, 5));
			break;
		}
		default: {
			luaL_error(L, "Incorrect arguments to gl.UniformInt()");
		}
	}
	return 0;
}


/*
static void UniformMatrix4dv(GLint location, const double dm[16]) // FIXME
{
	float fm[16];
	for (int i = 0; i < 16; i++) {
		fm[i] = (float)dm[i];
	}
	glUniformMatrix4fv(location, 1, GL_FALSE, fm);
}
*/


int LuaShaderMgr::UniformMatrix(lua_State* L)
{
	if (activeShaderDepth <= 0) {
		LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);
	}
	const GLuint location = (GLuint)luaL_checknumber(L, 1);

	const int values = lua_gettop(L) - 1;
	switch (values) {
		case 1: {
			if (!lua_israwstring(L, 2)) {
				luaL_error(L, "Incorrect arguments to gl.UniformMatrix()");
			}
			const string matName = lua_tostring(L, 2);
			if (matName == "shadow") {
//???				if (shadowHandler) {
//???
//					glUniformMatrix4fv(location, 1, GL_FALSE,
//					                   shadowHandler->shadowMatrix.m);
//???				}
			}
			else if (matName == "camera") {
				const ViewFrustum& vf = RENDERER.getViewFrustum();
				glUniformMatrix4fv(location, 1, GL_FALSE, vf.getViewMatrix());
			}
			else if (matName == "camprj") {
				const ViewFrustum& vf = RENDERER.getViewFrustum();
				glUniformMatrix4fv(location, 1, GL_FALSE, vf.getProjectionMatrix());
			}
			else if (matName == "caminv") {
//???				UniformMatrix4dv(location, camera->modelviewInverse);
			}
			else {
				luaL_error(L, "Incorrect arguments to gl.UniformMatrix()");
			}
			break;
		}
		case (2 * 2): {
			float array[2 * 2];
			for (int i = 0; i < (2 * 2); i++) {
				array[i] = luaL_checkfloat(L, i + 2);
			}
			glUniformMatrix2fv(location, 1, GL_FALSE, array);
			break;
		}
		case (3 * 3): {
			float array[3 * 3];
			for (int i = 0; i < (3 * 3); i++) {
				array[i] = luaL_checkfloat(L, i + 2);
			}
			glUniformMatrix3fv(location, 1, GL_FALSE, array);
			break;
		}
		case (4 * 4): {
			float array[4 * 4];
			for (int i = 0; i < (4 * 4); i++) {
				array[i] = luaL_checkfloat(L, i + 2);
			}
			glUniformMatrix4fv(location, 1, GL_FALSE, array);
			break;
		}
		default: {
			luaL_error(L, "Incorrect arguments to gl.UniformMatrix()");
		}
	}
	return 0;
}


int LuaShaderMgr::SetShaderParameter(lua_State* L)
{
	if (activeShaderDepth <= 0) {
		LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);
	}
	LuaShader* shader = CheckLuaShader(L, 1);
	if ((shader == NULL) || !shader->IsValid()) {
		return 0;
	}

	const GLenum param = (GLenum)luaL_checkint(L, 2);
	const GLint  value =  (GLint)luaL_checkint(L, 3);

	if (glProgramParameteriEXT) {
		glProgramParameteriEXT(shader->GetProgID(), param, value);
	}

	return 0;
}


/******************************************************************************/
/******************************************************************************/

bool LuaShaderMgr::InsertShader(LuaShader* shader)
{
	if (shaders.find(shader) != shaders.end()) {
		return false;
	}
	shaders.insert(shader);
	return true;
}


bool LuaShaderMgr::RemoveShader(LuaShader* shader)
{
	set<LuaShader*>::iterator it = shaders.find(shader);
	if (it == shaders.end()) {
		return false;
	}
	shaders.erase(it);
	return true;
}


void LuaShaderMgr::InitContext()
{
}


void LuaShaderMgr::FreeContext()
{
}


void LuaShaderMgr::StaticInitContext(void* data)
{
	((LuaShaderMgr*)data)->InitContext();
}


void LuaShaderMgr::StaticFreeContext(void* data)
{
	((LuaShaderMgr*)data)->FreeContext();
}


/******************************************************************************/
/******************************************************************************/
