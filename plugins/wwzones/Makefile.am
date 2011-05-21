lib_LTLIBRARIES = wwzones.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I$(top_srcdir)/../src/include
AM_CXXFLAGS = @CONF_CXXFLAGS@

wwzones_la_SOURCES = wwzones.cpp
wwzones_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	README.txt \
	wwzones.def \
	wwzones.sln \
	wwzones.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
