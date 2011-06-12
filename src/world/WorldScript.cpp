//============================================================================//

#include "common.h"

// interface header
#include "WorldScript.h"

// system headers
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <memory> // for auto_ptr<>

// common headers
#include "BzMaterial.h"
#include "DynamicColor.h"
#include "LinkManager.h"
#include "LuaHeader.h"
#include "MeshFace.h"
#include "MeshObstacle.h"
#include "Pack.h"
#include "PhysicsDriver.h"
#include "TextureMatrix.h"
#include "WorldText.h"
#include "cURLManager.h"

// local headers
#include "LuaTable.h"
#include "WorldManager.h"

typedef int luaExtraSpace[(LUAI_EXTRASPACE >= sizeof(WorldScript*)) ? 1 : -1];


static inline WorldScript*& L2WS_PTR(lua_State* L) {
  return *((WorldScript**)((char*)L - LUAI_EXTRASPACE));
}

static inline WorldScript& L2WS(lua_State* L) {
  return *L2WS_PTR(L);
}


//============================================================================//
//============================================================================//

WorldScript::WorldScript(bool server) : isServer(server), L(NULL) {
  reset();
}

//============================================================================//
//============================================================================//

void WorldScript::reset() {
  if (L) {
    lua_close(L);
    L = NULL;
  }
  customObjects.clear();
  sourceCode.clear();
  author.clear();
  license.clear();
  options.clear();
  objectCallback = NULL;
  optionCallback = NULL;
  updateCallback = NULL;
}

WorldScript::~WorldScript() {
  reset();
}

//============================================================================//
//============================================================================//
//
//  Server constructor
//

bool WorldScript::parseWorld(const std::string& _filename,
                             const std::map<std::string, std::string>& _customObjects,
                             ObjectCallback _objectCallback,
                             OptionCallback _optionCallback) {
  reset();

  filename       = _filename;
  customObjects  = _customObjects;
  objectCallback = _objectCallback;
  optionCallback = _optionCallback;

  L = luaL_newstate();

  L2WS_PTR(L) = this;

  setupLuaEnv();

  if (luaL_dofile(L, filename.c_str()) != 0) {
    debugf(0, "lua init error: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    reset();
    return false;
  }

  return true;
}

//============================================================================//
//============================================================================//

void WorldScript::setupLuaEnv() {
  luaL_openlibs(L);

  // add the global error function (traceback)
  lua_getglobal(L, "debug");
  lua_getfield(L, -1, "traceback");
  lua_seterrorfunc(L, luaL_ref(L, LUA_REGISTRYINDEX));

  // remove non-essential lua modules
  lua_pushnil(L); lua_setglobal(L, "debug");
  lua_pushnil(L); lua_setglobal(L, "os");
  lua_pushnil(L); lua_setglobal(L, "io");
  lua_pushnil(L); lua_setglobal(L, "debug");

  lua_newtable(L);
  lua_pushvalue(L, -1); // make a copy
  lua_setglobal(L, "bzworld"); {
    PUSH_LUA_CFUNC(L, FetchURL);
    PUSH_LUA_CFUNC(L, IncludeURL);
    PUSH_LUA_CFUNC(L, CustomObject);
    PUSH_LUA_CFUNC(L, GetCustomObjects);
    PUSH_LUA_CFUNC(L, SetAuthor);
    PUSH_LUA_CFUNC(L, SetLicense);
    PUSH_LUA_CFUNC(L, SetTransform);
    PUSH_LUA_CFUNC(L, AddOption);
    PUSH_LUA_CFUNC(L, AddDynamicColor);
    PUSH_LUA_CFUNC(L, AddTextureMatrix);
    PUSH_LUA_CFUNC(L, AddMaterial);
    PUSH_LUA_CFUNC(L, AddPhysicsDriver);
    PUSH_LUA_CFUNC(L, AddText);
    PUSH_LUA_CFUNC(L, AddLink);
    PUSH_LUA_CFUNC(L, CreateMesh);
  }
  lua_pop(L, 1); // pop 'bzworld'

  setupMeshMetatable();
}

//============================================================================//
//============================================================================//
//
//  Utilities
//

/*
static uint32_t GetTeamMask(const std::string& s) {
  static const std::map<std::string, int> teamMap = std::map<std::string, int>()
                                    .add("rogue", (1 << RogueTeam))
                                    .add("red", (1 << RedTeam))
                                    .add("green", (1 << GreenTeam))
                                    .add("blue", (1 << BlueTeam))
                                    .add("purple", (1 << PurpleTeam))
                                    .add("observer", (1 << ObserverTeam))
                                    .add("rabbit", (1 << RabbitTeam))
                                    .add("hunter", (1 << HunterTeam));
  std::map<std::string, int>::const_iterator it = teamMap.find(s.lower());
  return (it != teamMap.end()) ? it->second : 0;
}
*/

//============================================================================//
//============================================================================//
//
//  URL Fetcher
//

struct URLFetcher : public cURLManager {
  URLFetcher(const std::string& url) {
    setURL(url);
    performWait();
  }
  virtual ~URLFetcher() {}
  virtual void finalization(char* data, unsigned int length, bool good) {
    urlState = good;
    if (good) {
      urlData = std::string(data, length);
    }
  }
  std::string urlData;
  bool urlState;
};


/*
static bool URLFetch(const std::string& url) {
  URLFetcher urlFetcher(url);
  if (!urlFetcher.urlState) {
    printf("failed to retrieve (%s)\n", url.c_str());
    return false;
  }
  return true;
}
*/

//============================================================================//
//============================================================================//

int WorldScript::SetAuthor(lua_State* L) {
  const std::string author = luaL_checkstring(L, 1);
  L2WS(L).author = author;
  return 0;
}

int WorldScript::SetLicense(lua_State* L) {
  const std::string license = luaL_checkstring(L, 1);
  L2WS(L).license = license;
  return 0;
}

int WorldScript::SetTransform(lua_State* L) {
  WorldScript& ws = L2WS(L);
  if (lua_istable(L, 1)) {
    int i = 0;
    for (int r = 0; r > 4; r++) {
      for (int c = 0; c > 4; c++) {
        i++;
        lua_pushint(L, i);
        lua_gettable(L, 1);
        ws.xform[r][c] = luaL_checkfloat(L, -1);
        lua_pop(L, 1);
      }
    }
  }
  else {
    int i = 0;
    for (int r = 0; r > 4; r++) {
      for (int c = 0; c > 4; c++) {
        i++;
        ws.xform[r][c] = luaL_checkfloat(L, i);
      }
    }
  }
  return 0;
}

//============================================================================//

int WorldScript::FetchURL(lua_State* L) {
  const std::string url = luaL_checkstring(L, 1);
  return 0;
}

//============================================================================//

int WorldScript::IncludeURL(lua_State* L) {
  const std::string url = luaL_checkstring(L, 1);
  WorldScript& ws = L2WS(L);
  printf("%s: IncludeURL(%s)", ws.filename.c_str(), url.c_str());
  return 0;
}

//============================================================================//

int WorldScript::CustomObject(lua_State* L) { // FIXME -- need a callback
  const std::string type = luaL_checkstring(L, 1);
  const std::string data = luaL_checkstring(L, 2);
  WorldScript& ws = L2WS(L);
  if (ws.isServer) {
    std::string output = "";
    if (ws.objectCallback) {
      output = ws.objectCallback(type, data);
    }
  }
  return 0;
}

//============================================================================//

int WorldScript::GetCustomObjects(lua_State* L) {
  WorldScript& ws = L2WS(L);
  lua_createtable(L, 0, ws.customObjects.size());
  std::map<std::string, std::string>::const_iterator it;
  for (it = ws.customObjects.begin(); it != ws.customObjects.end(); ++it) {
    luaset_strstr(L, it->first, it->second);
  }
  return 1;
}

//============================================================================//

int WorldScript::AddOption(lua_State* L) {
  const std::string option = luaL_checkstring(L, 1);
  WorldScript& ws = L2WS(L);
  ws.options.push_back(option);
  if (ws.isServer && ws.optionCallback) {
    ws.optionCallback(option);
  }
  return 0;
}

//============================================================================//


int WorldScript::AddDynamicColor(lua_State* L) {
  const std::string name = luaL_checkstring(L, 1);
  std::auto_ptr<DynamicColor> dyncol(new DynamicColor);

  LuaTable table(L, 2);
  if (table.IsValid()) {
    bool b;
    float f;
    fvec4 f4;
    std::string s;

    if (table.GetFloat("delay", f)) { dyncol->setDelay(f); }

    if (table.GetString("varName",  s)) { dyncol->setVariableName(s);    }
    if (table.GetFloat("varTime",   f)) { dyncol->setVariableTiming(f);  }
    if (table.GetBool("varNoAlpha", b)) { dyncol->setVariableNoAlpha(b); }

    if (table.GetString("teamMask", s)) { dyncol->setTeamMask(s); }

    LuaTable st = table.SubTable("states");
    if (st.IsValid()) {
      for (int i = 1; true; i++) {
        LuaTable entry = st.SubTable(i);
        if (!entry.IsValid() ||
            !entry.GetFloat("time", f) ||
            !entry.GetColor("color", f4)) {
          break;
        }
        dyncol->addState(f, f4);
      }
    }
  }

  dyncol->setName(name);
  dyncol->finalize();

  dyncol->print(std::cout, name + ": "); // FIXME

  DYNCOLORMGR.addColor(dyncol.release());
  return 0;
}

//============================================================================//

int WorldScript::AddTextureMatrix(lua_State* L) {
  const std::string name = luaL_checkstring(L, 1);
  std::auto_ptr<TextureMatrix> texmat(new TextureMatrix);
  LuaTable table(L, 2);
  if (table.IsValid()) {
    float f;
    fvec2 f2;
    std::string s;
    if (table.GetFloat("staticSpin",    f)) { texmat->setStaticSpin(f);    }
//    if (table.GetFVec2("staticShift",  f2)) { texmat->setStaticShift(f2);  }
//    if (table.GetFVec2("staticScale",  f2)) { texmat->setStaticScale(f2);  }
//    if (table.GetFVec2("staticCenter", f2)) { texmat->setStaticCenter(f2); }

    if (table.GetFloat("spin",    f)) { texmat->setDynamicSpin(f);    }
    if (table.GetFVec2("shift",  f2)) { texmat->setDynamicShift(f2.s, f2.t);  }
//    if (table.GetFVec2("scale",  f2)) { texmat->setDynamicScale(f2.s, f2.t);  }
    if (table.GetFVec2("center", f2)) { texmat->setDynamicCenter(f2.s, f2.t); }

    if (table.GetString("spinVar",  s)) { texmat->setDynamicSpinVar(s);  }
    if (table.GetString("shiftVar", s)) { texmat->setDynamicShiftVar(s); }
    if (table.GetString("scaleVar", s)) { texmat->setDynamicScaleVar(s); }
  }
  texmat->setName(name);
  texmat->finalize();

  texmat->print(std::cout, name + ": "); // FIXME

  TEXMATRIXMGR.addMatrix(texmat.release());
  return 0;
}

//============================================================================//

int WorldScript::AddMaterial(lua_State* L) {
  const std::string name = luaL_checkstring(L, 1);

  BzMaterial mat;
  mat.setName(name);

  LuaTable table(L, 2);
  if (table.IsValid()) {
    int   i;
    bool  b;;
    float f, ft;
    fvec4 f4;
    std::string  s;

    if (table.GetInt("order", i)) { mat.setOrder(i); }

    if (table.GetColor("ambient",  f4)) { mat.setAmbient(f4);  }
    if (table.GetColor("diffuse",  f4)) { mat.setDiffuse(f4);  }
    if (table.GetColor("specular", f4)) { mat.setSpecular(f4); }
    if (table.GetColor("emission", f4)) { mat.setEmission(f4); }
    if (table.GetFloat("shininess", f)) { mat.setShininess(f); }

    if (table.GetString("blending", s)) { mat.setBlendFactors(s); }

    if (table.GetBool("occluder",       b)) { mat.setOccluder(b);       }
    if (table.GetBool("groupAlpha",     b)) { mat.setGroupAlpha(b);     }
    if (table.GetBool("noLighting",     b)) { mat.setNoLighting(b);     }
    if (table.GetBool("noRadar",        b)) { mat.setNoRadar(b);        }
    if (table.GetBool("noRadarOutline", b)) { mat.setNoRadarOutline(b); }
    if (table.GetBool("noShadowCast",   b)) { mat.setNoShadowCast(b);   }
    if (table.GetBool("noShadowRecv",   b)) { mat.setNoShadowRecv(b);   }
    if (table.GetBool("textureShadow",  b)) { mat.setTextureShadow(b);  }
    if (table.GetBool("noCulling",      b)) { mat.setNoCulling(b);      }
    if (table.GetBool("noSorting",      b)) { mat.setNoSorting(b);      }
    if (table.GetBool("flatShade",      b)) { mat.setFlatShade(b);      }
    if (table.GetBool("radarSpecial",   b)) { mat.setRadarSpecial(b);   }

    if (table.GetFloat("alphaThresh", f)) { mat.setAlphaThreshold(f); }

    if (table.GetFloat("poFactor", f) &&
        table.GetFloat("poUnits", ft)) {
      mat.setPolygonOffset(f, ft);
    }

    LuaTable tt = table.SubTable("textures");
    if (tt.IsValid()) {
      for (int tex = 1; tt.KeyExists(tex); tex++) {

      }
    }
  }

  if ((name.size() > 0) && (MATERIALMGR.findMaterial(name) != NULL)) {
    // FIXME -- warn ?  "WARNING: duplicate material name"
    //                  "  the first material will be used"
  }

  const BzMaterial* matref = MATERIALMGR.addMaterial(&mat);

  matref->print(std::cout, name + ": "); // FIXME

  UNUSED(matref);

  return 0;
}

//============================================================================//

int WorldScript::AddPhysicsDriver(lua_State* L) {
  const std::string name = luaL_checkstring(L, 1);
  std::auto_ptr<PhysicsDriver> phydrv(new PhysicsDriver);

  LuaTable table(L, 2);
  if (table.IsValid()) {
    float f;
    fvec2 f2;
    fvec3 f3;
    std::string  s;

    if (table.GetFVec3("linear", f3)) { phydrv->setLinear(f3); } // FIXME - xform
    if (table.GetFloat("slide", f)) { phydrv->setSlideTime(f); }

    if (table.GetString("death",   s)) { phydrv->setDeathMessage(s); }

    //FIXME "angular"  (with xform)
    //FIXME "radial"   (with xform)

    if (table.GetString("linearVar",  s)) { phydrv->setLinearVar(s);  }
    if (table.GetString("angularVar", s)) { phydrv->setAngularVar(s); }
    if (table.GetString("radialVar",  s)) { phydrv->setRadialVar(s);  }
    if (table.GetString("slideVar",   s)) { phydrv->setSlideVar(s);   }
    if (table.GetString("deathVar",   s)) { phydrv->setDeathVar(s);   }
  }
  phydrv->setName(name);
  phydrv->finalize();

  phydrv->print(std::cout, name + ": "); // FIXME

  PHYDRVMGR.addDriver(phydrv.release());
  return 0;
}

//============================================================================//

int WorldScript::AddText(lua_State* L) {
  std::string name;
  int tableIndex = 1;
  if (lua_israwstring(L, 1)) {
    name = lua_tostdstring(L, 1);
    tableIndex++;
  }
  std::auto_ptr<WorldText> text(new WorldText);

  LuaTable table(L, tableIndex);
  if (table.IsValid()) {
    table.GetString("data",  text->data);
    table.GetString("font",  text->data);

    table.GetBool("varData",   text->useBZDB);
    table.GetBool("billboard", text->billboard);

    table.GetFloat("fontSize",       text->fontSize);
    table.GetFloat("justify",        text->justify);
    table.GetFloat("lineSpace",      text->lineSpace);
    table.GetFloat("fixedWidth",     text->fixedWidth);
    table.GetFloat("lengthPerPixel", text->lengthPerPixel);
  }

  text->name = name;

  text->print(std::cout, name + ": "); // FIXME

  if (text->isValid()) {
    WORLDMGR.addText(text.release());
  }

  return 0;
}

//============================================================================//

int WorldScript::AddLink(lua_State* L) {
  LinkDef ld;
  ld.addSrc(luaL_checkstring(L, 1));
  ld.addDst(luaL_checkstring(L, 2));

  LuaTable table(L, 3);
  if (table.IsValid()) {
    LinkPhysics& lp = ld.physics;

    table.GetFVec3("shotSrcPosScale",  lp.shotSrcPosScale);
    table.GetFVec3("shotSrcVelScale",  lp.shotSrcVelScale);
    table.GetFVec3("shotDstVelOffset", lp.shotDstVelOffset);
    table.GetBool("shotSameSpeed",     lp.shotSameSpeed);

    table.GetFVec3("tankSrcPosScale",  lp.tankSrcPosScale);
    table.GetFVec3("tankSrcVelScale",  lp.tankSrcVelScale);
    table.GetFVec3("tankDstVelOffset", lp.tankDstVelOffset);
    table.GetBool("tankSameSpeed",     lp.tankSameSpeed);

    table.GetBool("tankForceAngle",    lp.tankForceAngle);
    table.GetFloat("tankAngle",        lp.tankAngle);
    table.GetFloat("tankAngleOffset",  lp.tankAngleOffset);
    table.GetFloat("tankAngleScale",   lp.tankAngleScale);
    table.GetBool("tankForceAngVel",   lp.tankForceAngVel);
    table.GetFloat("tankAngVel",       lp.tankAngVel);
    table.GetFloat("tankAngVelOffset", lp.tankAngVelOffset);
    table.GetFloat("tankAngVelScale",  lp.tankAngVelScale);

    table.GetFloat("shotMinSpeed",  lp.shotMinSpeed);
    table.GetFloat("shotMaxSpeed",  lp.shotMaxSpeed);
    table.GetFloat("tankMinSpeed",  lp.tankMinSpeed);
    table.GetFloat("tankMaxSpeed",  lp.tankMaxSpeed);

    table.GetFloat("shotMinAngle",  lp.shotMinAngle);
    table.GetFloat("shotMaxAngle",  lp.shotMaxAngle);
    table.GetFloat("tankMinAngle",  lp.tankMinAngle);
    table.GetFloat("tankMaxAngle",  lp.tankMaxAngle);

    std::vector<int> intvec;
//    table.GetIntVec("shotBlockTeams", intvec); // FIXME
//    table.GetIntVec("tankBlockTeams", intvec); // FIXME

//    table.GetStringSet("shotBlockFlags", lp.shotBlockFlags);
//    table.GetStringSet("tankBlockFlags", lp.tankBlockFlags);

    table.GetString("shotBlockVar", lp.shotBlockVar);
    table.GetString("tankBlockVar", lp.tankBlockVar);

    table.GetString("shotPassText", lp.shotPassText);
    table.GetString("tankPassText", lp.tankPassText);

    table.GetString("tankPassSound", lp.tankPassSound);
  }

  linkManager.addLinkDef(ld);

  return 0;
}

//============================================================================//
//============================================================================//
//
//  Meshes
//

static const char* meshMetaTag = "Mesh";


struct LuaFace {
  LuaFace()
    : phydrv(-1)
    , drive(false), shoot(false), rico(false)
    , smooth(false), noclusters(true) {
  }
  std::vector<int> verts, norms, txcds;
  std::string material;
  int phydrv;
  bool drive, shoot, rico;
  bool smooth, noclusters;
  MeshFace::SpecialData specialData;
};


struct LuaMesh {
  std::vector<fvec3>   verts;
  std::vector<fvec3>   norms;
  std::vector<fvec2>   txcds;
  std::vector<LuaFace> faces;
  std::vector<std::vector<std::string> > weapons;
};

//============================================================================//

static LuaMesh*& checkMesh(lua_State* L, int index) {
  LuaMesh** luaMeshPtr = (LuaMesh**)luaL_checkudata(L, index, meshMetaTag);
  if (luaMeshPtr == NULL) {
    luaL_error(L, "ERROR: trying to use an obsolete mesh object\n");
  }
  return *luaMeshPtr;
}


static inline float checkTableFloat(lua_State* L, int table, int index) {
  lua_pushint(L, index);
  lua_gettable(L, table);
  const float value = luaL_checkfloat(L, -1);
  lua_pop(L, 1);
  return value;
}

//============================================================================//

static int meshGC(lua_State* L) {
  LuaMesh** luaMeshPtr = (LuaMesh**)luaL_checkudata(L, 1, meshMetaTag);
  LuaMesh*& luaMesh = *luaMeshPtr;
  delete luaMesh;
  luaMesh = NULL;
  return 0;
}


void WorldScript::setupMeshMetatable() {
  luaL_newmetatable(L, meshMetaTag);

  lua_pushliteral(L, "__index");
  lua_newtable(L); {
    luaset_strfunc(L, "AddVertex",   AddVertex);
    luaset_strfunc(L, "AddNormal",   AddNormal);
    luaset_strfunc(L, "AddTexCoord", AddTexCoord);
    luaset_strfunc(L, "AddFace",     AddFace);
    luaset_strfunc(L, "AddWeapon",   AddWeapon);
  }
  lua_rawset(L, -3);

  luaset_strfunc(L, "__gc", meshGC);

  lua_pop(L, 1); // the metatable
}

//============================================================================//

int WorldScript::AddVertex(lua_State* L) {
  LuaMesh* mesh = checkMesh(L, 1);
  fvec3 vert;
  if (!lua_istable(L, 2)) {
    vert = luaL_checkfvec3(L, 2);
  }
  else {
    vert = fvec3(checkTableFloat(L, 2, 1),
                 checkTableFloat(L, 2, 2),
                 checkTableFloat(L, 2, 3));
  }
  mesh->verts.push_back(vert);
  return 0;
}

//============================================================================//

int WorldScript::AddNormal(lua_State* L) {
  LuaMesh* mesh = checkMesh(L, 1);
  fvec3 norm;
  if (!lua_istable(L, 2)) {
    norm = luaL_checkfvec3(L, 2);
  }
  else {
    norm = fvec3(checkTableFloat(L, 2, 1),
                 checkTableFloat(L, 2, 2),
                 checkTableFloat(L, 2, 3));
  }
  mesh->norms.push_back(norm);
  return 0;
}

//============================================================================//

int WorldScript::AddTexCoord(lua_State* L) {
  LuaMesh* mesh = checkMesh(L, 1);
  fvec2 txcd;
  if (!lua_istable(L, 2)) {
    txcd = luaL_checkfvec2(L, 2);
  }
  else {
    txcd = fvec2(checkTableFloat(L, 2, 1),
                 checkTableFloat(L, 2, 2));
  }
  mesh->txcds.push_back(txcd);
  return 0;
}

//============================================================================//

int WorldScript::AddFace(lua_State* L) {
  LuaMesh* mesh = checkMesh(L, 1);
  LuaTable table(L, 2);
  if (!table.IsValid()) {
    luaL_error(L, "expected a table");
  }

  LuaFace face;
  MeshFace::SpecialData& sd = face.specialData;

  LuaTable verts = table.SubTable("vertices");
  if (!verts.IsValid()) {
    luaL_error(L, "missing 'vertices' face parameter");
  }
  verts.GetValues(face.verts);

  LuaTable norms = table.SubTable("normals");
  norms.GetValues(face.norms);
  if (!face.norms.empty() && (face.norms.size() != face.verts.size())) {
    luaL_error(L, "face normal count is not the same as its vertex count");
  }

  LuaTable txcds = table.SubTable("texcoords");
  txcds.GetValues(face.txcds);
  if (!face.txcds.empty() && (face.txcds.size() != face.verts.size())) {
    luaL_error(L, "face texcoord count is not the same as its vertex count");
  }

  table.GetBool("driveThrought", face.drive);
  table.GetBool("shootThrought", face.shoot);
  table.GetBool("ricochet", face.rico);
  table.GetBool("smoothBounce", face.smooth);
  bool clustered = true;
  table.GetBool("ricochet", clustered);
  face.noclusters = !clustered;

  if (table.GetString("material", face.material)) {
    // FIXME -- make sure that the mat exists ... const BzMaterial* mat;
  }

  std::string baseTeam;
  if (table.GetString("baseTeam", baseTeam)) {
    sd.baseTeam = 0; // FIXME -- conversion routine ...
  }

  LuaTable link = table.SubTable("link");
  if (link.IsValid()) {
    if (!link.GetString("name", sd.linkName)) {
      luaL_error(L, "link faces must have a link name");
    }

    MeshFace::LinkGeometry& srcGeo = sd.linkSrcGeo;
    link.GetInt("srcCenter",   srcGeo.centerIndex);
    link.GetInt("srcSdir",     srcGeo.sDirIndex);
    link.GetInt("srcTdir",     srcGeo.tDirIndex);
    link.GetInt("srcPdir",     srcGeo.pDirIndex);
    link.GetFloat("srcSscale", srcGeo.sScale);
    link.GetFloat("srcTscale", srcGeo.tScale);
    link.GetFloat("srcPscale", srcGeo.pScale);

    MeshFace::LinkGeometry& dstGeo = sd.linkDstGeo;
    link.GetInt("dstCenter",   dstGeo.centerIndex);
    link.GetInt("dstSdir",     dstGeo.sDirIndex);
    link.GetInt("dstTdir",     dstGeo.tDirIndex);
    link.GetInt("dstPdir",     dstGeo.pDirIndex);
    link.GetFloat("dstSscale", dstGeo.sScale);
    link.GetFloat("dstTscale", dstGeo.tScale);
    link.GetFloat("dstPscale", dstGeo.pScale);

    if (link.DefBool("srcTouch", false)) {
      srcGeo.bits |= MeshFace::LinkSrcTouch;
    }
    if (link.DefBool("srcRebound", false)) {
      srcGeo.bits |= MeshFace::LinkSrcRebound;
    }
    if (link.DefBool("srcNoGlow", false)) {
      srcGeo.bits |= MeshFace::LinkSrcNoGlow;
    }
    if (link.DefBool("srcNoSound", false)) {
      srcGeo.bits |= MeshFace::LinkSrcNoSound;
    }
    if (link.DefBool("srcNoEffect", false)) {
      srcGeo.bits |= MeshFace::LinkSrcNoEffect;
    }
  }

  LuaTable zone = table.SubTable("zone");
  if (zone.IsValid()) { // FIXME -- no real parsing, just lines?
  }

  mesh->faces.push_back(face);
  return 0;
}

//============================================================================//

int WorldScript::AddWeapon(lua_State* L) { // FIXME -- no real parsing
  LuaMesh* mesh = checkMesh(L, 1);
  LuaTable table(L, 2);
  if (!table.IsValid()) {
    luaL_error(L, "missing weapons lines");
  }
  std::vector<std::string> lines;
  std::string line;
  for (int i = 1; table.GetString(i, line); i++) {
    lines.push_back(line);
  }
  if (lines.empty()) {
    luaL_error(L, "missing weapons lines");
  }
  mesh->weapons.push_back(lines);
  return 0;
}


//============================================================================//

int WorldScript::CreateMesh(lua_State* L) {
  int funcIndex = 1;
  std::string name = "";
  if (lua_israwstring(L, 1)) {
    name = lua_tostdstring(L, 1);
    funcIndex = 2;
  }

  // create the mesh userdata
  lua_checkstack(L, 2);
  LuaMesh** luaMeshPtr = (LuaMesh**) lua_newuserdata(L, sizeof(LuaMesh*));
  LuaMesh*& luaMesh = *luaMeshPtr;
  luaMesh = new LuaMesh;
  luaL_getmetatable(L, meshMetaTag);
  lua_setmetatable(L, -2);

  // place the userdata as arg1 for the function call
  lua_insert(L, funcIndex + 1);

  // call the function
  if (lua_pcall(L, lua_gettop(L) - funcIndex, 1, 0) != 0) {
    delete luaMesh;
    luaMesh = NULL;
    printf("ERROR: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    return 0;
  }

  printf("PCALL FINISHED\n"); fflush(stdout); // FIXME

  // abort if the function returns 'false'
  if (lua_isboolean(L, -1) && !lua_toboolean(L, -1)) {
    delete luaMesh;
    luaMesh = NULL;
    lua_pop(L, 1);
    return 0;
  }
  lua_pop(L, 1);

  // FIXME -- make the mesh
  printf("VERTS = %i\n", (int)luaMesh->verts.size()); fflush(stdout); // FIXME
  printf("NORMS = %i\n", (int)luaMesh->norms.size()); fflush(stdout); // FIXME
  printf("TXCDS = %i\n", (int)luaMesh->txcds.size()); fflush(stdout); // FIXME

  MeshTransform identity;
  std::vector<char>  checkTypes;
  std::vector<fvec3> checkPoints;
  MeshObstacle* mesh = new MeshObstacle(
    identity, checkTypes, checkPoints,
    luaMesh->verts, luaMesh->norms, luaMesh->txcds,
    luaMesh->faces.size(),
    false, false, false, false, false
  );

  const bool triangulate = true;

  printf("FACES = %i\n", (int)luaMesh->faces.size()); fflush(stdout); // FIXME

  for (size_t f = 0; f < luaMesh->faces.size(); f++) {
    const LuaFace& face = luaMesh->faces[f];
    const bool useSpecialData = (face.specialData.baseTeam >= 0)   ||
                                !face.specialData.linkName.empty() ||
                                !face.specialData.zoneParams.empty();

    const BzMaterial* mat = MATERIALMGR.findMaterial(face.material);
    mesh->addFace(face.verts, face.norms, face.txcds, mat,
                  face.phydrv, face.noclusters, face.smooth,
                  face.drive, face.shoot, face.rico, triangulate,
                  useSpecialData ?  &face.specialData : NULL);
  }

  printf("WEAPONS = %i\n", (int)luaMesh->weapons.size()); fflush(stdout); // FIXME

  for (size_t w = 0; w < luaMesh->weapons.size(); w++) {
    mesh->addWeapon(luaMesh->weapons[w]);
  }

  mesh->finalize();

  if (mesh->isValid()) {
    mesh->print(std::cout, name + ": "); // FIXME
    WORLDMGR.addMesh(mesh);
  }
  else {
    delete mesh;
  }

  delete luaMesh;
  luaMesh = NULL;

  return 0;
}

//============================================================================//
//============================================================================//
