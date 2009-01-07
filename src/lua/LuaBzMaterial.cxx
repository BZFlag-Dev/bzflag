
#include "common.h"

// implementation header
#include "LuaBzMaterial.h"

// system headers
#include <string>
using std::string;

// common headers
#include "BzMaterial.h"
#include "DynamicColor.h"
#include "TextureMatrix.h"

// local headers
#include "LuaInclude.h"
#include "LuaUtils.h"
#include "LuaHashString.h"


/******************************************************************************/
/******************************************************************************/

bool LuaBzMaterial::PushEntries(lua_State* L)
{
	PUSH_LUA_CFUNC(L, FindMaterial);

	PUSH_LUA_CFUNC(L, GetMaterialCount);

	PUSH_LUA_CFUNC(L, GetMaterialName);
	PUSH_LUA_CFUNC(L, GetMaterialOrder);

	PUSH_LUA_CFUNC(L, GetMaterialAmbient);
	PUSH_LUA_CFUNC(L, GetMaterialDiffuse);
	PUSH_LUA_CFUNC(L, GetMaterialSpecular);
	PUSH_LUA_CFUNC(L, GetMaterialEmission);
	PUSH_LUA_CFUNC(L, GetMaterialShininess);

	PUSH_LUA_CFUNC(L, GetMaterialDynCol);
	PUSH_LUA_CFUNC(L, GetMaterialOccluder);
	PUSH_LUA_CFUNC(L, GetMaterialGroupAlpha);
	PUSH_LUA_CFUNC(L, GetMaterialNoLighting);
	PUSH_LUA_CFUNC(L, GetMaterialNoRadar);
	PUSH_LUA_CFUNC(L, GetMaterialNoShadow);
	PUSH_LUA_CFUNC(L, GetMaterialNoCulling);
	PUSH_LUA_CFUNC(L, GetMaterialNoSorting);
	PUSH_LUA_CFUNC(L, GetMaterialAlphaThresh);
	PUSH_LUA_CFUNC(L, GetMaterialTexture);
	PUSH_LUA_CFUNC(L, GetMaterialTextureInfo);
	PUSH_LUA_CFUNC(L, GetMaterialShader);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static const BzMaterial* ParseBzMat(lua_State* L, int index)
{
	const int matIndex = luaL_checkint(L, index) - 1;
	return MATERIALMGR.getMaterial(matIndex);
}


static void PushColor(lua_State* L, const float* color)
{
	lua_pushnumber(L, color[0]);
	lua_pushnumber(L, color[1]);
	lua_pushnumber(L, color[2]);
	lua_pushnumber(L, color[3]);
}


/******************************************************************************/
/******************************************************************************/

int LuaBzMaterial::FindMaterial(lua_State* L)
{
	const BzMaterial* mat = NULL;
	if (lua_israwnumber(L, 1)) {
		const int matID = lua_toint(L, 1) - 1;
		mat = MATERIALMGR.getMaterial(matID);
	}
	else if (lua_israwstring(L, 1)) {
		const string matName = lua_tostring(L, 1);
		mat = MATERIALMGR.findMaterial(matName);
	}
	else {
		return 0;
	}

	if (mat == NULL) {
		return 0;
	}

	lua_pushinteger(L, mat->getID() + 1);

	return 1;
}


int LuaBzMaterial::GetMaterialCount(lua_State* L)
{
	lua_pushinteger(L, MATERIALMGR.getCount());
	return 1;
}


/******************************************************************************/

#define MAT_PARAMETER_FUNC(name, getFunc, pushFunc, count) \
	int LuaBzMaterial::GetMaterial ## name(lua_State* L)     \
	{                                                        \
		const BzMaterial* mat = ParseBzMat(L, 1);              \
		if (mat == NULL) {                                     \
			return 0;                                            \
		}                                                      \
		pushFunc(L, mat->getFunc());                           \
		return count;                                          \
	}

MAT_PARAMETER_FUNC(Name,        getName,           lua_pushstdstring, 1)
MAT_PARAMETER_FUNC(Order,       getOrder,          lua_pushinteger,   1)
MAT_PARAMETER_FUNC(Ambient,     getAmbient,        PushColor,         4)
MAT_PARAMETER_FUNC(Diffuse,     getDiffuse,        PushColor,         4)
MAT_PARAMETER_FUNC(Emission,    getEmission,       PushColor,         4)
MAT_PARAMETER_FUNC(Specular,    getSpecular,       PushColor,         4)
MAT_PARAMETER_FUNC(Shininess,   getShininess,      lua_pushnumber,    1)
MAT_PARAMETER_FUNC(AlphaThresh, getAlphaThreshold, lua_pushnumber,    1)
MAT_PARAMETER_FUNC(Occluder,    getOccluder,       lua_pushboolean,   1)
MAT_PARAMETER_FUNC(GroupAlpha,  getGroupAlpha,     lua_pushboolean,   1)
MAT_PARAMETER_FUNC(NoLighting,  getNoLighting,     lua_pushboolean,   1)
MAT_PARAMETER_FUNC(NoRadar,     getNoRadar,        lua_pushboolean,   1)
MAT_PARAMETER_FUNC(NoShadow,    getNoShadow,       lua_pushboolean,   1)
MAT_PARAMETER_FUNC(NoCulling,   getNoCulling,      lua_pushboolean,   1)
MAT_PARAMETER_FUNC(NoSorting,   getNoSorting,      lua_pushboolean,   1)


/******************************************************************************/

int LuaBzMaterial::GetMaterialDynCol(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	const int dyncol = mat->getDynamicColor();
	if (dyncol >= 0) {
		lua_pushinteger(L, dyncol);
	} else {
		lua_pushboolean(L, false);
	}
	return 1;
}


int LuaBzMaterial::GetMaterialTexture(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	const int index = luaL_optint(L, 2, 1) - 1;
	if ((index < 0) || (index >= mat->getTextureCount())) {
		return 0;
	}
	lua_pushstdstring(L, mat->getTexture(index));
	return 1;
}


int LuaBzMaterial::GetMaterialTextureInfo(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	const int index = luaL_optint(L, 2, 1) - 1;
	if ((index < 0) || (index >= mat->getTextureCount())) {
		return 0;
	}
	lua_newtable(L);
	HSTR_PUSH_STRING(L, "name",      mat->getTexture(index));
	HSTR_PUSH_STRING(L, "local",     mat->getTexture(index));
	HSTR_PUSH_INT(L,    "envMode",   mat->getCombineMode(index));
	HSTR_PUSH_BOOL(L,   "useAlpha",  mat->getUseTextureAlpha(index));
	HSTR_PUSH_BOOL(L,   "useColor",  mat->getUseColorOnTexture(index));
	HSTR_PUSH_BOOL(L,   "sphereMap", mat->getUseSphereMap(index));
	const int texmat = mat->getTextureMatrix(index);
	if (texmat >= 0) {
		HSTR_PUSH_INT(L,  "texmat",    texmat);
	}
	return 1;
}


int LuaBzMaterial::GetMaterialShader(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	const int index = luaL_optint(L, 2, 1) - 1;
	if ((index < 0) || (index >= mat->getShaderCount())) {
		return 0;
	}
	lua_pushstdstring(L, mat->getShader(index));
	return 1;
}


/******************************************************************************/
/******************************************************************************/
