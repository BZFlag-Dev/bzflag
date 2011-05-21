noinst_LTLIBRARIES = libGLKit.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/bzflag \
	-I$(top_srcdir)/src/clientbase \
	@CONF_CPPFLAGS@

AM_CXXFLAGS = @CONF_CXXFLAGS@

libGLKit_la_LDFLAGS = -static
libGLKit_la_SOURCES =		\
	OpenGLContext.cpp	\
	OpenGLGState.cpp	\
	OpenGLLight.cpp		\
	OpenGLMaterial.cpp	\
	OpenGLPassState.cpp	\
	OpenGLTexture.cpp	\
	OpenGLUtils.cpp		\
	RenderNode.cpp

EXTRA_DIST = 		\
	README
