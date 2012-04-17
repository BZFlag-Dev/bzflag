lib_LTLIBRARIES = teamflagreset.la

teamflagreset_la_SOURCES = teamflagreset.cpp
teamflagreset_la_CPPFLAGS = -I$(top_srcdir)/include
teamflagreset_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.txt \
	teamflagreset.def \
	teamflagreset.sln \
	teamflagreset.vcxproj	\
	tfr_admin.txt

MAINTAINERCLEANFILES =	\
	Makefile.in
