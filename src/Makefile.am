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

# the order matters greatly here
SUBDIRS = \
	other \
	date \
	common \
	net \
	game \
	obstacle \
	$(CLIENT_DIRS) \
	${BZFS_DIR} \
	${BZADMIN_DIR} \
	${BZROBOTS_DIR} \
	${BZFLAG_DIR}

MAINTAINERCLEANFILES = \
	Makefile.in
