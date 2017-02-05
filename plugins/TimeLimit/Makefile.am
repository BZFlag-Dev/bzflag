lib_LTLIBRARIES = TimeLimit.la

TimeLimit_la_SOURCES = TimeLimit.cpp
TimeLimit_la_CPPFLAGS = -I$(top_srcdir)/include
TimeLimit_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.TimeLimit.txt \
	TimeLimit.sln \
	TimeLimit.vcxproj	\
	TimeLimit.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
