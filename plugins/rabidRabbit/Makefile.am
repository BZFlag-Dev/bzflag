lib_LTLIBRARIES = rabidRabbit.la

rabidRabbit_la_SOURCES = rabidRabbit.cpp
rabidRabbit_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
rabidRabbit_la_LDFLAGS = -module -avoid-version
rabidRabbit_la_LIBADD = $(top_builddir)/plugin_utils/libplugin_utils.la


EXTRA_DIST = \
	README.txt \
	rabidRabbit.def \
	rabidRabbit.sln \
	rabidRabbit.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
