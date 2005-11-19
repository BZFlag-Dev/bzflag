noinst_LTLIBRARIES = libregex.la

AM_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
AM_CFLAGS = @CONF_CFLAGS@

libregex_la_LDFLAGS = -static
libregex_la_SOURCES  =	\
	cclass.h	\
	cname.h		\
	regcomp.c	\
	regerror.c	\
	regex2.h	\
	regexec.c	\
	regfree.c	\
	utils.h

EXTRA_DIST =		\
	engine.c

MAINTAINERCLEANFILES = \
	Makefile.in
