lib_LTLIBRARIES = wwzones.la

wwzones_la_SOURCES = wwzones.cpp
wwzones_la_CPPFLAGS = -I$(top_srcdir)/include
wwzones_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.wwzones.txt \
	wwzones.sln \
	wwzones.vcxproj	\
	wwzones.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
