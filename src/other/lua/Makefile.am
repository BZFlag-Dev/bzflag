
noinst_LTLIBRARIES = liblua.la

if BUILD_LUA_EXECS
bin_PROGRAMS = bzlua bzluac
bzlua:	liblua.la
bzluac:	liblua.la
bzlua_LDFLAGS  = -static
bzluac_LDFLAGS = -static
bzlua_SOURCES  = src/lua.cxx
bzluac_SOURCES = src/luac.cxx
LDADD = $(LIBLUA) -lreadline
endif

# works for linux and macos
AM_CXXFLAGS = @CONF_CXXFLAGS@ -fexceptions -DLUA_USE_LINUX

liblua_la_LDFLAGS = -static

liblua_la_SOURCES =      \
	src/lapi.cxx     \
	src/lauxlib.cxx  \
	src/lbaselib.cxx \
	src/lcode.cxx    \
	src/ldblib.cxx   \
	src/ldebug.cxx   \
	src/ldo.cxx      \
	src/ldump.cxx    \
	src/lfunc.cxx    \
	src/lgc.cxx      \
	src/linit.cxx    \
	src/liolib.cxx   \
	src/llex.cxx     \
	src/lmathlib.cxx \
	src/lmem.cxx     \
	src/loadlib.cxx  \
	src/lobject.cxx  \
	src/lopcodes.cxx \
	src/loslib.cxx   \
	src/lparser.cxx  \
	src/lstate.cxx   \
	src/lstring.cxx  \
	src/lstrlib.cxx  \
	src/ltable.cxx   \
	src/ltablib.cxx  \
	src/ltm.cxx      \
	src/lundump.cxx  \
	src/lvm.cxx      \
	src/lzio.cxx     \
	src/print.cxx

noinst_HEADERS =       \
	src/lapi.h     \
	src/lauxlib.h  \
	src/lcode.h    \
	src/ldebug.h   \
	src/ldo.h      \
	src/lfunc.h    \
	src/lgc.h      \
	src/llex.h     \
	src/llimits.h  \
	src/lmem.h     \
	src/lobject.h  \
	src/lopcodes.h \
	src/lparser.h  \
	src/lstate.h   \
	src/lstring.h  \
	src/ltable.h   \
	src/ltm.h      \
	src/luaconf.h  \
	src/lua.h      \
	src/lua_extra.h \
	src/lualib.h   \
	src/lundump.h  \
	src/lvm.h      \
	src/lzio.h

EXTRA_DIST =               \
	COPYRIGHT          \
	HISTORY            \
	Makefile.orig      \
	doc                \
	etc                \
	test               \
	src/Makefile.orig  \
	src/lua.cxx        \
	src/luac.cxx       \
	README_BZFLAG

MAINTAINERCLEANFILES = Makefile.in

