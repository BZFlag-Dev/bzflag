lib_LTLIBRARIES = vocaliser.la

vocaliser_la_SOURCES = vocaliser.cpp
vocaliser_la_CPPFLAGS = -I../../include
vocaliser_la_LDFLAGS = -module -avoid-version ${top_builddir}/src/bzfs/libBZFSplugin.la

EXTRA_DIST = \
	vocaliser.def \
	vocaliser.sln \
	vocaliser.vcproj \
	voices.txt

MAINTAINERCLEANFILES =	\
	Makefile.in
