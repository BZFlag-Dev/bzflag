noinst_LTLIBRARIES = libLuaServer.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/lua \
	$(LIBCURL_CPPFLAGS) \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libLuaServer_la_LDFLAGS = -static

noinst_HEADERS =      \
	CallIns.h     \
	CallOuts.h    \
	Constants.h   \
	LuaServer.h   \
	MapObject.h   \
	RawLink.h     \
	SlashCmd.h

libLuaServer_la_SOURCES = \
	CallIns.cxx    \
	CallOuts.cxx   \
	Constants.cxx  \
	LuaServer.cxx  \
	MapObject.cxx  \
	RawLink.cxx    \
	SlashCmd.cxx

EXTRA_DIST =       \
	README.txt
