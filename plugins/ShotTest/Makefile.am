lib_LTLIBRARIES = ShotTest.la

AM_CPPFLAGS = @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

ShotTest_la_SOURCES = ShotTest.cpp
ShotTest_la_LDFLAGS = -module -avoid-version
ShotTest_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
ShotTest_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	ShotTest.def \
	ShotTest.sln \
	ShotTest.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
