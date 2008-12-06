lib_LTLIBRARIES = lua.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I../../src/other/lua/src
AM_CXXFLAGS = @CONF_CXXFLAGS@ -I../../src/other/lua/src

noinst_HEADERS = \
	bzdb.h        \
	callins.h     \
	callouts.h    \
	constants.h   \
	fetchurl.h    \
	lualfs.h      \
	mapobject.h   \
	mylua.h       \
	slashcmd.h

lua_la_SOURCES =      \
	bzdb.cpp      \
	callins.cpp   \
	callouts.cpp  \
	constants.cpp \
	fetchurl.cpp  \
	lua.cpp       \
	lualfs.cpp    \
	mapobject.cpp \
	slashcmd.cpp

lua_la_LDFLAGS = -module -avoid-version

lua_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils

lua_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la \
                 $(top_builddir)/../src/other/lua/liblua.la

EXTRA_DIST = \
	README.txt \
	example \
	lua.def \
	lua.sln \
	lua.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
