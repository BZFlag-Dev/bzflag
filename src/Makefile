# bzflag
# Copyright (c) 1993 - 2002 Tim Riker
#
# This package is free software;  you can redistribute it and/or
# modify it under the terms of the license found in the file
# named LICENSE that should have accompanied this file.
#
# THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.

#
# makefile to build sources
#

DEPTH = ..
COMMONPREF = src
include $(DEPTH)/Make-common

SUBDIRS =				\
	zlib				\
	bzfrelay			\
	game				\
	platform			\
	net				\
	common				\
	bzfls				\
	bzfs				\
	mediafile			\
	geometry			\
	obstacle			\
	scene				\
	ogl				\
	ui				\
	view				\
	mplatform			\
	bzflag				\
					\
	bzgview				\
	$(NULL)

targets:
	$(SUBDIRS_MAKERULE)

$(COMMONTARGETS):
	$(SUBDIRS_MAKERULE)

