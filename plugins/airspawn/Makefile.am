lib_LTLIBRARIES = airspawn.la

airspawn_la_SOURCES = airspawn.cpp
airspawn_la_CPPFLAGS = -I$(top_srcdir)/include
airspawn_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	airspawn.def \
	airspawn.sln \
	airspawn.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
