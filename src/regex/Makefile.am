if BUILD_REGEX
noinst_LIBRARIES = libregex.a
endif

AM_CPPFLAGS = -I$(top_srcdir)/include

libregex_a_SOURCES  =	\
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
