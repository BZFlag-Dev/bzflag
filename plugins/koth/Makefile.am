lib_LTLIBRARIES = koth.la

koth_la_SOURCES = koth.cpp
koth_la_CPPFLAGS = -I$(top_srcdir)/include
koth_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.txt \
	koth.def \
	koth.sln \
	koth.vcxproj	\
	koth_admin.txt	\
	koth_rules.txt

MAINTAINERCLEANFILES =	\
	Makefile.in
