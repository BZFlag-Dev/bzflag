
noinst_LTLIBRARIES = liblua.la

if LINUX
endif

if MACOS
endif

AM_CFLAGS = @CONF_CFLAGS@ -fPIC

liblua_la_LDFLAGS = -static

liblua_la_SOURCES =    \
	src/lapi.c     \
	src/lauxlib.c  \
	src/lbaselib.c \
	src/lcode.c    \
	src/ldblib.c   \
	src/ldebug.c   \
	src/ldo.c      \
	src/ldump.c    \
	src/lfunc.c    \
	src/lgc.c      \
	src/linit.c    \
	src/liolib.c   \
	src/llex.c     \
	src/lmathlib.c \
	src/lmem.c     \
	src/loadlib.c  \
	src/lobject.c  \
	src/lopcodes.c \
	src/loslib.c   \
	src/lparser.c  \
	src/lstate.c   \
	src/lstring.c  \
	src/lstrlib.c  \
	src/ltable.c   \
	src/ltablib.c  \
	src/ltm.c      \
	src/lundump.c  \
	src/lvm.c      \
	src/lzio.c     \
	src/print.c

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
	src/Makefile.orig  \
	src/lua.c.gz       \
	src/luac.c.gz      \
	README_BZFLAG

MAINTAINERCLEANFILES = Makefile.in

