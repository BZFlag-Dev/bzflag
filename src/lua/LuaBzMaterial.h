#ifndef LUA_BZMATERIAL_H
#define LUA_BZMATERIAL_H

struct lua_State;


class LuaBzMaterial {
	public:
		static bool PushEntries(lua_State* L);

	private: // call-outs
		static int FindMaterial(lua_State* L);

		static int GetMaterialCount(lua_State* L);

		static int GetMaterialName(lua_State* L);
		static int GetMaterialOrder(lua_State* L);

		static int GetMaterialAmbient(lua_State* L);
		static int GetMaterialDiffuse(lua_State* L);
		static int GetMaterialSpecular(lua_State* L);
		static int GetMaterialEmission(lua_State* L);
		static int GetMaterialShininess(lua_State* L);

		static int GetMaterialDynCol(lua_State* L);
		static int GetMaterialOccluder(lua_State* L);
		static int GetMaterialGroupAlpha(lua_State* L);
		static int GetMaterialNoLighting(lua_State* L);
		static int GetMaterialNoRadar(lua_State* L);
		static int GetMaterialNoShadow(lua_State* L);
		static int GetMaterialNoCulling(lua_State* L);
		static int GetMaterialNoSorting(lua_State* L);
		static int GetMaterialAlphaThresh(lua_State* L);

		static int GetMaterialTextureCount(lua_State* L);
		static int GetMaterialTexture(lua_State* L);
		static int GetMaterialTextureInfo(lua_State* L);

		static int GetMaterialShaderCount(lua_State* L);
		static int GetMaterialShader(lua_State* L);
};


#endif // LUA_BZMATERIAL_H
