
#include "common.h"

// implementation header
#include "LuaObstacle.h"

// system headers
#include <string>
using std::string;

// common headers
#include "Obstacle.h"
#include "ObstacleMgr.h"
#include "WallObstacle.h"
#include "BoxBuilding.h"
#include "PyramidBuilding.h"
#include "BaseBuilding.h"
#include "Teleporter.h"
#include "MeshObstacle.h"
#include "ArcObstacle.h"
#include "ConeObstacle.h"
#include "SphereObstacle.h"
#include "TetraBuilding.h"
#include "MeshDrawInfo.h"
#include "MeshSceneNodeGenerator.h" // FIXME -- not being used, code was copied

// bzflag headers
#include "../bzflag/World.h"

// local headers
#include "LuaInclude.h"
#include "LuaUtils.h"
#include "LuaHashString.h"


/******************************************************************************/
/******************************************************************************/

bool LuaObstacle::PushEntries(lua_State* L)
{
	PUSH_LUA_CFUNC(L, GetObstacleList);
	PUSH_LUA_CFUNC(L, GetObstacleName);
	PUSH_LUA_CFUNC(L, GetObstacleType);

	PUSH_LUA_CFUNC(L, GetObstacleDriveThrough);
	PUSH_LUA_CFUNC(L, GetObstacleShootThrough);
	PUSH_LUA_CFUNC(L, GetObstacleRicochet);

	PUSH_LUA_CFUNC(L, GetObstaclePosition);
	PUSH_LUA_CFUNC(L, GetObstacleSize);
	PUSH_LUA_CFUNC(L, GetObstacleRotation);

	PUSH_LUA_CFUNC(L, GetObstacleFlatTop);
	PUSH_LUA_CFUNC(L, GetObstacleExtents);
	PUSH_LUA_CFUNC(L, GetObstacleTeam);
	PUSH_LUA_CFUNC(L, GetObstacleFlipZ);
	PUSH_LUA_CFUNC(L, GetObstacleBorder);

	PUSH_LUA_CFUNC(L, GetObstacleFaceCount);
	PUSH_LUA_CFUNC(L, GetFaceMesh);
	PUSH_LUA_CFUNC(L, GetFaceElementCount);
	PUSH_LUA_CFUNC(L, GetFaceVerts);
	PUSH_LUA_CFUNC(L, GetFaceNorms);
	PUSH_LUA_CFUNC(L, GetFaceTxcds);
	PUSH_LUA_CFUNC(L, GetFacePlane);
	PUSH_LUA_CFUNC(L, GetFacePhyDrv);
	PUSH_LUA_CFUNC(L, GetFaceMaterial);
	PUSH_LUA_CFUNC(L, GetFaceSmoothBounce);

	PUSH_LUA_CFUNC(L, GetTeleByName);
	PUSH_LUA_CFUNC(L, GetTeleBorder);
	PUSH_LUA_CFUNC(L, GetTeleLinks);
	PUSH_LUA_CFUNC(L, GetLinkDestinations);

	PUSH_LUA_CFUNC(L, HasMeshDrawInfo);
	PUSH_LUA_CFUNC(L, GetMeshDrawInfo);

	return true;
}


/******************************************************************************/
/******************************************************************************/

// FIXME -- copied from MeshSceneNodeGenerator
//       -- doesn't want to link

static bool makeTexcoords(const float* plane,
					                const GLfloat3Array& vertices,
					                GLfloat2Array& texcoords)
{
	float x[3], y[3];

	vec3sub (x, vertices[1], vertices[0]);
	vec3cross (y, plane, x);

	float len = vec3dot(x, x);
	if (len > 0.0f) {
		len = 1.0f / sqrtf(len);
		x[0] = x[0] * len;
		x[1] = x[1] * len;
		x[2] = x[2] * len;
	} else {
		return false;
	}

	len = vec3dot(y, y);
	if (len > 0.0f) {
		len = 1.0f / sqrtf(len);
		y[0] = y[0] * len;
		y[1] = y[1] * len;
		y[2] = y[2] * len;
	} else {
		return false;
	}

	const float uvScale = 8.0f;

	texcoords[0][0] = 0.0f;
	texcoords[0][1] = 0.0f;
	const int count = vertices.getSize();
	for (int i = 1; i < count; i++) {
		float delta[3];
		vec3sub (delta, vertices[i], vertices[0]);
		texcoords[i][0] = vec3dot(delta, x) / uvScale;
		texcoords[i][1] = vec3dot(delta, y) / uvScale;
	}

	return true;
}


/******************************************************************************/
/******************************************************************************/

static int GetTypeFromName(const string& name)
{
	     if (name == "wall")   { return wallType;   }
	else if (name == "box")    { return boxType;    }
	else if (name == "pyr")    { return pyrType;    }
	else if (name == "base")   { return baseType;   }
	else if (name == "tele")   { return teleType;   }
	else if (name == "mesh")   { return meshType;   }
	else if (name == "arc")    { return arcType;    }
	else if (name == "cone")   { return coneType;   }
	else if (name == "sphere") { return sphereType; }
	else if (name == "tetra")  { return tetraType;  }
	return -1;
}


/******************************************************************************/
/******************************************************************************/
//
//  Utility routines
//

static inline bool IsMeshFace(const Obstacle* obs)
{
	return (obs->getType() == MeshFace::getClassName());
}


static inline bool IsMeshObstacle(const Obstacle* obs)
{
	return (obs->getType() == MeshObstacle::getClassName());
}


static inline const Obstacle* ParsePrimaryObstacle(lua_State* L, int index)
{
	const unsigned int obsID = luaL_checkint(L, index);
	return OBSTACLEMGR.getObstacleFromID(obsID);
}


static inline const MeshFace* GetFace(const Obstacle* obs, int faceIndex)
{
	if (obs->getTypeID() == teleType) {
		const Teleporter* tele = (const Teleporter*)obs;
		switch (faceIndex) {
			case 0:  { return tele->getFrontLink(); }
			case 1:  { return tele->getBackLink(); }
		}
		return NULL;
	}
	else if (obs->getType() == MeshObstacle::getClassName()) {
		const MeshObstacle* mesh = (const MeshObstacle*)obs;
		if ((faceIndex < 0) || (faceIndex >= mesh->getFaceCount())) {
			return NULL;
		}
		return mesh->getFace(faceIndex);
	}
	return NULL;
}


static inline const Obstacle* ParseObstacle(lua_State* L, int index)
{
	const Obstacle* obs = ParsePrimaryObstacle(L, index);
	if (obs == NULL) {
		return NULL;
	}
	if (!lua_israwnumber(L, index + 1)) {
		return obs;
	}
	const int faceIndex = lua_toint(L, index + 1) - 1;
	return (const Obstacle*)GetFace(obs, faceIndex);
}


static inline const MeshFace* ParseMeshFace(lua_State* L, int index)
{
	const Obstacle* obs = ParsePrimaryObstacle(L, index);
	if (obs == NULL) {
		return NULL;
	}
	const int faceIndex = luaL_checkint(L, index + 1) - 1;
	return GetFace(obs, faceIndex);
}


static bool PushObstacleList(lua_State* L, int type, int& index)
{
	const GroupDefinition* world = OBSTACLEMGR.getWorld();
	if (world == NULL) {
		return false;
	}
	if ((type < 0) || (type >= ObstacleTypeCount)) {
		return false;
	}
	const ObstacleList& obsList = world->getList(type);
	const size_t count = obsList.size();
	for (size_t i = 0; i < count; i++) {
		index++;
		lua_pushinteger(L, index);
		lua_pushinteger(L, obsList[i]->getGUID());
		lua_rawset(L, -3);
	}

	return true;
}


/******************************************************************************/
/******************************************************************************/

int LuaObstacle::GetObstacleList(lua_State* L)
{
	lua_settop(L, 1);
	lua_newtable(L);
	const int table = 1;

	if (!lua_istable(L, table)) {
		int index = 0;
		for (int type = 0; type < ObstacleTypeCount; type++) {
			PushObstacleList(L, type, index);
		}
	}
	else {
		lua_newtable(L);
		const int newTable = lua_gettop(L);
		for (int i = 1; lua_checkgeti(L, table, i); lua_pop(L, 1), i++) {
			int type = -1;
			if (lua_israwnumber(L, -1)) {
				type = lua_toint(L, -1);
			}
			else if (lua_israwstring(L, -1)) {
				type = GetTypeFromName(lua_tostring(L, -1));
			}
			if (type >= 0) {
				lua_pushinteger(L, type);
				lua_newtable(L);
				int index = 0;
				PushObstacleList(L, type, index);
				lua_rawset(L, newTable);
			}
		}
	}

	return 1;
}


int LuaObstacle::GetObstacleName(lua_State* L)
{
	const Obstacle* obs = ParseObstacle(L, 1);
	if (obs == NULL) {
		return 0;
	}
	lua_pushstring(L, obs->getName());
	return 1;
}


int LuaObstacle::GetObstacleType(lua_State* L)
{
	const Obstacle* obs = ParsePrimaryObstacle(L, 1);
	if (obs == NULL) {
		return 0;
	}
	lua_pushinteger(L, obs->getTypeID());
	return 1;
}


int LuaObstacle::GetObstacleDriveThrough(lua_State* L)
{
	const Obstacle* obs = ParseObstacle(L, 1);
	if (obs == NULL) {
		return 0;
	}
	lua_pushboolean(L, obs->isDriveThrough());
	return 1;
}


int LuaObstacle::GetObstacleShootThrough(lua_State* L)
{
	const Obstacle* obs = ParseObstacle(L, 1);
	if (obs == NULL) {
		return 0;
	}
	lua_pushboolean(L, obs->isShootThrough());
	return 1;
}


int LuaObstacle::GetObstacleRicochet(lua_State* L)
{
	const Obstacle* obs = ParseObstacle(L, 1);
	if (obs == NULL) {
		return 0;
	}
	lua_pushboolean(L, obs->canRicochet());
	return 1;
}


int LuaObstacle::GetObstaclePosition(lua_State* L)
{
	const Obstacle* obs = ParseObstacle(L, 1);
	if (obs == NULL) {
		return 0;
	}
	lua_pushnumber(L, obs->getPosition()[0]);
	lua_pushnumber(L, obs->getPosition()[1]);
	lua_pushnumber(L, obs->getPosition()[2]);
	return 3;
}


int LuaObstacle::GetObstacleSize(lua_State* L)
{
	const Obstacle* obs = ParseObstacle(L, 1);
	if (obs == NULL) {
		return 0;
	}
	lua_pushnumber(L, obs->getSize()[0]);
	lua_pushnumber(L, obs->getSize()[1]);
	lua_pushnumber(L, obs->getSize()[2]);
	return 3;
}


int LuaObstacle::GetObstacleRotation(lua_State* L)
{
	const Obstacle* obs = ParseObstacle(L, 1);
	if (obs == NULL) {
		return 0;
	}
	lua_pushnumber(L, obs->getRotation());
	return 1;
}




int LuaObstacle::GetObstacleFlatTop(lua_State* L)
{
	const Obstacle* obs = ParseObstacle(L, 1);
	if (obs == NULL) {
		return 0;
	}
	lua_pushboolean(L, obs->isFlatTop());
	return 1;
}


int LuaObstacle::GetObstacleExtents(lua_State* L)
{
	const Obstacle* obs = ParseObstacle(L, 1);
	if (obs == NULL) {
		return 0;
	}
	const Extents& exts = obs->getExtents();
	lua_pushnumber(L, exts.mins[0]);
	lua_pushnumber(L, exts.mins[1]);
	lua_pushnumber(L, exts.mins[2]);
	lua_pushnumber(L, exts.maxs[0]);
	lua_pushnumber(L, exts.maxs[1]);
	lua_pushnumber(L, exts.maxs[2]);
	return 6;
}


int LuaObstacle::GetObstacleTeam(lua_State* L)
{
	const Obstacle* obs = ParseObstacle(L, 1);
	if (obs == NULL) {
		return 0;
	}
	if (obs->getTypeID() != baseType) {
		return 0;
	}
	const BaseBuilding* base = (BaseBuilding*)obs;	
	lua_pushinteger(L, base->getTeam());
	return 1;
}


int LuaObstacle::GetObstacleFlipZ(lua_State* L)
{
	const Obstacle* obs = ParseObstacle(L, 1);
	if (obs == NULL) {
		return 0;
	}
	lua_pushboolean(L, obs->getZFlip());
	return 1;
}


int LuaObstacle::GetObstacleBorder(lua_State* L)
{
	const Obstacle* obs = ParseObstacle(L, 1);
	if (obs == NULL) {
		return 0;
	}
	if (obs->getTypeID() != teleType) {
		return 0;
	}
	const Teleporter* tele = (Teleporter*)obs;	
	lua_pushnumber(L, tele->getBorder());
	return 1;
}


/******************************************************************************/
/******************************************************************************/

int LuaObstacle::GetObstacleFaceCount(lua_State* L)
{
	const Obstacle* obs = ParseObstacle(L, 1);

	if (obs->getTypeID() == teleType) {
		lua_pushinteger(L, 2);
		return 1;
	}

	if (!IsMeshObstacle(obs)) {
		return 0;
	}

	const MeshObstacle* mesh = (const MeshObstacle*)obs;
	lua_pushinteger(L, mesh->getFaceCount());		

	return 1;
}


int LuaObstacle::GetFaceMesh(lua_State* L)
{
	const MeshFace* face = ParseMeshFace(L, 1);
	if (face == NULL) {
		return 0;
	}
	const MeshObstacle* mesh = face->getMesh();
	if (mesh == NULL) {
		lua_pushboolean(L, false);
		return 1;
	}
	lua_pushinteger(L, mesh->getGUID());
	return 1;
}


int LuaObstacle::GetFaceElementCount(lua_State* L)
{
	const MeshFace* face = ParseMeshFace(L, 1);
	if (face == NULL) {
		return 0;
	}
	const int elements = face->getVertexCount();
	lua_pushinteger(L, elements);
	return 1;
}


int LuaObstacle::GetFaceVerts(lua_State* L)
{
	const MeshFace* face = ParseMeshFace(L, 1);
	if (face == NULL) {
		return 0;
	}
	const int elements = face->getVertexCount();
	lua_createtable(L, elements, 0);
	for (int i = 0; i < elements; i++) {
		const float* vec = face->getVertex(i);
		lua_createtable(L, 3, 0);
		lua_pushnumber(L, vec[0]); lua_rawseti(L, -2, 1);
		lua_pushnumber(L, vec[1]); lua_rawseti(L, -2, 2);
		lua_pushnumber(L, vec[2]); lua_rawseti(L, -2, 3);
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}


int LuaObstacle::GetFaceNorms(lua_State* L)
{
	const MeshFace* face = ParseMeshFace(L, 1);
	if (face == NULL) {
		return 0;
	}
	if (!face->useNormals()) {
		return 0;
	}
	const int elements = face->getVertexCount();
	lua_createtable(L, elements, 0);
	for (int i = 0; i < elements; i++) {
		const float* vec = face->getNormal(i);
		lua_createtable(L, 3, 0);
		lua_pushnumber(L, vec[0]); lua_rawseti(L, -2, 1);
		lua_pushnumber(L, vec[1]); lua_rawseti(L, -2, 2);
		lua_pushnumber(L, vec[2]); lua_rawseti(L, -2, 3);
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}


int LuaObstacle::GetFaceTxcds(lua_State* L)
{
	const MeshFace* face = ParseMeshFace(L, 1);
	if (face == NULL) {
		return 0;
	}

	// defined texcoords	
	if (face->useTexcoords()) {
		const int elements = face->getVertexCount();
		lua_createtable(L, elements, 0);
		for (int i = 0; i < elements; i++) {
			const float* vec = face->getTexcoord(i);
			lua_createtable(L, 2, 0);
			lua_pushnumber(L, vec[0]); lua_rawseti(L, -2, 1);
			lua_pushnumber(L, vec[1]); lua_rawseti(L, -2, 2);
			lua_rawseti(L, -2, i + 1);
		}
		return 1;
	}

	// generated texcoords	
	if (!lua_isboolean(L, 3) || lua_tobool(L, 3)) {
		const int elements = face->getVertexCount();
		GLfloat3Array vertArray(elements);
		for (int i = 0; i < elements; i++) {
			memcpy(vertArray[i], face->getVertex(i), sizeof(float[3]));
		}
		GLfloat2Array txcdArray(elements);
		if (!makeTexcoords(face->getPlane(), vertArray, txcdArray)) {
			return 0;
		}
		lua_createtable(L, elements, 0);
		for (int i = 0; i < elements; i++) {
			const float* vec = txcdArray[i];
			lua_createtable(L, 2, 0);
			lua_pushnumber(L, vec[0]); lua_rawseti(L, -2, 1);
			lua_pushnumber(L, vec[1]); lua_rawseti(L, -2, 2);
			lua_rawseti(L, -2, i + 1);
		}
		return 1;
	}

	return 0;
}


int LuaObstacle::GetFacePlane(lua_State* L)
{
	const MeshFace* face = ParseMeshFace(L, 1);
	if (face == NULL) {
		return 0;
	}
	const float* vec = face->getPlane();
	lua_createtable(L, 4, 0);
	lua_pushnumber(L, vec[0]); lua_rawseti(L, -2, 1);
	lua_pushnumber(L, vec[1]); lua_rawseti(L, -2, 2);
	lua_pushnumber(L, vec[2]); lua_rawseti(L, -2, 3);
	lua_pushnumber(L, vec[3]); lua_rawseti(L, -2, 4);
	return 1;
}


int LuaObstacle::GetFacePhyDrv(lua_State* L)
{
	const MeshFace* face = ParseMeshFace(L, 1);
	if (face == NULL) {
		return 0;
	}
	lua_pushinteger(L, face->getPhysicsDriver());
	return 1;
}


int LuaObstacle::GetFaceMaterial(lua_State* L)
{
	const MeshFace* face = ParseMeshFace(L, 1);
	if (face == NULL) {
		return 0;
	}
	lua_pushinteger(L, face->getMaterial()->getID() + 1);
	return 1;
}


int LuaObstacle::GetFaceSmoothBounce(lua_State* L)
{
	const MeshFace* face = ParseMeshFace(L, 1);
	if (face == NULL) {
		return 0;
	}
	lua_pushboolean(L, face->isSmoothBounce());
	return 1;
}


/******************************************************************************/
/******************************************************************************/

static const Teleporter* ParseTeleporter(lua_State* L, int index)
{
	if (lua_israwstring(L, index)) {
	}
	else {
		const Obstacle* obs = ParseObstacle(L, index);
		if ((obs == NULL) || (obs->getTypeID() != teleType)) {
			return NULL;
		}
		return (const Teleporter*)obs;
	}
	return NULL;
}


/******************************************************************************/

int LuaObstacle::GetTeleByName(lua_State* L)
{
	const Teleporter* tele = ParseTeleporter(L, 1);
	if (tele == NULL) {
		return 0;
	}
	lua_pushinteger(L, tele->getGUID());  
	return 1;
}


int LuaObstacle::GetTeleBorder(lua_State* L)
{
	const Teleporter* tele = ParseTeleporter(L, 1);
	if (tele == NULL) {
		return 0;
	}
	lua_pushnumber(L, tele->getBorder());
	return 1;
}


int LuaObstacle::GetTeleLinks(lua_State* L)
{
	const Teleporter* tele = ParseTeleporter(L, 1);
	if (tele == NULL) {
		return 0;
	}
	const int frontLink = tele->getListID() * 2;
	const int backLink  = frontLink + 1;
	lua_pushinteger(L, frontLink);
	lua_pushinteger(L, backLink);
	return 2;
}


int LuaObstacle::GetLinkDestinations(lua_State* L)
{
	const int linkID = luaL_checkint(L, 1);
	const World* world = World::getWorld();
	if (world == NULL) {
		return 0;
	}
	const LinkManager& linkMgr = world->getLinkManager();
	const LinkManager::LinkNumberSet* links = linkMgr.getLinkDsts(linkID);
	if (links == NULL) {
		return 0;
	}
	const std::vector<int>& dsts = links->dsts;
	std::vector<int>::const_iterator it;
	lua_createtable(L, dsts.size(), 0);
	int count = 0;
	for (it = dsts.begin(); it != dsts.end(); ++it) {
		count++;
		lua_pushinteger(L, *it);
		lua_rawseti(L, -2, count);
	}
	return 1;
}


/******************************************************************************/
/******************************************************************************/

static void PushDrawCmd(lua_State* L, const DrawCmd& cmd)
{
	lua_newtable(L);

	HSTR_PUSH_INT(L, "mode", cmd.drawMode);

	HSTR_PUSH(L, "indices");
	lua_createtable(L, cmd.count, 0);
	if (cmd.indexType == DrawCmd::DrawIndexUShort) {
		unsigned short* array = (unsigned short*)cmd.indices;
		for (int i = 0; i < cmd.count; i++) {
			lua_pushinteger(L, array[i]);
			lua_rawseti(L, -2, i + 1);
		}
	}
	else if (cmd.indexType == DrawCmd::DrawIndexUInt) {
		unsigned int* array = (unsigned int*)cmd.indices;
		for (int i = 0; i < cmd.count; i++) {
			lua_pushinteger(L, array[i]);
			lua_rawseti(L, -2, i + 1);
		}
	}
	lua_rawset(L, -3);
}


static void PushDrawSet(lua_State* L, const DrawSet& set)
{
	lua_newtable(L);

	HSTR_PUSH_INT(L, "material", set.material->getID() + 1);
	HSTR_PUSH_BOOL(L, "wantList", set.wantList);

	HSTR_PUSH(L, "sphere");
	lua_createtable(L, 0, 4);
	HSTR_PUSH_NUMBER(L, "x", set.sphere[0]);
	HSTR_PUSH_NUMBER(L, "y", set.sphere[1]);
	HSTR_PUSH_NUMBER(L, "z", set.sphere[2]);
	HSTR_PUSH_NUMBER(L, "r", set.sphere[3]);
	lua_rawset(L, -3);

	HSTR_PUSH(L, "commands");
	lua_createtable(L, set.count, 0);
	for (int i = 0; i < set.count; i++) {
		PushDrawCmd(L, set.cmds[i]);
		lua_rawseti(L, -2, i + 1);
	}
	lua_rawset(L, -3);
}


static void PushDrawLod(lua_State* L, const DrawLod& lod)
{
	lua_newtable(L);

	HSTR_PUSH_NUMBER(L, "lengthPerPixel", lod.lengthPerPixel);

	HSTR_PUSH(L, "lods");
	lua_createtable(L, lod.count, 0);
	for (int i = 0; i < lod.count; i++) {
		PushDrawSet(L, lod.sets[i]);
		lua_rawseti(L, -2, i + 1);
	}
	lua_rawset(L, -3);
}


static void PushTransform(lua_State* L, const MeshTransform::Tool& tool)
{
	lua_newtable(L);

	HSTR_PUSH_BOOL(L, "inverted", tool.isInverted());
	HSTR_PUSH_BOOL(L, "skewed",   tool.isSkewed());

	HSTR_PUSH(L, "matrix");
	const float* matrix = tool.getMatrix();
	lua_createtable(L, 16, 0);
	for (int i = 0; i < 16; i++) {
		lua_pushnumber(L, matrix[i]);
		lua_rawseti(L, -2, i + 1);
	}
	lua_rawset(L, -3);
}


static void PushDrawInfo(lua_State* L, const MeshDrawInfo& info)
{
	lua_newtable(L);

	HSTR_PUSH_STRING(L, "name", info.getName());

	const AnimationInfo* anim = info.getAnimationInfo();
	if (anim != NULL) {
		HSTR_PUSH_NUMBER(L, "angvel", anim->angvel);
	}

	HSTR_PUSH(L, "sphere");
	const float* sphere = info.getSphere();
	lua_createtable(L, 0, 4);
	HSTR_PUSH_NUMBER(L, "x", sphere[0]);
	HSTR_PUSH_NUMBER(L, "y", sphere[1]);
	HSTR_PUSH_NUMBER(L, "z", sphere[2]);
	HSTR_PUSH_NUMBER(L, "r", sphere[3]);
	lua_rawset(L, -3);

	HSTR_PUSH(L, "extents");
	const Extents& exts = info.getExtents();
	lua_createtable(L, 0, 6);
	HSTR_PUSH_NUMBER(L, "minx", exts.mins[0]);
	HSTR_PUSH_NUMBER(L, "miny", exts.mins[1]);
	HSTR_PUSH_NUMBER(L, "minz", exts.mins[2]);
	HSTR_PUSH_NUMBER(L, "maxx", exts.maxs[0]);
	HSTR_PUSH_NUMBER(L, "maxy", exts.maxs[1]);
	HSTR_PUSH_NUMBER(L, "maxz", exts.maxs[2]);
	lua_rawset(L, -3);

	const MeshTransform::Tool* tool = info.getTransformTool();
	if (tool != NULL) {
		HSTR_PUSH(L, "transform");
		PushTransform(L, *tool);
		lua_rawset(L, -3);
	}

	const int cornerCount = info.getCornerCount();

	HSTR_PUSH(L, "verts");
	lua_createtable(L, cornerCount, 0);
	const fvec3* verts = info.getVertices();
	for (int i = 0; i < cornerCount; i++) {
		lua_createtable(L, 3, 0);
		lua_pushnumber(L, verts[i][0]); lua_rawseti(L, -2, 1);
		lua_pushnumber(L, verts[i][1]); lua_rawseti(L, -2, 2);
		lua_pushnumber(L, verts[i][2]); lua_rawseti(L, -2, 3);
		lua_rawseti(L, -2, i + 1);
	}
	lua_rawset(L, -3);

	HSTR_PUSH(L, "norms");
	lua_createtable(L, cornerCount, 0);
	const fvec3* norms = info.getNormals();
	for (int i = 0; i < cornerCount; i++) {
		lua_createtable(L, 3, 0);
		lua_pushnumber(L, norms[i][0]); lua_rawseti(L, -2, 1);
		lua_pushnumber(L, norms[i][1]); lua_rawseti(L, -2, 2);
		lua_pushnumber(L, norms[i][2]); lua_rawseti(L, -2, 3);
		lua_rawseti(L, -2, i + 1);
	}
	lua_rawset(L, -3);

	HSTR_PUSH(L, "txcds");
	lua_createtable(L, cornerCount, 0);
	const fvec2* txcds = info.getTexcoords();
	for (int i = 0; i < cornerCount; i++) {
		lua_createtable(L, 2, 0);
		lua_pushnumber(L, txcds[i][0]); lua_rawseti(L, -2, 1);
		lua_pushnumber(L, txcds[i][1]); lua_rawseti(L, -2, 2);
		lua_rawseti(L, -2, i + 1);
	}
	lua_rawset(L, -3);

	HSTR_PUSH(L, "drawLods");
	const DrawLod* drawLods = info.getDrawLods();
	lua_createtable(L, info.getLodCount(), 0);
	for (int i = 0; i < info.getLodCount(); i++) {
		PushDrawLod(L, drawLods[i]);
		lua_rawseti(L, -2, i + 1);
	}
	lua_rawset(L, -3);

	HSTR_PUSH(L, "radarLods");
	const DrawLod* radarLods = info.getRadarLods();
	lua_createtable(L, info.getRadarCount(), 0);
	for (int i = 0; i < info.getRadarCount(); i++) {
		PushDrawLod(L, radarLods[i]);
		lua_rawseti(L, -2, i + 1);
	}
	lua_rawset(L, -3);
}


int LuaObstacle::HasMeshDrawInfo(lua_State* L)
{
	const Obstacle* obs = ParsePrimaryObstacle(L, 1);
	if (obs == NULL) {
		return 0;
	}
	if (obs->getTypeID() != meshType) {
		return 0;
	}
	const MeshObstacle* mesh = (const MeshObstacle*)obs;
	const MeshDrawInfo* drawInfo = mesh->getDrawInfo();
	lua_pushboolean(L, (drawInfo != NULL) && drawInfo->isValid());
	return 1;
}


int LuaObstacle::GetMeshDrawInfo(lua_State* L)
{
	const Obstacle* obs = ParsePrimaryObstacle(L, 1);
	if (obs == NULL) {
		return 0;
	}
	if (obs->getTypeID() != meshType) {
		return 0;
	}
	const MeshObstacle* mesh = (const MeshObstacle*)obs;
	const MeshDrawInfo* drawInfo = mesh->getDrawInfo();
	if ((drawInfo == NULL) || !drawInfo->isValid()) {
		return 0;
	}
	PushDrawInfo(L, *drawInfo);
	return 1;
}


/******************************************************************************/
/******************************************************************************/
