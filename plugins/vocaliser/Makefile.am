lib_LTLIBRARIES = vocaliser.la

vocaliser_la_SOURCES = vocaliser.cpp
vocaliser_la_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
vocaliser_la_CXXFLAGS = @CONF_CXXFLAGS@
vocaliser_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	vocaliser.def \
	vocaliser.sln \
	vocaliser.vcproj \
	voices.txt

MAINTAINERCLEANFILES =	\
	Makefile.in
