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

if CLIENT_INCLUDED
BZFLAG_DIR = bzflag
else
BZFLAG_DIR =
endif

if BZADMIN_INCLUDED
BZADMIN_DIR = bzadmin
else
BZADMIN_DIR =
endif

if BZROBOTS_INCLUDED
BZROBOTS_DIR = bzrobots
else
BZROBOTS_DIR =
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
	${BZFLAG_DIR}	\
	${BZADMIN_DIR} \
	${BZROBOTS_DIR}

MAINTAINERCLEANFILES = \
	Makefile.in

