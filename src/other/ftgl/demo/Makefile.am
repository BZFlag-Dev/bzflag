
if HAVE_GLUT
noinst_PROGRAMS = simple c-demo FTGLDemo FTGLMFontDemo
endif

simple_SOURCES = \
    simple.cpp \
    $(NULL)
simple_CPPFLAGS = -I$(top_srcdir)/src
simple_CXXFLAGS = $(FT2_CFLAGS) $(GL_CFLAGS)
simple_LDFLAGS = $(FT2_LIBS) $(GLUT_LIBS)
simple_LDADD = ../src/libftgl.la

c_demo_SOURCES = \
    c-demo.c \
    $(NULL)
c_demo_CPPFLAGS = -I$(top_srcdir)/src
c_demo_CFLAGS = $(FT2_CFLAGS) $(GL_CFLAGS)
c_demo_LDFLAGS = $(FT2_LIBS) $(GLUT_LIBS)
c_demo_LDADD = ../src/libftgl.la

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
