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

if SERVER_INCLUDED
LIBIRC_DIR = libirc
else
LIBIRC_DIR =
endif

SUBDIRS = \
	${ARES_DIR} \
	${CURL_DIR} \
	${REGEX_DIR} \
	${ZLIB_DIR} \
	${LIBIRC_DIR}

DIST_SUBDIRS = \
	ares \
	curl \
	regex \
	zlib \
	libirc

MAINTAINERCLEANFILES = \
	Makefile.in
