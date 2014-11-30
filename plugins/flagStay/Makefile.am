lib_LTLIBRARIES = flagStay.la

flagStay_la_SOURCES = flagStay.cpp
flagStay_la_CPPFLAGS = -I$(top_srcdir)/include
flagStay_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.txt \
	flagStay.bzw \
	flagStay.def \
	flagStay.sln \
	flagStay.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
