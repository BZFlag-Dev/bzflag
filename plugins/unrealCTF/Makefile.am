lib_LTLIBRARIES = unrealCTF.la

unrealCTF_la_SOURCES = unrealCTF.cpp
unrealCTF_la_LDFLAGS = -module -avoid-version
unrealCTF_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
unrealCTF_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	unrealCTF.def \
	unrealCTF.sln \
	unrealCTF.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
