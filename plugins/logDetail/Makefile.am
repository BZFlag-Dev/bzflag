lib_LTLIBRARIES = logDetail.la

logDetail_la_SOURCES = logDetail.cpp
logDetail_la_CPPFLAGS = -I$(top_srcdir)/include
logDetail_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.logDetail.txt \
	logDetail.sln \
	logDetail.vcxproj	\
	logDetail.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
