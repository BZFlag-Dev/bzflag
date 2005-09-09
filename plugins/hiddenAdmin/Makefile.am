lib_LTLIBRARIES = hiddenAdmin.la

hiddenAdmin_la_SOURCES = hiddenAdmin.cpp
hiddenAdmin_la_CPPFLAGS = -I../../include
hiddenAdmin_la_LDFLAGS = -module -avoid-version ${top_builddir}/src/bzfs/libBZFSplugin.la

EXTRA_DIST = \
	hiddenAdmin.def \
	hiddenAdmin.sln \
	hiddenAdmin.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
