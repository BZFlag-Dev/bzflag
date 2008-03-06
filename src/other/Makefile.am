if BUILD_ZLIB
ZLIB_DIR = zlib
else
ZLIB_DIR =
endif

if BUILD_ARES
ARES_DIR = ares
else
ARES_DIR =
endif

if BUILD_REGEX
REGEX_DIR = regex
else
REGEX_DIR =
endif

if BUILD_CURL
CURL_DIR = curl
else
CURL_DIR =
endif

if BUILD_FTGL
FTGL_DIR = ftgl
else
FTGL_DIR =
endif

if BUILD_FREETYPE
FREETYPE_DIR = freetype
else
FREETYPE_DIR =
endif

SUBDIRS = \
	${ARES_DIR} \
	${CURL_DIR} \
	${FREETYPE_DIR} \
	${FTGL_DIR} \
	${REGEX_DIR} \
	${ZLIB_DIR}

DIST_SUBDIRS = \
	ares \
	curl \
	freetype \
	ftgl \
	regex \
	zlib

# include the files for subconfigured packages as needed left out of the dist
EXTRA_DIST = \
	ares/FILES \
	ares/Makefile.dj \
	ares/Makefile.m32 \
	ares/Makefile.netware \
	ares/Makefile.vc6 \
	ares/ares_getopt.c \
	ares/ares_getopt.h \
	ares/buildconf \
	ares/maketgz \
	curl/CHANGES.0 \
	curl/CHANGES.1999 \
	curl/CHANGES.2000 \
	curl/CHANGES.2001 \
	curl/CHANGES.2002 \
	curl/CHANGES.2003 \
	curl/CHANGES.2004 \
	curl/CHANGES.2005 \
	curl/CVS-INFO \
	curl/Makefile.dist \
	curl/TODO-RELEASE \
	curl/docs/examples/adddocsref.pl \
	curl/docs/libcurl/curl_multi_socket_all.3 \
	curl/docs/libcurl/curl_strnequal.3 \
	curl/docs/libcurl/getinfo-times \
	curl/hiper \
	curl/lib/Makefile.inc \
	curl/lib/libcurl.def \
	curl/lib/vc8proj.foot \
	curl/lib/vc8proj.head \
	curl/perl \
	curl/src/hugehelp.c.cvs \
	curl/src/stamp-h2

MAINTAINERCLEANFILES = \
	Makefile.in
