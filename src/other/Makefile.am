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

MAINTAINERCLEANFILES = \
	Makefile.in
