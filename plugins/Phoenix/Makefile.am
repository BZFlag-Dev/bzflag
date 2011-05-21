lib_LTLIBRARIES = Phoenix.la

AM_CPPFLAGS = @CONF_CPPFLAGS@ -I$(top_srcdir)/../src/include
AM_CXXFLAGS = @CONF_CXXFLAGS@

Phoenix_la_SOURCES = Phoenix.cpp
Phoenix_la_LDFLAGS = -module -avoid-version
Phoenix_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
Phoenix_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	Phoenix.def \
	Phoenix.sln \
	Phoenix.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
