lib_LTLIBRARIES = airspawn.la

airspawn_la_SOURCES = airspawn.cpp
airspawn_la_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
airspawn_la_CXXFLAGS = @CONF_CXXFLAGS@
airspawn_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	airspawn.def \
	airspawn.sln \
	airspawn.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
