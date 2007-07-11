noinst_LTLIBRARIES = libSceneDB.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

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
