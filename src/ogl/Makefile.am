if CLIENT_INCLUDED
noinst_LTLIBRARIES = libGLKit.la
endif

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = $(CONF_CPPFLAGS) -I$(top_srcdir)/include

libGLKit_la_LDFLAGS = -static
libGLKit_la_SOURCES =			\
	OpenGLGState.cxx		\
	OpenGLLight.cxx			\
	OpenGLMaterial.cxx		\
	OpenGLTexture.cxx		\
	OpenGLUtils.cxx		\
	RenderNode.cxx

EXTRA_DIST = \
	GLCollect.cxx	\
	README
