bin_PROGRAMS = bzwtransform

all-am: $(top_builddir)/src/common/libCommon.la

$(top_builddir)/src/common/libCommon.la:
	if test ! -f $(top_builddir)/src/common/libCommon.la ; then \
		cd $(top_builddir)/src/common && $(MAKE) $(AM_MAKEFLAGS) ; \
		cd ../../tools ; \
	fi

AM_CPPFLAGS = @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@ -fexceptions

bzwtransform_SOURCES = bzwTransform.cxx
bzwtransform_LDADD = $(top_builddir)/src/common/libCommon.la

EXTRA_DIST = input.bzt

MAINTAINERCLEANFILES = Makefile.in
