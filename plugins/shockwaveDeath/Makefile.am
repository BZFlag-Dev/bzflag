lib_LTLIBRARIES = shockwaveDeath.la

shockwaveDeath_la_SOURCES = shockwaveDeath.cpp
shockwaveDeath_la_CPPFLAGS = -I../../include
shockwaveDeath_la_LDFLAGS = -module -avoid-version ${top_builddir}/src/bzfs/libBZFSplugin.la

EXTRA_DIST = \
	shockwaveDeath.def \
	shockwaveDeath.sln \
	shockwaveDeath.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
