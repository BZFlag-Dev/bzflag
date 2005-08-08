if BUILDZLIB
ZLIB_DIR = zlib
else
ZLIB_DIR =
endif

if BUILDCARES
CARES_DIR = c-ares
else
CARES_DIR =
endif

if BUILD_REGEX
REGEX_DIR = regex
else
REGEX_DIR =
endif

SUBDIRS = \
	${CARES_DIR}	\
	3D \
	${REGEX_DIR} \
	common \
	net \
	game \
	geometry \
	obstacle \
	${ZLIB_DIR} \
	ogl \
	scene \
	platform \
	mediafile \
	date \
	bzfs \
	bzflag \
	bzadmin

MAINTAINERCLEANFILES = \
	Makefile.in

