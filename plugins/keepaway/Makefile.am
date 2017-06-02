lib_LTLIBRARIES = keepaway.la

keepaway_la_SOURCES = keepaway.cpp
keepaway_la_CPPFLAGS = -I$(top_srcdir)/include
keepaway_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.keepaway.txt \
	help_keepaway_admin.txt \
	help_keepaway_rules.txt \
	keepaway.sln \
	keepaway.vcxproj	\
	keepaway.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
