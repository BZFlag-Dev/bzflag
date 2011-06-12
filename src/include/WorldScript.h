
#ifndef WORLD_SCRIPT_H
#define WORLD_SCRIPT_H

// system headers
#include <string>
#include <vector>
#include <map>

// common headers
#include "vectors.h"


struct lua_State;


class WorldScript {
  public:
    // for the server
    typedef std::string(*OptionCallback)(const std::string& option);
    typedef std::string(*ObjectCallback)(const std::string& tag, const std::string& data);
    // for the clients
    typedef std::string(*UpdateCallback)(const std::string& option);

  public:
    WorldScript(bool isServer);
    ~WorldScript();

    void reset();

    // for servers
    bool parseWorld(const std::string&    url,
                    const std::map<std::string, std::string>& customObjects,
                    ObjectCallback objectCallback,
                    OptionCallback optionCallback);

    int   packSize()  const;
    void* pack(void*) const;

    // for clients
    void* unpack(void*);

    bool startResourcing(UpdateCallback);
    bool checkResourcing() const;
    void cancelResourcng();

    std::string getPrintString(bool embed_everything = true) const;

  private:
    void setupLuaEnv();
    void setupMeshMetatable();

  private:
    struct Resource {
      enum Type {
        WS_TIME,         // only saved if used  (invalidates caching)
        WS_URL_INFO,
        WS_FILE,
        WS_CUSTOM_OBJECT
      };
      Type type;
    };

  private:
    const bool isServer;
    std::string filename;
    std::map<std::string, std::string> customObjects;

    ObjectCallback objectCallback;
    OptionCallback optionCallback;
    UpdateCallback updateCallback;

    lua_State* L;

    std::string    sourceCode;
    std::vector<std::string> sourceSwaps;

    std::string    author;
    std::string    license;
    std::vector<std::string> options;

    fvec4 xform[4];

  private: // lua call-outs
    static int FetchURL(lua_State* L);
    static int IncludeURL(lua_State* L);
    static int CustomObject(lua_State* L);

    static int GetCustomObjects(lua_State* L);

    static int SetAuthor(lua_State* L);
    static int SetLicense(lua_State* L);

    static int SetTransform(lua_State* L);

    static int AddOption(lua_State* L);
    static int AddDynamicColor(lua_State* L);
    static int AddTextureMatrix(lua_State* L);
    static int AddMaterial(lua_State* L);
    static int AddPhysicsDriver(lua_State* L);
    static int AddText(lua_State* L);
    static int AddLink(lua_State* L);

    static int CreateMesh(lua_State* L);
    static int AddVertex(lua_State* L);
    static int AddNormal(lua_State* L);
    static int AddTexCoord(lua_State* L);
    static int AddFace(lua_State* L);
    static int AddWeapon(lua_State* L);
    static int SetDrawInfo(lua_State* L);

  private: // no copying
    WorldScript(const WorldScript&);
    WorldScript& operator=(const WorldScript&);
};


#endif // WORLD_SCRIPT_H
