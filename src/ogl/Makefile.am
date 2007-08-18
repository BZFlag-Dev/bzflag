include $(top_srcdir)/Makefile.common

if CLIENT_INCLUDED
noinst_LTLIBRARIES = libGLKit.la
endif

MAINTAINERCLEANFILES = \
	Makefile.in

libGLKit_la_LDFLAGS = -static
libGLKit_la_SOURCES =			\
	OpenGLGState.cxx		\
	OpenGLLight.cxx			\
	OpenGLMaterial.cxx		\
	OpenGLTexture.cxx		\
	OpenGLDisplayList.cxx		\
	RenderNode.cxx

EXTRA_DIST = \
	GLCollect.cxx	\
	README
