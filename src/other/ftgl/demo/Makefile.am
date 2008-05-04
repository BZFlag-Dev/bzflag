
if HAVE_GLUT
noinst_PROGRAMS = FTGLDemo FTGLMFontDemo
endif

FTGLDemo_SOURCES = \
	FTGLDemo.cpp \
	tb.c \
	tb.h \
	trackball.c \
	trackball.h \
	$(NULL)
FTGLDemo_CPPFLAGS = -I$(top_srcdir)/src
FTGLDemo_CXXFLAGS = $(FT2_CFLAGS) $(GL_CFLAGS)
FTGLDemo_LDFLAGS = $(FT2_LIBS) $(GLUT_LIBS)
FTGLDemo_LDADD = ../src/libftgl.la

FTGLMFontDemo_SOURCES = \
	FTGLMFontDemo.cpp \
	tb.c \
	tb.h \
	trackball.c \
	trackball.h \
	$(NULL)
FTGLMFontDemo_CPPFLAGS = -I$(top_srcdir)/src
FTGLMFontDemo_CXXFLAGS = $(FT2_CFLAGS) $(GL_CFLAGS)
FTGLMFontDemo_LDFLAGS = $(FT2_LIBS) $(GLUT_LIBS)
FTGLMFontDemo_LDADD = ../src/libftgl.la

NULL =
