lib_LTLIBRARIES = rabidRabbit.la

rabidRabbit_la_SOURCES = rabidRabbit.cpp
rabidRabbit_la_CPPFLAGS = -I$(top_srcdir)/include
rabidRabbit_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.rabidRabbit.txt \
	help_rabidRabbit.txt \
	rabidRabbit.sln \
	rabidRabbit.vcxproj	\
	rabidRabbit.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
