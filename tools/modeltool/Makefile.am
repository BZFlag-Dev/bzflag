SUBDIRS = MSVC

bin_PROGRAMS = modeltool

modeltool_SOURCES =	\
	model.h		\
	modeltool.cxx	\
	Q3BSP.cxx	\
	Q3BSP.h		\
	SimpleTextUtils.cxx	\
	SimpleTextUtils.h	\
	wavefrontOBJ.cxx	\
	wavefrontOBJ.h

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES = \
	Makefile.in
