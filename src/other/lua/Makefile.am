# $Id: Makefile.am 16712 2008-03-06 01:04:35Z brlcad $

noinst_LTLIBRARIES = liblua.la

if LINUX
AM_CXXFLAGS = @CONF_CXXFLAGS@ -DLUA_USE_LINUX
endif

if MACOS
AM_CXXFLAGS = @CONF_CXXFLAGS@ -DLUA_USE_MACOSX
endif

liblua_la_LDFLAGS = -static

liblua_la_SOURCES = \
	src/lapi.cxx \
	src/lauxlib.cxx \
	src/lbaselib.cxx \
	src/lcode.cxx \
	src/ldblib.cxx \
	src/ldebug.cxx \
	src/ldo.cxx \
	src/ldump.cxx \
	src/lfunc.cxx \
	src/lgc.cxx \
	src/linit.cxx \
	src/liolib.cxx \
	src/llex.cxx \
	src/lmathlib.cxx \
	src/lmem.cxx \
	src/loadlib.cxx \
	src/lobject.cxx \
	src/lopcodes.cxx \
	src/loslib.cxx \
	src/lparser.cxx \
	src/lstate.cxx \
	src/lstring.cxx \
	src/lstrlib.cxx \
	src/ltable.cxx \
	src/ltablib.cxx \
	src/ltm.cxx \
	src/lundump.cxx \
	src/lvm.cxx \
	src/lzio.cxx \
	src/print.cxx

noinst_HEADERS = \
	src/lapi.h \
	src/lauxlib.h \
	src/lcode.h \
	src/ldebug.h \
	src/ldo.h \
	src/lfunc.h \
	src/lgc.h \
	src/llex.h \
	src/llimits.h \
	src/lmem.h \
	src/lobject.h \
	src/lopcodes.h \
	src/lparser.h \
	src/lstate.h \
	src/lstring.h \
	src/ltable.h \
	src/ltm.h \
	src/luaconf.h \
	src/lua.h \
	src/lualib.h \
	src/lundump.h \
	src/lvm.h \
	src/lzio.h

EXTRA_DIST = \
	COPYRIGHT \
	HISTORY \
	Makefile.orig \
	src/Makefile.orig \
	src/lua.c \
	src/luac.c \
	README_BZFLAG

MAINTAINERCLEANFILES = Makefile.in

