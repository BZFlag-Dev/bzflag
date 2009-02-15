
#include "common.h"

// interface header
#include "LuaTexMat.h"

// system headers
#include <string>
using std::string;

// common headers
#include "TextureMatrix.h"

// local headers
#include "LuaInclude.h"
#include "LuaHashString.h"


/******************************************************************************/
/******************************************************************************/

bool LuaTexMat::PushEntries(lua_State* L)
{
	PUSH_LUA_CFUNC(L, GetTexMatName);
	PUSH_LUA_CFUNC(L, GetTexMat);
	PUSH_LUA_CFUNC(L, SetTexMat);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static inline const TextureMatrix* ParseTexMat(lua_State* L, int index)
{
	const int texmatIndex = luaL_checkint(L, index);
	return TEXMATRIXMGR.getMatrix(texmatIndex);
}


/******************************************************************************/
/******************************************************************************/

int LuaTexMat::GetTexMatName(lua_State* L)
{
	const TextureMatrix* texmat = ParseTexMat(L, 1);
	if (texmat == NULL) {
		return 0;
	}
	lua_pushstdstring(L, texmat->getName());
	return 1;
}


int LuaTexMat::GetTexMat(lua_State* L)
{
	const TextureMatrix* texmat = ParseTexMat(L, 1);
	if (texmat == NULL) {
		return 0;
	}
	const float* matrix = texmat->getMatrix();
	for (int i = 0; i < 16; i++) {
		lua_pushnumber(L, matrix[i]);
	}
	return 16;
}


int LuaTexMat::SetTexMat(lua_State* L)
{
	TextureMatrix* texmat = const_cast<TextureMatrix*>(ParseTexMat(L, 1));
	if (texmat == NULL) {
		return 0;
	}
	const float* oldMatrix = texmat->getMatrix();
	float newMatrix[4][4];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			const int k = (i * 4) + j;
			newMatrix[i][j] = luaL_optfloat(L, k + 2, oldMatrix[k]);
		}
	}
	texmat->setMatrix(newMatrix);
	lua_pushboolean(L, true);
	return 1;
}


/******************************************************************************/
/******************************************************************************/
