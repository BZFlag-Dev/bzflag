lib_LTLIBRARIES = customflagsample.la

customflagsample_la_SOURCES = customflagsample.cpp
customflagsample_la_LDFLAGS = -module -avoid-version -shared
customflagsample_la_CPPFLAGS= -I$(top_srcdir)/plugins/plugin_utils -I$(top_srcdir)/include
customflagsample_la_LIBADD  = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.customflagsample.txt \
	customflagsample.sln \
	customflagsample.vcxproj	\
	customflagsample.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
