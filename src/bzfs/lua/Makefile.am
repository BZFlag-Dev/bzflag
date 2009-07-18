noinst_LTLIBRARIES = libLuaServer.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libLuaServer_la_LDFLAGS = -static

noinst_HEADERS =      \
	BZDB.h        \
	CallIns.h     \
	CallOuts.h    \
	Constants.h   \
	LuaFloat.h    \
	LuaHeader.h   \
	LuaServer.h   \
	LuaURL.h      \
	MapObject.h   \
	Obstacles.h   \
	RawLink.h     \
	SlashCmd.h

libLuaServer_la_SOURCES = \
	BZDB.cxx       \
	CallIns.cxx    \
	CallOuts.cxx   \
	Constants.cxx  \
	LuaFloat.cxx   \
	LuaServer.cxx  \
	LuaURL.cxx     \
	MapObject.cxx  \
	Obstacles.cxx  \
	RawLink.cxx    \
	SlashCmd.cxx

EXTRA_DIST =       \
	README.txt

#
#	LuaSpatial.h  \
#	LuaSpatial.cxx \
#
