lib_LTLIBRARIES = httpTest.la

httpTest_la_SOURCES = httpTest.cpp
httpTest_la_LDFLAGS = -module -avoid-version
httpTest_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
httpTest_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	httpTest.def \
	httpTest.sln \
	httpTest.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
