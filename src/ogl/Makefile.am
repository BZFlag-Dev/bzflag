if CLIENT_INCLUDED
noinst_LTLIBRARIES = libGLKit.la
endif

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CXXFLAGS = @CRYSTAL_CFLAGS@

libGLKit_la_LDFLAGS = -static
libGLKit_la_SOURCES =			\
	OpenGLGState.cxx		\
	OpenGLLight.cxx			\
	OpenGLMaterial.cxx		\
	OpenGLDisplayList.cxx		\
	RenderNode.cxx

EXTRA_DIST = \
	GLCollect.cxx	\
	README
