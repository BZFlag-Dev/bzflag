lib_LTLIBRARIES = customflagsample.la

customflagsample_la_SOURCES = customflagsample.cpp
customflagsample_la_LDFLAGS = -module -avoid-version
customflagsample_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
customflagsample_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	customflagsample.def \
	customflagsample.sln \
	customflagsample.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
