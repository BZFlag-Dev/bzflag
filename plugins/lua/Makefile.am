lib_LTLIBRARIES = lua.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I../../src/other/lua/src
AM_CXXFLAGS = @CONF_CXXFLAGS@ -I../../src/other/lua/src

lua_la_SOURCES =      \
	lua.cpp       \
	callins.cpp   \
	callouts.cpp  \
	constants.cpp \
	mapobject.cpp \
	bzdb.cpp

lua_la_LDFLAGS = -module -avoid-version

lua_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils

lua_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la \
                 $(top_builddir)/../src/other/lua/liblua.la

EXTRA_DIST = \
	README.txt \
	bzfs.lua \
	lua.def \
	lua.sln \
	lua.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
