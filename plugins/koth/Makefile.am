lib_LTLIBRARIES = koth.la

koth_la_SOURCES = koth.cpp
koth_la_CPPFLAGS = -I$(top_srcdir)/include
koth_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.koth.txt \
	help_koth_admin.txt \
	help_koth_rules.txt \
	koth.sln \
	koth.vcxproj	\
	koth.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
