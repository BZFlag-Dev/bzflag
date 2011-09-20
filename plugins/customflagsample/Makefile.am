lib_LTLIBRARIES = customflagsample.la

customflagsample_la_SOURCES = customflagsample.cpp
customflagsample_la_LDFLAGS = -module -avoid-version -shared
customflagsample_la_CPPFLAGS= -I$(top_srcdir)/plugins/plugin_utils -I$(top_srcdir)/include
customflagsample_la_LIBADD  = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	customflagsample.def \
	customflagsample.sln \
	customflagsample.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
