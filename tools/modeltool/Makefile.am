SUBDIRS = vc71

bin_PROGRAMS = modeltool

modeltool_SOURCES = \
	modeltool.cxx \
	wavefrontOBJ.cxx\
	Q3BSP.cxx

modeltool_CPPFLAGS = \
	-I$(top_srcdir)/include

MAINTAINERCLEANFILES = \
	Makefile.in

LDADD = \
	$(top_srcdir)/src/common/libCommon.a

