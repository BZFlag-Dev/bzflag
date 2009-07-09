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
	FetchURL.h    \
	Float.h       \
	LuaServer.h   \
	LuaHeader.h   \
	MapObject.h   \
	Obstacles.h   \
	RawLink.h     \
	SlashCmd.h

libLuaServer_la_SOURCES = \
	BZDB.cxx      \
	CallIns.cxx   \
	CallOuts.cxx  \
	Constants.cxx \
	FetchURL.cxx  \
	Float.cxx     \
	LuaServer.cxx \
	MapObject.cxx \
	Obstacles.cxx \
	RawLink.cxx   \
	SlashCmd.cxx

EXTRA_DIST =       \
	README.txt
