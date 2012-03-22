noinst_LTLIBRARIES = libDate.la

AM_CPPFLAGS = $(CONF_CPPFLAGS) -I$(top_srcdir)/include
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES = \
	Makefile.in

EXTRA_DIST = README

libDate_la_LDFLAGS = -static
libDate_la_SOURCES = \
	buildDate.cxx
