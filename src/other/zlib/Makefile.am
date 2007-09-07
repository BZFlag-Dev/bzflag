# $Id$

noinst_LTLIBRARIES = libz.la

libz_la_LDFLAGS = -static
libz_la_SOURCES = \
	adler32.c \
	compress.c \
	crc32.c \
	deflate.c \
	gzio.c \
	infback.c \
	inffast.c \
	inflate.c \
	inftrees.c \
	trees.c \
	uncompr.c \
	zutil.c

noinst_HEADERS = \
	crc32.h \
	deflate.h \
	inffast.h \
	inffixed.h \
	inflate.h \
	inftrees.h \
	trees.h \
	zconf.h \
	zlib.h \
	zutil.h

EXTRA_DIST = \
	ChangeLog \
	FAQ \
	INDEX \
	algorithm.txt \
	zlib.3

MAINTAINERCLEANFILES = Makefile.in
