lib_LTLIBRARIES = torBlock.la

AM_CPPFLAGS = @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

torBlock_la_SOURCES = torBlock.cpp
torBlock_la_LDFLAGS = -module -avoid-version
torBlock_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
torBlock_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la


EXTRA_DIST = \
	README.txt \
	torBlock.def \
	torBlock.sln \
	torBlock.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
