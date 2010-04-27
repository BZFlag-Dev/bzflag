noinst_LTLIBRARIES = libGLKit.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/bzflag \
	-I$(top_srcdir)/src/clientbase \
	@CONF_CPPFLAGS@

AM_CXXFLAGS = @CONF_CXXFLAGS@

libGLKit_la_LDFLAGS = -static
libGLKit_la_SOURCES =		\
	OpenGLContext.cxx	\
	OpenGLGState.cxx	\
	OpenGLLight.cxx		\
	OpenGLMaterial.cxx	\
	OpenGLPassState.cxx	\
	OpenGLTexture.cxx	\
	OpenGLUtils.cxx		\
	RenderNode.cxx

EXTRA_DIST = 		\
	README
