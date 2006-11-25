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

if SERVER_INCLUDED
BZFS_DIR = bzfs
else
BZFS_DIR =
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

# the order matters greatly here
SUBDIRS = \
	${CARES_DIR}	\
	${REGEX_DIR} \
	${ZLIB_DIR} \
	date \
	common \
	mediafile \
	ogl \
	3D \
	net \
	game \
	geometry \
	obstacle \
	scene \
	platform \
	${BZFS_DIR} \
	${BZADMIN_DIR} \
	${BZROBOTS_DIR} \
	${BZFLAG_DIR}

MAINTAINERCLEANFILES = \
	Makefile.in

