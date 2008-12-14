lib_LTLIBRARIES = libbzfslua.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I../../src/other/lua/src
AM_CXXFLAGS = @CONF_CXXFLAGS@ -I../../src/other/lua/src

noinst_HEADERS =      \
	bzdb.h        \
	callins.h     \
	callouts.h    \
	constants.h   \
	fetchurl.h    \
	lualfs.h      \
	mapobject.h   \
	mylua.h       \
	rawlink.h     \
	slashcmd.h

libbzfslua_la_SOURCES = \
	bzdb.cpp      \
	callins.cpp   \
	callouts.cpp  \
	constants.cpp \
	fetchurl.cpp  \
	lua.cpp       \
	lualfs.cpp    \
	mapobject.cpp \
	rawlink.cpp   \
	slashcmd.cpp

libbzfslua_la_LDFLAGS = -module -avoid-version

libbzfslua_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils

libbzfslua_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la \
                        $(top_builddir)/../src/other/lua/liblua.la

EXTRA_DIST =       \
	README.txt \
	example \
	bzfslua.def \
	lua.sln \
	lua.vcproj

MAINTAINERCLEANFILES = \
	Makefile.in
