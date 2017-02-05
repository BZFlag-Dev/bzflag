lib_LTLIBRARIES = nagware.la

nagware_la_SOURCES = nagware.cpp
nagware_la_CPPFLAGS = -I$(top_srcdir)/include
nagware_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.nagware.txt \
	nagware.cfg	\
	nagware.sln \
	nagware.vcxproj	\
	nagware.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
