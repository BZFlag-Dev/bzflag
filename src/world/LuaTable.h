
#ifndef LUA_TABLE_H
#define LUA_TABLE_H


#include "common.h"

// system headers
#include <string>
#include <vector>
#include <set>
#include <map>

// common headers
#include "vectors.h"


struct lua_State;


class LuaTable {
  public:
    LuaTable();
    LuaTable(lua_State* L, int index);
    LuaTable(const LuaTable&);
    LuaTable& operator=(const LuaTable&);
    virtual ~LuaTable();

    LuaTable SubTable(int key) const;
    LuaTable SubTable(const std::string& key) const;
    LuaTable SubTableExpr(const std::string& expr) const;

    inline bool IsValid() const { return (L != NULL); }

    int GetLength() const;                // lua '#' operator
    int GetLength(int key) const;         // lua '#' operator
    int GetLength(const std::string& key) const; // lua '#' operator

    bool KeyExists(int key) const;
    bool KeyExists(const std::string& key) const;

    enum DataType {
      none_type = -1,
      nil_type,
      boolean_type,
      lightuserdata_type,
      number_type,
      string_type,
      table_type,
      function_type,
      userdate_type,
      thread_type
    };
    static DataType LuaTypeToDataType(int);
    static int DataTypeToLuaType(DataType);

    DataType GetType(int key) const;
    DataType GetType(const std::string& key) const;

    bool GetColor(int key, fvec4& value) const;
    bool GetColor(const std::string& key, fvec4& value) const;

    bool GetInt(int key, int&   value) const;
    bool GetBool(int key, bool&  value) const;
    bool GetFloat(int key, float& value) const;
    bool GetFVec2(int key, fvec2& value) const;
    bool GetFVec3(int key, fvec3& value) const;
    bool GetFVec4(int key, fvec4& value) const;
    bool GetString(int key, std::string&  value) const;

    bool GetInt(const std::string& key, int&   value) const;
    bool GetBool(const std::string& key, bool&  value) const;
    bool GetFloat(const std::string& key, float& value) const;
    bool GetFVec2(const std::string& key, fvec2& value) const;
    bool GetFVec3(const std::string& key, fvec3& value) const;
    bool GetFVec4(const std::string& key, fvec4& value) const;
    bool GetString(const std::string& key, std::string&  value) const;

    bool GetKeys(std::set<int>&            data) const;
    bool GetKeys(std::set<float>&          data) const;
    bool GetKeys(std::set<std::string>&    data) const;
    bool GetKeys(std::vector<int>&         data) const;
    bool GetKeys(std::vector<float>&       data) const;
    bool GetKeys(std::vector<std::string>& data) const;

    bool GetValues(std::set<int>&             data) const;
    bool GetValues(std::set<float>&           data) const;
    bool GetValues(std::set<std::string>&     data) const;
    bool GetValues(std::vector<int>&          data) const;
    bool GetValues(std::vector<float>&        data) const;
    bool GetValues(std::vector<std::string>&  data) const;

    bool GetMap(std::map<int,                 int>& data) const;
    bool GetMap(std::map<int,               float>& data) const;
    bool GetMap(std::map<int,         std::string>& data) const;
    bool GetMap(std::map<std::string,         int>& data) const;
    bool GetMap(std::map<std::string,       float>& data) const;
    bool GetMap(std::map<std::string, std::string>& data) const;
    bool GetMap(std::map<float,               int>& data) const;
    bool GetMap(std::map<float,             float>& data) const;
    bool GetMap(std::map<float,       std::string>& data) const;

    // convenience
    inline int   DefInt(int key, int def) const {
      int   value; return GetInt(key, value) ? value : def;
    }
    inline bool  DefBool(int key, bool def) const {
      bool  value; return GetBool(key, value) ? value : def;
    }
    inline float DefFloat(int key, float def) const {
      float value; return GetFloat(key, value) ? value : def;
    }
    inline std::string  DefString(int key, const std::string& def) const {
      std::string  value; return GetString(key, value) ? value : def;
    }

    inline int   DefInt(const std::string& key, int def) const {
      int   value; return GetInt(key, value) ? value : def;
    }
    inline bool  DefBool(const std::string& key, bool def) const {
      bool  value; return GetBool(key, value) ? value : def;
    }
    inline float DefFloat(const std::string& key, float def) const {
      float value; return GetFloat(key, value) ? value : def;
    }
    inline std::string  DefString(const std::string& key, const std::string& def) const {
      std::string  value; return GetString(key, value) ? value : def;
    }

  private:
    template <typename K, typename V> bool GetValue(K key, V& value) const;
    template <typename C> bool GetKeysTemplate(C&) const;
    template <typename C> bool GetValuesTemplate(C&) const;
    template <typename K, typename V> bool GetMapTemplate(std::map<K, V>&) const;

  private:
    bool PushTable() const;
    bool PushValue(int key) const;
    bool PushValue(const std::string& key) const;

  private:
    lua_State* L;
    int luaRef;
};



#endif //LUA_TABLE_H
