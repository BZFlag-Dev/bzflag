if CLIENT_INCLUDED
noinst_LIBRARIES = libSceneDB.a
endif

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include

libSceneDB_a_SOURCES =			\
	Occluder.cxx			\
	Occluder.h			\
	Octree.cxx			\
	Octree.h			\
	SceneDatabase.cxx		\
	ZSceneDatabase.cxx		\
	BSPSceneDatabase.cxx

EXTRA_DIST = \
	README
