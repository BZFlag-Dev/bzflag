
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


const int bufSize = 65536;

enum CompressMode {
	CompressRaw  = 0,
	CompressGzip = 1,
	CompressZlib = 2
};


/******************************************************************************/
/******************************************************************************/

bool LuaZip::PushEntries(lua_State* L)
{
	PUSH_LUA_CFUNC(L, Zip);
	PUSH_LUA_CFUNC(L, Unzip);

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


static char* ConcatVector(const vector<string>& vs, size_t total)
{
	char* data = new char[total];
	char* pos = data;
	for (size_t i = 0; i < vs.size(); i++) {
		const string& str = vs[i]; 
		memcpy(pos, str.data(), str.size());
		pos += str.size();
	}
	return data;
}


static void SetupZStream(z_stream& s, const char* inData, size_t inLen)
{
	s.next_in = (Bytef*)inData;
	s.avail_in = inLen;
	s.total_in = inLen;
	s.total_out = 0;
	s.zalloc = (alloc_func)Z_NULL;
	s.zfree  =  (free_func)Z_NULL;
	s.opaque =     (voidpf)Z_NULL;
}


CompressMode ParseCompressMode(lua_State* L, int index, const char* def)
{
	const string modeStr = luaL_optstring(L, index, def);
	if (modeStr == "raw")  { return CompressRaw;  }
	if (modeStr == "gzip") { return CompressGzip; }
	if (modeStr == "zlib") { return CompressZlib; }
	luaL_error(L, "invalid zip mode");
	return CompressRaw;
}


/******************************************************************************/
/******************************************************************************/

static int CompressString(const char* inData, size_t inLen,
                          char*& outData, size_t& outLen,
                          CompressMode mode)
{
	z_stream s;
	SetupZStream(s, inData, inLen);

	int windowBits;
	switch (mode) {
		case CompressRaw:  { windowBits = -MAX_WBITS;      break; }
		case CompressGzip: { windowBits = +MAX_WBITS + 16; break; }
		case CompressZlib: { windowBits = +MAX_WBITS;      break; }
	}

	const int initCode = deflateInit2(&s, 9, Z_DEFLATED, windowBits,
	                                  MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
	if (initCode != Z_OK) {
		return initCode;
	}

	outLen = 0;
	vector<string> outVec;
	char bufData[bufSize];

	int flushMode = Z_NO_FLUSH;
	while (true) {
		s.next_out = (Bytef*)bufData;
		s.avail_out = bufSize;
		const int retcode = deflate(&s, flushMode);
		if (retcode == Z_STREAM_END) {
			const size_t outSize = (bufSize - s.avail_out);
			outLen += outSize;
			outVec.push_back(string(bufData, outSize));
			break;
		}
		else if (retcode == Z_OK) {
			flushMode = Z_FINISH;
			const size_t outSize = (bufSize - s.avail_out);
			outLen += outSize;
			outVec.push_back(string(bufData, outSize));
		}
		else {
			deflateEnd(&s);
			outLen = 0;
			outData = NULL;
			return retcode;
		}
	}

	deflateEnd(&s);

	outData = ConcatVector(outVec, outLen);

	return Z_OK;
}


static int DecompressString(const char* inData, size_t inLen,
                            char*& outData, size_t& outLen,
                            CompressMode mode)
{
	z_stream s;
	SetupZStream(s, inData, inLen);

	int windowBits;
	switch (mode) {
		case CompressRaw:  { windowBits = -MAX_WBITS;      break; }
		case CompressGzip: { windowBits = +MAX_WBITS + 16; break; }
		case CompressZlib: { windowBits = +MAX_WBITS + 32; break; } // zlib or gzip
	}

	const int initCode = inflateInit2(&s, windowBits);
	if (initCode != Z_OK) {
		return initCode;
	}

	outLen = 0;
	vector<string> outVec;
	char bufData[bufSize];

	while (true) {
		s.next_out = (Bytef*)bufData;
		s.avail_out = bufSize;
		const int retcode = inflate(&s, Z_NO_FLUSH);
		if (retcode == Z_STREAM_END) {
			const size_t outSize = (bufSize - s.avail_out);
			outLen += outSize;
			outVec.push_back(string(bufData, outSize));
			break;
		}
		else if (retcode == Z_OK) {
			const size_t outSize = (bufSize - s.avail_out);
			outLen += outSize;
			outVec.push_back(string(bufData, outSize));
		}
		else {
			inflateEnd(&s);
			outLen = 0;
			outData = NULL;
			return retcode;
		}
	}

	inflateEnd(&s);

	outData = ConcatVector(outVec, outLen);

	return Z_OK;
}


/******************************************************************************/
/******************************************************************************/

int LuaZip::Zip(lua_State* L)
{
	size_t inLen;
	const char* inData = luaL_checklstring(L, 1, &inLen);
	CompressMode mode = ParseCompressMode(L, 2, "gzip");
	
	char* outData = NULL;
	size_t outLen;
	const int zCode = CompressString(inData, inLen, outData, outLen, mode);
	if (zCode != Z_OK) {
		lua_pushnil(L);
		PushZlibErrorString(L, zCode);
		return 2;
	}

	lua_pushlstring(L, outData, outLen);
	delete[] outData;

	return 1;
}


int LuaZip::Unzip(lua_State* L)
{
	size_t inLen;
	const char* inData = luaL_checklstring(L, 1, &inLen);
	CompressMode mode = ParseCompressMode(L, 2, "zlib"); // zlib or gzip

	char* outData = NULL;
	size_t outLen;
	const int zCode = DecompressString(inData, inLen, outData, outLen, mode);
	if (zCode != Z_OK) {
		lua_pushnil(L);
		PushZlibErrorString(L, zCode);
		return 2;
	}

	lua_pushlstring(L, outData, outLen);
	delete[] outData;

	return 1;
}


/******************************************************************************/
/******************************************************************************/
