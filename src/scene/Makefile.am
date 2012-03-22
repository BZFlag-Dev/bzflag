if CLIENT_INCLUDED
noinst_LTLIBRARIES = libSceneDB.la
endif

AM_CPPFLAGS = $(CONF_CPPFLAGS) -I$(top_srcdir)/include
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES = \
	Makefile.in

libSceneDB_la_LDFLAGS = -static
libSceneDB_la_SOURCES =			\
	Occluder.cxx			\
	Occluder.h			\
	Octree.cxx			\
	Octree.h			\
	SceneDatabase.cxx		\
	ZSceneDatabase.cxx		\
	BSPSceneDatabase.cxx

EXTRA_DIST = \
	README
