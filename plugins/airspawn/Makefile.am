lib_LTLIBRARIES = airspawn.la

airspawn_la_SOURCES = airspawn.cpp
airspawn_la_CPPFLAGS = -I../../include
airspawn_la_LDFLAGS = -module -avoid-version ${top_builddir}/src/bzfs/libBZFSplugin.la

EXTRA_DIST = \
	airspawn.def \
	airspawn.sln \
	airspawn.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
