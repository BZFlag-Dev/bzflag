lib_LTLIBRARIES = flagStay.la

flagStay_la_SOURCES = flagStay.cpp
flagStay_la_CPPFLAGS = -I$(top_srcdir)/include
flagStay_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.flagStay.txt \
	flagStay.bzw \
	flagStay.sln \
	flagStay.vcxproj	\
	flagStay.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
