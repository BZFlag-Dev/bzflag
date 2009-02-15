
#include "common.h"

// interface header
#include "LuaParser.h"

// system headers
#include <assert.h>
#include <algorithm>

// local headers
#include "LuaInclude.h"


static const bool lowerCppKeys = false;

LuaParser* LuaParser::currentParser = NULL;


/******************************************************************************/
/******************************************************************************/
//
//  Utils from Spring
//

static inline void StringToLowerInPlace(std::string &s)
{
	std::transform(s.begin(), s.end(), s.begin(), (int (*)(int))tolower);
}


static inline std::string StringToLower(std::string s)
{
	StringToLowerInPlace(s);
	return s;
}


/******************************************************************************/
/******************************************************************************/
//
//  LuaParser
//

LuaParser::LuaParser(const string& _fileName)
: fileName(_fileName)
, textChunk("")
, valid(false)
, initDepth(0)
, rootRef(LUA_NOREF)
, currentRef(LUA_NOREF)
, lowerKeys(false)
{
	L = lua_open();

	if (L != NULL) {
		SetupEnv();
	}
}


LuaParser::~LuaParser()
{
	if (L != NULL) {
		lua_close(L);
	}
	set<LuaTable*>::iterator it;
	for (it = tables.begin(); it != tables.end(); ++it) {
		LuaTable& table = **it;
		table.parser  = NULL;
		table.L       = NULL;
		table.isValid = false;
		table.refnum  = LUA_NOREF;
	}
}


void LuaParser::SetupEnv()
{
	luaL_openlibs(L);

	AddFunc("DontMessWithMyCase", DontMessWithMyCase);
}


int LuaParser::DontMessWithMyCase(lua_State* L)
{
	if (currentParser == NULL) {
		luaL_error(L, "invalid call to DontMessWithMyCase() after execution");
	}
	currentParser->SetLowerKeys(lua_tobool(L, 1));
	return 0;  
}


/******************************************************************************/

bool LuaParser::Execute()
{
	if (L == NULL) {
		errorLog = "could not initialize LUA library";
		return false;
	}

	rootRef = LUA_NOREF;

	assert(initDepth == 0);
	initDepth = -1;

	string code;
	string codeLabel;
	if (!textChunk.empty()) {
		code = textChunk;
		codeLabel = "text chunk";
	}
	else if (!fileName.empty()) {
		codeLabel = fileName;
		FILE* fh = fopen(fileName.c_str(), "r");
		if (fh == NULL) {
			errorLog = "could not open file: " + fileName;
			lua_close(L);
			L = NULL;
			return false;
		}
		char buf[4096];
		while (feof(fh) == 0) {
			const size_t bytes = fread(buf, 1, sizeof(buf), fh);
			if (bytes > 0) {
				code.append(buf, bytes);
			}
		}
		fclose(fh);
	}
	else {
		errorLog = "no source file or text";
		lua_close(L);
		L = NULL;
		return false;
	}

	int error;
	error = luaL_loadbuffer(L, code.c_str(), code.size(), codeLabel.c_str());
	if (error != 0) {
		errorLog = lua_tostring(L, -1);
		printf("error = %i, %s, %s\n", error, codeLabel.c_str(), errorLog.c_str());
		lua_close(L);
		L = NULL;
		return false;
	}

	currentParser = this;

	error = lua_pcall(L, 0, 1, 0);

	currentParser = NULL;

	if (error != 0) {
		errorLog = lua_tostring(L, -1);
		printf("error = %i, %s, %s\n", error, fileName.c_str(), errorLog.c_str());
		lua_close(L);
		L = NULL;
		return false;
	}

	if (!lua_istable(L, 1)) {
		errorLog = "missing return table from " + fileName;
		printf("missing return table from %s\n", fileName.c_str());
		lua_close(L);
		L = NULL;
		return false;
	}

	if (lowerKeys) {
		LowerKeys(L, 1);
	}

	rootRef = luaL_ref(L, LUA_REGISTRYINDEX);

	lua_settop(L, 0);

	valid = true;

	return true;
}


void LuaParser::AddTable(LuaTable* tbl)
{
	tables.insert(tbl);
}


void LuaParser::RemoveTable(LuaTable* tbl)
{
	tables.erase(tbl);
}


LuaTable LuaParser::GetRoot()
{
	return LuaTable(this);
}


void LuaParser::CallFunction(int (*func)(lua_State*))
{
	if (L == NULL) {
		return;
	}
	lua_pushcclosure(L, func, 0);
	lua_pcall(L, 0, 0, 0);
}


/******************************************************************************/

void LuaParser::PushParam()
{
	if ((L == NULL) || (initDepth < 0)) { return; }
	if (initDepth > 0) {
		lua_rawset(L, -3);
	} else {
		lua_rawset(L, LUA_GLOBALSINDEX);
	}
}


void LuaParser::GetTable(const string& name, bool overwrite)
{
	if ((L == NULL) || (initDepth < 0)) { return; }

	lua_pushstring(L, name.c_str());

	if (overwrite) {
		lua_newtable(L);
	}
	else {
		lua_pushstring(L, name.c_str());
		lua_gettable(L, (initDepth == 0) ? LUA_GLOBALSINDEX : -3);
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			lua_newtable(L);
		}
	}

	initDepth++;
}


void LuaParser::GetTable(int index, bool overwrite)
{
	if ((L == NULL) || (initDepth < 0)) { return; }

	lua_pushinteger(L, index); 

	if (overwrite) {
		lua_newtable(L);
	}
	else {
		lua_pushinteger(L, index);
		lua_gettable(L, (initDepth == 0) ? LUA_GLOBALSINDEX : -3);
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			lua_newtable(L);
		}
	}

	initDepth++;
}


void LuaParser::EndTable()
{
	if ((L == NULL) || (initDepth < 0)) { return; }
	assert(initDepth > 0);
	initDepth--;
	PushParam();
}


/******************************************************************************/

void LuaParser::AddFunc(const string& key, int (*func)(lua_State*))
{
	if ((L == NULL) || (initDepth < 0)) { return; }
	if (func == NULL) { return; }
	lua_pushstring(L, key.c_str());
	lua_pushcfunction(L, func);
	PushParam();
}


void LuaParser::AddInt(const string& key, int value)
{
	if ((L == NULL) || (initDepth < 0)) { return; }
	lua_pushstring(L, key.c_str());
	lua_pushinteger(L, value);
	PushParam();
}


void LuaParser::AddBool(const string& key, bool value)
{
	if ((L == NULL) || (initDepth < 0)) { return; }
	lua_pushstring(L, key.c_str());
	lua_pushboolean(L, value);
	PushParam();
}


void LuaParser::AddFloat(const string& key, float value)
{
	if ((L == NULL) || (initDepth < 0)) { return; }
	lua_pushstring(L, key.c_str());
	lua_pushnumber(L, value);
	PushParam();
}


void LuaParser::AddString(const string& key, const string& value)
{
	if ((L == NULL) || (initDepth < 0)) { return; }
	lua_pushstring(L, key.c_str());
	lua_pushstring(L, value.c_str());
	PushParam();
}


/******************************************************************************/

void LuaParser::AddFunc(int key, int (*func)(lua_State*))
{
	if ((L == NULL) || (initDepth < 0)) { return; }
	if (func == NULL) { return; }
	lua_pushinteger(L, key);
	lua_pushcfunction(L, func);
	PushParam();
}


void LuaParser::AddInt(int key, int value)
{
	if ((L == NULL) || (initDepth < 0)) { return; }
	lua_pushinteger(L, key);
	lua_pushinteger(L, value);
	PushParam();
}


void LuaParser::AddBool(int key, bool value)
{
	if ((L == NULL) || (initDepth < 0)) { return; }
	lua_pushinteger(L, key);
	lua_pushboolean(L, value);
	PushParam();
}


void LuaParser::AddFloat(int key, float value)
{
	if ((L == NULL) || (initDepth < 0)) { return; }
	lua_pushinteger(L, key);
	lua_pushnumber(L, value);
	PushParam();
}


void LuaParser::AddString(int key, const string& value)
{
	if ((L == NULL) || (initDepth < 0)) { return; }
	lua_pushinteger(L, key);
	lua_pushstring(L, value.c_str());
	PushParam();
}


/******************************************************************************/
/******************************************************************************/
//
//  LuaTable
//

LuaTable::LuaTable()
: isValid(false)
, path("")
, parser(NULL)
, L(NULL)
, refnum(LUA_NOREF)
{
}


LuaTable::LuaTable(LuaParser* _parser)
{
	assert(_parser != NULL);

	isValid = _parser->IsValid();
	path    = "ROOT";
	parser  = _parser;
	L       = parser->L;
	refnum  = parser->rootRef;
	
	if (PushTable()) {
		lua_pushvalue(L, -1); // copy
		refnum = luaL_ref(L, LUA_REGISTRYINDEX);
	} else {
	 	refnum = LUA_NOREF;
	}
	isValid = (refnum != LUA_NOREF);

	parser->AddTable(this);
}


LuaTable::LuaTable(const LuaTable& tbl)
{
	parser = tbl.parser;
	L      = tbl.L;
	path   = tbl.path;

	if (tbl.PushTable()) {
		lua_pushvalue(L, -1); // copy
		refnum = luaL_ref(L, LUA_REGISTRYINDEX);
	} else {
		refnum = LUA_NOREF;
	}	
	isValid = (refnum != LUA_NOREF);

	if (parser) {
		parser->AddTable(this);
	}
}


LuaTable& LuaTable::operator=(const LuaTable& tbl)
{
	if (parser && (refnum != LUA_NOREF) && (parser->currentRef == refnum)) {
		lua_settop(L, 0);
		parser->currentRef = LUA_NOREF;
	}

	if (parser != tbl.parser) {
		if (parser != NULL) {
			parser->RemoveTable(this);
		}
		if (L && (refnum != LUA_NOREF)) {
			luaL_unref(L, LUA_REGISTRYINDEX, refnum);
		}
		parser = tbl.parser;
		if (parser != NULL) {
			parser->AddTable(this);
		}
	}

	L    = tbl.L;
	path = tbl.path;
	
	if (tbl.PushTable()) {
		lua_pushvalue(L, -1); // copy
		refnum = luaL_ref(L, LUA_REGISTRYINDEX);
	} else {
		refnum = LUA_NOREF;
	}	

	isValid = (refnum != LUA_NOREF);

	return *this;
}


LuaTable LuaTable::SubTable(int key) const
{
	LuaTable subTable;
	char buf[32];
	snprintf(buf, 32, "[%i]", key);
	subTable.path = path + buf;

	if (!PushTable()) {
		return subTable;
	}

	lua_pushinteger(L, key);
	lua_gettable(L, -2);
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return subTable;
	}

	subTable.parser  = parser;
	subTable.L       = L;
	subTable.refnum  = luaL_ref(L, LUA_REGISTRYINDEX);
	subTable.isValid = (subTable.refnum != LUA_NOREF);

	parser->AddTable(&subTable);

	return subTable;
}


LuaTable LuaTable::SubTable(const string& mixedKey) const
{
	
	const string key = !lowerCppKeys ? mixedKey : StringToLower(mixedKey);

	LuaTable subTable;
	subTable.path = path + "." + key;

	if (!PushTable()) {
		return subTable;
	}

	lua_pushstring(L, key.c_str());
	lua_gettable(L, -2);
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return subTable;
	}

	subTable.parser  = parser;
	subTable.L       = L;
	subTable.refnum  = luaL_ref(L, LUA_REGISTRYINDEX);
	subTable.isValid = (subTable.refnum != LUA_NOREF);

	parser->AddTable(&subTable);

	return subTable;
}


LuaTable LuaTable::SubTableExpr(const string& expr) const
{
	if (expr.empty()) {
		return LuaTable(*this);
	}
	if (!isValid) {
		return LuaTable();
	}

	string::size_type endPos;
	LuaTable nextTable;

	if (expr[0] == '[') { // numeric key
		endPos = expr.find(']');
		if (endPos == string::npos) {
			return LuaTable(); // missing brace
		}
		const char* startPtr = expr.c_str() + 1; // skip the '['
		char* endPtr;
		const int index = strtol(startPtr, &endPtr, 10);
		if (endPtr == startPtr) {
			return LuaTable(); // invalid index
		}
		endPos++; // eat the ']'
		nextTable = SubTable(index);
	}
	else { // string key
		endPos = expr.find_first_of(".[");
		if (endPos == string::npos) {
			return SubTable(expr);
		}
		nextTable = SubTable(expr.substr(0, endPos));
	}

	if (expr[endPos] == '.') {
		endPos++; // eat the dot
	}
	return nextTable.SubTableExpr(expr.substr(endPos));
}


LuaTable::~LuaTable()
{
	if (L && (refnum != LUA_NOREF)) {
		luaL_unref(L, LUA_REGISTRYINDEX, refnum);
		if (parser && (parser->currentRef == refnum)) {
			lua_settop(L, 0);
			parser->currentRef = LUA_NOREF;
		}
	}
	if (parser) {
		parser->RemoveTable(this);
	}
}


/******************************************************************************/

bool LuaTable::PushTable() const
{
	if (!isValid) {
		return false;
	}

	if ((refnum != LUA_NOREF) && (parser->currentRef == refnum)) {
		if (!lua_istable(L, -1)) {
			printf("Internal Error: LuaTable::PushTable() = %s\n", path.c_str());
			parser->currentRef = LUA_NOREF;
			lua_settop(L, 0);
			return false;
		}
		return true;
	}

	lua_settop(L, 0);

	lua_rawgeti(L, LUA_REGISTRYINDEX, refnum);
	if (!lua_istable(L, -1)) {
		isValid = false;
		parser->currentRef = LUA_NOREF;
		lua_settop(L, 0);
		return false;
	}

	parser->currentRef = refnum;

	return true;
}


bool LuaTable::PushValue(int key) const
{
	if (!PushTable()) {
		return false;
	}
	lua_pushinteger(L, key);
	lua_gettable(L, -2);
	if (lua_isnoneornil(L, -1)) {
		lua_pop(L, 1);
		return false;
	}
	return true;	
}


bool LuaTable::PushValue(const string& mixedKey) const
{
	const string key = !lowerCppKeys ? mixedKey : StringToLower(mixedKey);
	if (!PushTable()) {
		return false;
	}
	lua_pushstring(L, key.c_str());
	lua_gettable(L, -2);
	if (lua_isnoneornil(L, -1)) {
		lua_pop(L, 1);
		return false;
	}
	return true;	
}


/******************************************************************************/
/******************************************************************************/
//
//  Key existance testing
//

bool LuaTable::KeyExists(int key) const
{
	if (!PushValue(key)) {
		return false;
	}
	lua_pop(L, 1);
	return true;
}


bool LuaTable::KeyExists(const string& key) const
{
	if (!PushValue(key)) {
		return false;
	}
	lua_pop(L, 1);
	return true;
}


/******************************************************************************/
/******************************************************************************/
//
//  Value types
//

int LuaTable::GetType(int key) const
{
	if (!PushValue(key)) {
		return -1;
	}
	const int type = lua_type(L, -1);
	lua_pop(L, 1);
	return type;
}


int LuaTable::GetType(const string& key) const
{
	if (!PushValue(key)) {
		return -1;
	}
	const int type = lua_type(L, -1);
	lua_pop(L, 1);
	return type;
}


/******************************************************************************/
/******************************************************************************/
//
//  Object lengths
//

int LuaTable::GetLength() const
{
	if (!PushTable()) {
		return 0;
	}
	return lua_objlen(L, -1);
}


int LuaTable::GetLength(int key) const
{
	if (!PushValue(key)) {
		return 0;
	}
	const int len = lua_objlen(L, -1);
	lua_pop(L, 1);
	return len;
}


int LuaTable::GetLength(const string& key) const
{
	if (!PushValue(key)) {
		return 0;
	}
	const int len = lua_objlen(L, -1);
	lua_pop(L, 1);
	return len;
}


/******************************************************************************/
/******************************************************************************/
//
//  Key list functions
//

bool LuaTable::GetKeys(vector<int>& data) const
{
	if (!PushTable()) {
		return false;
	}
	const int table = lua_gettop(L);
	for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
		if (lua_israwnumber(L, -2)) {
			const int value = lua_toint(L, -2);
			data.push_back(value);
		}
	}
	std::sort(data.begin(), data.end());
	return true;
}


bool LuaTable::GetKeys(vector<string>& data) const
{
	if (!PushTable()) {
		return false;
	}
	const int table = lua_gettop(L);
	for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
		if (lua_israwstring(L, -2)) {
			const string value = lua_tostring(L, -2);
			data.push_back(value);
		}
	}
	std::sort(data.begin(), data.end());
	return true;
}


/******************************************************************************/
/******************************************************************************/
//
//  Map functions
//

bool LuaTable::GetMap(map<int, float>& data) const
{
	if (!PushTable()) {
		return false;
	}
	const int table = lua_gettop(L);
	for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
		if (lua_israwnumber(L, -2) && lua_israwnumber(L, -1)) {
			const int   key   =   lua_toint(L, -2);
			const float value = lua_tofloat(L, -1);
			data[key] = value;
		}
	}
	return true;
}


bool LuaTable::GetMap(map<int, string>& data) const
{
	if (!PushTable()) {
		return false;
	}
	const int table = lua_gettop(L);
	for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
		if (lua_israwnumber(L, -2) && lua_israwstring(L, -1)) {
			const int    key   = lua_toint(L, -2);
			const string value = lua_tostring(L, -1);
			data[key] = value;
		}
	}
	return true;
}


bool LuaTable::GetMap(map<string, float>& data) const
{
	if (!PushTable()) {
		return false;
	}
	const int table = lua_gettop(L);
	for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
		if (lua_israwstring(L, -2) && lua_israwnumber(L, -1)) {
			const string key   = lua_tostring(L, -2);
			const float  value = lua_tofloat(L, -1);
			data[key] = value;
		}
	}
	return true;
}


bool LuaTable::GetMap(map<string, string>& data) const
{
	if (!PushTable()) {
		return false;
	}
	const int table = lua_gettop(L);
	for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
		if (lua_israwstring(L, -2) && lua_israwstring(L, -1)) {
			const string key   = lua_tostring(L, -2);
			const string value = lua_tostring(L, -1);
			data[key] = value;
		}
	}
	return true;
}


/******************************************************************************/
/******************************************************************************/
//
//  Parsing utilities
//

/*
static bool ParseTableFloat(lua_State* L,
                            int tableIndex, int index, float& value)
{
	lua_pushnumber(L, index);
	lua_gettable(L, tableIndex);
	if (!lua_israwnumber(L, -1)) {
		lua_pop(L, 1);
		return false;
	}
	value = lua_tofloat(L, -1);
	lua_pop(L, 1);
	return true;
}
*/


static bool ParseBoolean(lua_State* L, int index, bool& value)
{
	const int type = lua_type(L, index);
	if (type == LUA_TBOOLEAN) {
		value = lua_tobool(L, index);
		return true;
	}
	else if (type == LUA_TNUMBER) {
		value = (lua_tofloat(L, index) != 0.0f);
		return true;
	}
	else if (type == LUA_TSTRING) {
		const string str = StringToLower(lua_tostring(L, index));
		if ((str == "1") || (str == "true")) {
			value = true;
			return true;
		}
		if ((str == "0") || (str == "false")) {
			value = false;
			return true;
		}
	}
	return false;
}


/******************************************************************************/
/******************************************************************************/
//
//  String key functions
//

int LuaTable::GetInt(const string& key, int def) const
{
	if (!PushValue(key)) {
		return def;
	}
	if (!lua_israwnumber(L, -1)) {
		lua_pop(L, 1);
		return def;
	}
	const int value = lua_toint(L, -1);
	lua_pop(L, 1);
	return value;
}


bool LuaTable::GetBool(const string& key, bool def) const
{
	if (!PushValue(key)) {
		return def;
	}
	bool value;
	if (!ParseBoolean(L, -1, value)) {
		lua_pop(L, 1);
		return def;
	}
	lua_pop(L, 1);
	return value;
}


float LuaTable::GetFloat(const string& key, float def) const
{
	if (!PushValue(key)) {
		return def;
	}
	if (!lua_israwnumber(L, -1)) {
		lua_pop(L, 1);
		return def;
	}
	const float value = lua_tofloat(L, -1);
	lua_pop(L, 1);
	return value;
}


string LuaTable::GetString(const string& key, const string& def) const
{
	if (!PushValue(key)) {
		return def;
	}
	if (!lua_israwstring(L, -1)) {
		lua_pop(L, 1);
		return def;
	}
	const string value = lua_tostring(L, -1);
	lua_pop(L, 1);
	return value;
}


/******************************************************************************/
/******************************************************************************/
//
//  Number key functions
//

int LuaTable::GetInt(int key, int def) const
{
	if (!PushValue(key)) {
		return def;
	}
	if (!lua_israwnumber(L, -1)) {
		lua_pop(L, 1);
		return def;
	}
	const int value = lua_toint(L, -1);
	lua_pop(L, 1);
	return value;
}


bool LuaTable::GetBool(int key, bool def) const
{
	if (!PushValue(key)) {
		return def;
	}
	bool value;
	if (!ParseBoolean(L, -1, value)) {
		lua_pop(L, 1);
		return def;
	}
	lua_pop(L, 1);
	return value;
}


float LuaTable::GetFloat(int key, float def) const
{
	if (!PushValue(key)) {
		return def;
	}
	if (!lua_israwnumber(L, -1)) {
		lua_pop(L, 1);
		return def;
	}
	const float value = lua_tofloat(L, -1);
	lua_pop(L, 1);
	return value;
}


string LuaTable::GetString(int key, const string& def) const
{
	if (!PushValue(key)) {
		return def;
	}
	if (!lua_israwstring(L, -1)) {
		lua_pop(L, 1);
		return def;
	}
	const string value = lua_tostring(L, -1);
	lua_pop(L, 1);
	return value;
}


/******************************************************************************/
/******************************************************************************/

static int lowerKeysTable = 0;


static bool LowerKeysCheck(lua_State* L, int table)
{
	bool used = false;
	lua_pushvalue(L, table);
	lua_rawget(L, lowerKeysTable);
	if (lua_isnil(L, -1)) {
		used = false;
		lua_pushvalue(L, table);
		lua_pushboolean(L, true);
		lua_rawset(L, lowerKeysTable);
	}
	lua_pop(L, 1);
	return used;
}


static bool LowerKeysReal(lua_State* L, int depth)
{
	lua_checkstack(L, 4);

	const int table = lua_gettop(L);
	if (LowerKeysCheck(L, table)) {
		return true;
	}

	// a new table for changed values
	const int changed = table + 1;
	lua_newtable(L);

	for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
		if (lua_istable(L, -1)) {
			LowerKeysReal(L, depth + 1);
		}
		if (lua_israwstring(L, -2)) {
			const string rawKey = lua_tostring(L, -2);
			const string lowerKey = StringToLower(rawKey);
			if (rawKey != lowerKey) {
				// removed the mixed case entry
				lua_pushvalue(L, -2); // the key
				lua_pushnil(L);
				lua_rawset(L, table);
				// does the lower case key alread exist in the table?
				lua_pushstring(L, lowerKey.c_str());
				lua_rawget(L, table);
				if (lua_isnil(L, -1)) {
					// lower case does not exist, add it to the changed table
					lua_pushstring(L, lowerKey.c_str());
					lua_pushvalue(L, -3); // the value
					lua_rawset(L, changed);
				}
				lua_pop(L, 1);
			}
		}
	}

	// copy the changed values into the table
	for (lua_pushnil(L); lua_next(L, changed) != 0; lua_pop(L, 1)) {
		lua_pushvalue(L, -2); // copy the key to the top
		lua_pushvalue(L, -2); // copy the value to the top
		lua_rawset(L, table);		
	}

	lua_pop(L, 1); // pop the changed table

	return true;
}


bool LuaParser::LowerKeys(lua_State* L, int table)
{
	if (!lua_istable(L, table)) {
		return false;
	}

	// table of processed tables
	lowerKeysTable = lua_gettop(L) + 1;
	lua_checkstack(L, 4);
	lua_newtable(L);

	lua_pushvalue(L, table); // push the table onto the top of the stack

	LowerKeysReal(L, 0);

	lua_pop(L, 2); // the lowered table, and the check table

	return true;
}


/******************************************************************************/
/******************************************************************************/
