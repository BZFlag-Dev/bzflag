lib_LTLIBRARIES = recordmatch.la

recordmatch_la_SOURCES = recordmatch.cpp
recordmatch_la_CPPFLAGS = -I$(top_srcdir)/include
recordmatch_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.recordmatch.txt \
	recordmatch.sln \
	recordmatch.vcxproj	\
	recordmatch.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
