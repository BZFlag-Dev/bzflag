
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
	PUSH_LUA_CFUNC(L, GetBzMatName);
	PUSH_LUA_CFUNC(L, GetBzMatOrder);
	PUSH_LUA_CFUNC(L, GetBzMatDynCol);
	PUSH_LUA_CFUNC(L, GetBzMatAmbient);
	PUSH_LUA_CFUNC(L, GetBzMatDiffuse);
	PUSH_LUA_CFUNC(L, GetBzMatSpecular);
	PUSH_LUA_CFUNC(L, GetBzMatEmission);
	PUSH_LUA_CFUNC(L, GetBzMatShininess);
	PUSH_LUA_CFUNC(L, GetBzMatOccluder);
	PUSH_LUA_CFUNC(L, GetBzMatGroupAlpha);
	PUSH_LUA_CFUNC(L, GetBzMatNoLighting);
	PUSH_LUA_CFUNC(L, GetBzMatNoRadar);
	PUSH_LUA_CFUNC(L, GetBzMatNoShadow);
	PUSH_LUA_CFUNC(L, GetBzMatNoCulling);
	PUSH_LUA_CFUNC(L, GetBzMatNoSorting);
	PUSH_LUA_CFUNC(L, GetBzMatAlphaThresh);
	PUSH_LUA_CFUNC(L, GetBzMatTexture);
	PUSH_LUA_CFUNC(L, GetBzMatTextureInfo);
	PUSH_LUA_CFUNC(L, GetBzMatShader);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static const BzMaterial* ParseBzMat(lua_State* L, int index)
{
	const int matIndex = luaL_checkint(L, index);
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

int LuaBzMaterial::GetBzMatName(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	lua_pushstdstring(L, mat->getName());
	return 1;
}


int LuaBzMaterial::GetBzMatOrder(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	lua_pushinteger(L, mat->getOrder());
	return 1;
}


int LuaBzMaterial::GetBzMatDynCol(lua_State* L)
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


int LuaBzMaterial::GetBzMatAmbient(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	PushColor(L, mat->getAmbient());
	return 4;
}


int LuaBzMaterial::GetBzMatDiffuse(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	PushColor(L, mat->getDiffuse());
	return 4;
}


int LuaBzMaterial::GetBzMatSpecular(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	PushColor(L, mat->getSpecular());
	return 4;
}


int LuaBzMaterial::GetBzMatEmission(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	PushColor(L, mat->getEmission());
	return 4;
}


int LuaBzMaterial::GetBzMatShininess(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	lua_pushnumber(L, mat->getShininess());
	return 1;
}


int LuaBzMaterial::GetBzMatOccluder(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	lua_pushboolean(L, mat->getOccluder());
	return 1;
}


int LuaBzMaterial::GetBzMatGroupAlpha(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	lua_pushboolean(L, mat->getGroupAlpha());
	return 1;
}


int LuaBzMaterial::GetBzMatNoLighting(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	lua_pushboolean(L, mat->getNoLighting());
	return 1;
}


int LuaBzMaterial::GetBzMatNoRadar(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	lua_pushboolean(L, mat->getNoRadar());
	return 1;
}


int LuaBzMaterial::GetBzMatNoShadow(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	lua_pushboolean(L, mat->getNoShadow());
	return 1;
}


int LuaBzMaterial::GetBzMatNoCulling(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	lua_pushboolean(L, mat->getNoCulling());
	return 1;
}


int LuaBzMaterial::GetBzMatNoSorting(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	lua_pushboolean(L, mat->getNoSorting());
	return 1;
}


int LuaBzMaterial::GetBzMatAlphaThresh(lua_State* L)
{
	const BzMaterial* mat = ParseBzMat(L, 1);
	if (mat == NULL) {
		return 0;
	}
	lua_pushnumber(L, mat->getAlphaThreshold());
	return 1;
}


int LuaBzMaterial::GetBzMatTexture(lua_State* L)
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


int LuaBzMaterial::GetBzMatTextureInfo(lua_State* L)
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


int LuaBzMaterial::GetBzMatShader(lua_State* L)
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
