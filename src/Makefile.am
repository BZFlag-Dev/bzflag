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

# needed by clients and server, always build
core_dirs = \
	date \
	obstacle \
	game \
	net \
	common

# needed only by clients, generally requires OpenGL
gui_dirs = \
	mediafile \
	ogl \
	3D \
	geometry \
	scene \
	platform

if CLIENT_INCLUDED
CLIENT_DIRS = $(gui_dirs)
else

if BZROBOTS_INCLUDED
CLIENT_DIRS = $(gui_dirs)
else
CLIENT_DIRS =
endif # BZROBOTS_INCLUDED

endif # CLIENT_INCLUDED

# the order matters
SUBDIRS = \
	other \
	$(core_dirs) \
	$(CLIENT_DIRS) \
	${BZFS_DIR} \
	${BZADMIN_DIR} \
	${BZROBOTS_DIR} \
	${BZFLAG_DIR}

MAINTAINERCLEANFILES = \
	Makefile.in
