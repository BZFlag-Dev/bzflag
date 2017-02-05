lib_LTLIBRARIES = Phoenix.la

Phoenix_la_SOURCES = Phoenix.cpp
Phoenix_la_CPPFLAGS = -I$(top_srcdir)/include
Phoenix_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.Phoenix.txt \
	Phoenix.sln \
	Phoenix.vcxproj	\
	Phoenix.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
