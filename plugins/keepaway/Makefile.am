lib_LTLIBRARIES = keepaway.la

keepaway_la_SOURCES = keepaway.cpp
keepaway_la_CPPFLAGS = -I$(top_srcdir)/include
keepaway_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.txt \
	keepaway.def \
	keepaway.sln \
	keepaway.vcxproj		\
	keepaway_admin.txt	\
	keepaway_rules.txt

MAINTAINERCLEANFILES =	\
	Makefile.in
