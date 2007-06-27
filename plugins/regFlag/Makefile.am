lib_LTLIBRARIES = regFlag.la

regFlag_la_SOURCES = regFlag.cpp
regFlag_la_CXXFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
regFlag_la_LDFLAGS = -module -avoid-version
regFlag_la_LIBADD = $(top_srcdir)/plugins/plugin_utils/libplugin_utils.la


EXTRA_DIST = \
	README.txt \
	regFlag.def \
	regFlag.sln \
	regFlag.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
