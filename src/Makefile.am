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

# "other" must be near the top, if not first, for maintainer-clean to work
SUBDIRS = \
	other \
	3D \
	common \
	net \
	game \
	geometry \
	obstacle \
	ogl \
	scene \
	platform \
	mediafile \
	date \
	${BZFS_DIR} \
	${BZFLAG_DIR} \
	${BZADMIN_DIR}

MAINTAINERCLEANFILES = \
	Makefile.in
