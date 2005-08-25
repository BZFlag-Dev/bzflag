if CLIENT_INCLUDED
noinst_LTLIBRARIES = libSceneDB.la
endif

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include

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
