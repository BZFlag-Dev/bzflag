lib_LTLIBRARIES = fastmap.la

fastmap_la_SOURCES = fastmap.cpp
fastmap_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS) -I$(top_srcdir)/include
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.fastmap.txt \
	fastmap.sln \
	fastmap.vcxproj	\
	fastmap.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
