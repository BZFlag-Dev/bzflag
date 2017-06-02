lib_LTLIBRARIES = rabbitTimer.la

rabbitTimer_la_SOURCES = rabbitTimer.cpp
rabbitTimer_la_CPPFLAGS = -I$(top_srcdir)/include
rabbitTimer_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.rabbitTimer.txt \
	rabbitTimer.sln \
	rabbitTimer.vcxproj	\
	rabbitTimer.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
