lib_LTLIBRARIES = customPollOptionSample.la

customPollOptionSample_la_SOURCES = customPollOptionSample.cpp
customPollOptionSample_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
customPollOptionSample_la_LDFLAGS = -module -avoid-version -shared
customPollOptionSample_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.customPollOptionSample.txt \
	customPollOptionSample.def \
	customPollOptionSample.sln \
	customPollOptionSample.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
