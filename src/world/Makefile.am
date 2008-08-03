#
# Build libBZW
#

noinst_LTLIBRARIES = libBZW.la

AM_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

noinst_HEADERS = 	\
	Grammar.h	\
	Parser.h

libBZW_la_LDFLAGS = -static
libBZW_la_SOURCES =	\
	World.cxx		\
	WorldObject.cxx		\
	Parser.cxx

EXTRA_DIST = README

MAINTAINERCLEANFILES = \
	Makefile.in

