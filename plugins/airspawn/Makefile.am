lib_LTLIBRARIES = airspawn.la

AM_CPPFLAGS = @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

airspawn_la_SOURCES = airspawn.cpp
airspawn_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	airspawn.def \
	airspawn.sln \
	airspawn.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
