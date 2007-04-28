if BUILD_ZLIB
ZLIB_DIR = zlib
else
ZLIB_DIR =
endif

if BUILD_CARES
CARES_DIR = ares
else
CARES_DIR =
endif

if BUILD_REGEX
REGEX_DIR = regex
else
REGEX_DIR =
endif

SUBDIRS = \
	${CARES_DIR} \
	${REGEX_DIR} \
	${ZLIB_DIR}

MAINTAINERCLEANFILES = \
	Makefile.in
