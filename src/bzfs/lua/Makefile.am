noinst_LTLIBRARIES = libLuaBZFS.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libLuaBZFS_la_LDFLAGS = -static

noinst_HEADERS =      \
	BZDB.h        \
	CallIns.h     \
	CallOuts.h    \
	Constants.h   \
	FetchURL.h    \
	LuaBZFS.h     \
	LuaHeader.h   \
	LuaLFS.h      \
	MapObject.h   \
	RawLink.h     \
	SlashCmd.h

libLuaBZFS_la_SOURCES = \
	BZDB.cxx      \
	CallIns.cxx   \
	CallOuts.cxx  \
	Constants.cxx \
	FetchURL.cxx  \
	LuaBZFS.cxx   \
	LuaLFS.cxx    \
	MapObject.cxx \
	RawLink.cxx   \
	SlashCmd.cxx

EXTRA_DIST =       \
	README.txt \
	example
