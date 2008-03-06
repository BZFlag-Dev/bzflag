SUBDIRS = \
	vc71 \
	vc8

bin_PROGRAMS = modeltool

all-am: $(top_builddir)/src/common/libCommon.la

$(top_builddir)/src/common/libCommon.la:
	if test ! -f $(top_builddir)/src/common/libCommon.la ; then \
		cd $(top_builddir)/src/common && $(MAKE) $(AM_MAKEFLAGS) ; \
		cd ../../tools ; \
	fi

AM_CPPFLAGS = @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

modeltool_SOURCES =	\
	model.h		\
	modeltool.cxx	\
	Q3BSP.cxx	\
	Q3BSP.h		\
	wavefrontOBJ.cxx\
	wavefrontOBJ.h

modeltool_CPPFLAGS = -I$(top_srcdir)/include
modeltool_LDADD = $(top_builddir)/src/common/libCommon.la

EXTRA_DIST = example.diconf

MAINTAINERCLEANFILES = Makefile.in
