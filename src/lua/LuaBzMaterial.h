#ifndef LUA_BZMATERIAL_H
#define LUA_BZMATERIAL_H

struct lua_State;


class LuaBzMaterial {
	public:
		static bool PushEntries(lua_State* L);

	private: // call-outs
		static int GetBzMatName(lua_State* L);
		static int GetBzMatOrder(lua_State* L);
		static int GetBzMatDynCol(lua_State* L);
		static int GetBzMatAmbient(lua_State* L);
		static int GetBzMatDiffuse(lua_State* L);
		static int GetBzMatSpecular(lua_State* L);
		static int GetBzMatEmission(lua_State* L);
		static int GetBzMatShininess(lua_State* L);
		static int GetBzMatOccluder(lua_State* L);
		static int GetBzMatGroupAlpha(lua_State* L);
		static int GetBzMatNoLighting(lua_State* L);
		static int GetBzMatNoRadar(lua_State* L);
		static int GetBzMatNoShadow(lua_State* L);
		static int GetBzMatNoCulling(lua_State* L);
		static int GetBzMatNoSorting(lua_State* L);
		static int GetBzMatAlphaThresh(lua_State* L);
		static int GetBzMatTexture(lua_State* L);
		static int GetBzMatTextureInfo(lua_State* L);
		static int GetBzMatShader(lua_State* L);
};


#endif // LUA_BZMATERIAL_H
