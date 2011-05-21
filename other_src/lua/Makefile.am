
noinst_LTLIBRARIES = liblua.la

if BUILD_LUA_EXECS
bin_PROGRAMS = bzlua bzluac
bzlua_DEPENDENCIES  = liblua.la
bzluac_DEPENDENCIES = liblua.la
bzlua_LDFLAGS  = -static
bzluac_LDFLAGS = -static
bzlua_SOURCES  = src/lua.cpp
bzluac_SOURCES = src/luac.cpp
bzlua_LDADD  = $(LIBLUA) -lreadline -ldl
bzluac_LDADD = $(LIBLUA) -lreadline -ldl
endif

# works for linux and macos
AM_CXXFLAGS = @CONF_CXXFLAGS@ -fexceptions -DLUA_USE_LINUX

liblua_la_LDFLAGS = -static

liblua_la_SOURCES =      \
	src/lapi.cpp     \
	src/lauxlib.cpp  \
	src/lbaselib.cpp \
	src/lcode.cpp    \
	src/ldblib.cpp   \
	src/ldebug.cpp   \
	src/ldo.cpp      \
	src/ldump.cpp    \
	src/lfunc.cpp    \
	src/lgc.cpp      \
	src/linit.cpp    \
	src/liolib.cpp   \
	src/llex.cpp     \
	src/lpeg.cpp     \
	src/lmathlib.cpp \
	src/lmem.cpp     \
	src/loadlib.cpp  \
	src/lobject.cpp  \
	src/lopcodes.cpp \
	src/loslib.cpp   \
	src/lparser.cpp  \
	src/lstate.cpp   \
	src/lstring.cpp  \
	src/lstrlib.cpp  \
	src/ltable.cpp   \
	src/ltablib.cpp  \
	src/ltm.cpp      \
	src/lundump.cpp  \
	src/lvm.cpp      \
	src/lzio.cpp     \
	src/print.cpp

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
	src/lua.cpp        \
	src/luac.cpp       \
	README_BZFLAG

MAINTAINERCLEANFILES = Makefile.in

