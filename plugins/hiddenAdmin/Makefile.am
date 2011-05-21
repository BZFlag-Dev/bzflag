lib_LTLIBRARIES = hiddenAdmin.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I$(top_srcdir)/../src/include
AM_CXXFLAGS = @CONF_CXXFLAGS@

hiddenAdmin_la_SOURCES = hiddenAdmin.cpp
hiddenAdmin_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	hiddenAdmin.def \
	hiddenAdmin.sln \
	hiddenAdmin.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
