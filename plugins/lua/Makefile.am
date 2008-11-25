lib_LTLIBRARIES = lua.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I../../src/other/lua/src
AM_CXXFLAGS = @CONF_CXXFLAGS@ -I../../src/other/lua/src

lua_la_SOURCES =      \
	mylua.h       \
	lua.cpp       \
	callins.h     \
	callins.cpp   \
	callouts.h    \
	callouts.cpp  \
	constants.h   \
	constants.cpp \
	mapobject.h   \
	mapobject.cpp \
	slashcmd.h    \
	slashcmd.cpp  \
	bzdb.h        \
	bzdb.cpp      \
	lualfs.h      \
	lualfs.cpp

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
