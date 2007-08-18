include $(top_srcdir)/Makefile.common

if CLIENT_INCLUDED
noinst_LTLIBRARIES = libSceneDB.la
endif

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
