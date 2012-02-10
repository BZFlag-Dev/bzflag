lib_LTLIBRARIES = fairCTF.la

fairCTF_la_SOURCES = fairCTF.cpp
fairCTF_la_CXXFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
fairCTF_la_LDFLAGS = -module -avoid-version -shared
fairCTF_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	fairCTF.def \
	fairCTF.sln \
	fairCTF.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
