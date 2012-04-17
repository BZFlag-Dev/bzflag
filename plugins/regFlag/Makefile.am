lib_LTLIBRARIES = regFlag.la

regFlag_la_SOURCES = regFlag.cpp
regFlag_la_CPPFLAGS = -I$(top_srcdir)/include
regFlag_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.txt \
	regFlag.def \
	regFlag.sln \
	regFlag.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
