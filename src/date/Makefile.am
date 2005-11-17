noinst_LTLIBRARIES = libDate.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

EXTRA_DIST = README

libDate_la_LDFLAGS = -static
libDate_la_SOURCES = \
	buildDate.cxx
