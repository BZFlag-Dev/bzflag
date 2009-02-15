
#include "common.h"

// interface header
#include "LuaZip.h"

// system headers
#include <errno.h>
#include <string.h>
#include <string>
#include <vector>
using std::string;
using std::vector;

// common headers
#include "zlib.h"

// local headers
#include "LuaInclude.h"
#include "LuaHashString.h"


/******************************************************************************/
/******************************************************************************/

bool LuaZip::PushEntries(lua_State* L)
{
	PUSH_LUA_CFUNC(L, Zip);
	PUSH_LUA_CFUNC(L, Unzip);
	PUSH_LUA_CFUNC(L, Gzip);
	PUSH_LUA_CFUNC(L, Gunzip);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static void PushZlibErrorString(lua_State* L, int zCode)
{
	switch (zCode) {
		case Z_BUF_ERROR:     { lua_pushliteral(L, "buffer error");  break; }
		case Z_DATA_ERROR:    { lua_pushliteral(L, "data error");    break; }
		case Z_MEM_ERROR:     { lua_pushliteral(L, "memory error");  break; }
		case Z_STREAM_ERROR:  { lua_pushliteral(L, "stream error");  break; }
		case Z_VERSION_ERROR: { lua_pushliteral(L, "version error"); break; }
		case Z_ERRNO: {
			lua_pushstring(L, strerror(errno));
			break;
		}
		default: {
			lua_pushliteral(L, "unknown error");
			break;
		}
	}
}


/******************************************************************************/
/******************************************************************************/

/* FIXME
static int compressString(const string& input, string& output,
                          bool gzip, const string& filename)
{
	z_stream s;
	s.next_in = (Bytef*)input.data();
	s.avail_in = input.size();
	s.total_in = 0;
	s.total_out = 0;
	s.zalloc = (alloc_func)Z_NULL;
	s.zfree  =  (free_func)Z_NULL;
	s.opaque =     (voidpf)Z_NULL;

	const int windowBits = gzip ? -MAX_WBITS : +MAX_WBITS;
	deflateInit2(&s, 9, Z_DEFLATED, windowBits, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);

	vector<string> outVec;
	char outData[16384];
	size_t total = 0;

	while (true) {
		s.next_out = (Bytef*)outData;
		s.avail_out = sizeof(outData);
		const int retcode = deflate(&s, Z_FINISH);
		if (retcode == Z_STREAM_END) {
			total += s.avail_out;
			outVec.push_back(string(outData, s.avail_out));
			break;
		}
		else if (retcode == Z_OK) {
			total += s.avail_out;
			outVec.push_back(string(outData, s.avail_out));
		}
		else {
			return retcode;
		}
	}

	output.resize(total);
	size_t offset = 0;
	for (size_t i = 0; i < outVec.size(); i++) {
		const string& str = outVec[i]; 
		output.assign(str, str.size(), offset);
		offset += str.size();
	}

	return Z_OK;
}


static int decompressString(const string& input, string& output, bool gzip)
{
}
*/

/******************************************************************************/
/******************************************************************************/

int LuaZip::Zip(lua_State* L)
{
	size_t srcLen;
	const char* src = luaL_checklstring(L, 1, &srcLen);

	uLongf dstLen = compressBound(srcLen);
	char* dst = new char[dstLen];
	const int zCode = compress2((Bytef*)dst, &dstLen, (Bytef*)src, srcLen, 9);
	if (zCode != Z_OK) {
		lua_pushnil(L);
		PushZlibErrorString(L, zCode);
		delete[] dst;
		return 2;
	}

	lua_pushlstring(L, dst, dstLen);
	delete[] dst;
	return 1;
}


int LuaZip::Unzip(lua_State* L)
{
	size_t srcLen;
	const char* src = luaL_checklstring(L, 1, &srcLen);

	uLongf dstLen = luaL_optint(L, 2, srcLen * 10); // FIXME -- use a stream
	char* dst = new char[dstLen];
	
	const int zCode = uncompress((Bytef*)dst, &dstLen, (Bytef*)src, srcLen);
	if (zCode != Z_OK) {
		lua_pushnil(L);
		PushZlibErrorString(L, zCode);
		delete[] dst;
		return 2;
	}

	lua_pushlstring(L, dst, dstLen);
	delete[] dst;
	return 1;
}


int LuaZip::Gzip(lua_State* L)
{
	L = L; // FIXME
	return 0;
}


int LuaZip::Gunzip(lua_State* L)
{
	L = L; // FIXME
	return 0;
}


/******************************************************************************/
/******************************************************************************/
