lib_LTLIBRARIES = customPollTypeSample.la

customPollTypeSample_la_SOURCES = customPollTypeSample.cpp
customPollTypeSample_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
customPollTypeSample_la_LDFLAGS = -module -avoid-version -shared
customPollTypeSample_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.customPollTypeSample.txt \
	customPollTypeSample.def \
	customPollTypeSample.sln \
	customPollTypeSample.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
