noinst_LTLIBRARIES = libSceneDB.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/src/include @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libSceneDB_la_LDFLAGS = -static
libSceneDB_la_SOURCES =			\
	Occluder.cpp			\
	Occluder.h			\
	Octree.cpp			\
	Octree.h			\
	SceneDatabase.cpp		\
	ZSceneDatabase.cpp		\
	BSPSceneDatabase.cpp

EXTRA_DIST = \
	README
