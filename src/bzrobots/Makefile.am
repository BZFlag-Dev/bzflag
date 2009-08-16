# NOTE: This Makefile.am is hideous!  We link with a bunch of files found
# in ../bzflag.  These files will be compiled to .o files in the current
# directory.  Most of these files are only linked to because the linker
# needs the symbols somewhere.  Several of the files should have all of the
# graphics stuff taken out and moved to ../game.  There's a ton of work
# ahead for this to happen.  In the meantime, this seems to be the least
# ugly of all of the solutions.

bin_PROGRAMS = bzrobots
lib_LTLIBRARIES = TestRobot.la $(libbzrobot)

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/clientbase \
	-I$(top_srcdir)/src/bzflag \
	$(SDL_CFLAGS) \
	$(LIBCURL_CPPFLAGS) \
	@X_CFLAGS@ \
	@CONF_CPPFLAGS@

PYTHON_LOADER = \
	PythonScript.cxx	\
	PythonScript.h

if BZROBOTS_PYTHON_ENABLED
AM_CXXFLAGS = @CONF_CXXFLAGS@ \
	$(PYTHON_CPPFLAGS) -DWITH_PYTHONLOADER=1
bzrobots_LDFLAGS = -export-dynamic -static $(PYTHON_LDFLAGS)
pythonloaderfiles = $(PYTHON_LOADER)
else
AM_CXXFLAGS = @CONF_CXXFLAGS@
bzrobots_LDFLAGS = -export-dynamic -static
pythonloaderfiles =
endif

if CLIENT_INCLUDED
clientfiles = \
	../obstacle/ObstacleMgr.cxx	
else
clientfiles = \
	OpenGLUtils.cxx	\
	../obstacle/ObstacleMgr.cxx	
endif

apisources = \
	AdvancedRobot.cxx		\
	Bullet.cxx			\
	Events.cxx			\
	Robot.cxx			\
	RobotStatus.cxx

apiheaders = \
	AdvancedRobot.h			\
	Bullet.h			\
	Events.h			\
	Robot.h				\
	RobotStatus.h
	

bzrobots_SOURCES =			\
	$(pythonloaderfiles)		\
	$(clientfiles)			\
	$(apisources)			\
	$(apiheaders)			\
	bzrobots.cxx			\
	botplaying.cxx			\
	botplaying.h			\
	RobotCallbacks.h		\
	RobotControl.cxx		\
	RobotControl.h			\
	BZRobotPlayer.cxx		\
	BZRobotPlayer.h			\
	RobotScript.cxx			\
	RobotScript.h			\
	LuaHeader.h			\
	LuaScript.cxx			\
	LuaScript.h			\
	ScriptLoaderFactory.cxx		\
	ScriptLoaderFactory.h		\
	SharedObjectScript.cxx		\
	SharedObjectScript.h		


if WIN32
RESOURCE = bzflag.res
else
RESOURCE =
endif

if MACOS
NACLIB = ../platform/MacOSX/libMac.la
endif

bzrobots_LDADD =	\
	../obstacle/libObstacle.la	\
	../clientbase/libClientBase.la	\
	../game/libGame.la		\
	../geometry/libGeometry.la	\
	../3D/lib3D.la			\
	../net/libNet.la		\
	$(NACLIB)			\
	../platform/libPlatform.la	\
	../ogl/libGLKit.la		\
	../scene/libSceneDB.la		\
	../mediafile/libMediaFile.la	\
	../common/libCommon.la		\
	$(LIBLUA)			\
	$(LIBFTGL)			\
	$(LIBFREETYPE)			\
	$(LIBCURL)			\
	$(LIBCARES)			\
	$(LIBREGEX)			\
	$(LIBZ)				\
	$(RESOURCE)			\
	$(SDL_LIBS)			\
	$(GLIBS)			\
	$(GLEW_LIBS)			\
	../date/libDate.la

TestRobot_la_LDFLAGS = -module -avoid-version
TestRobot_la_SOURCES = \
	BZAdvancedRobot.h \
	../../bots/testbot/TestRobot.cxx \
	../../bots/testbot/TestRobot.h

python_wrap_files = \
	bzrobot_python_wrap.cxx \
	bzrobot_python_wrap.h \
	bzrobot_python_runtime.h

if BZROBOTS_PYTHON_ENABLED
libbzrobot = bzrobot_pyext.la

bzrobot_python_wrap.h: bzrobot_python_wrap.cxx

bzrobot_python_wrap.cxx: bzrobot.i $(apiheaders)
	$(SWIG) -Wall -c++ -python -o $@ -interface bzrobot_pyext $< # Consider using -O for 'optimized mode'

bzrobot_python_runtime.h:
	$(SWIG) -python -external-runtime $@

BUILT_SOURCES = $(python_wrap_files)

bzrobot_pyext_la_CXXFLAGS = -fexceptions $(SWIG_PYTHON_CPPFLAGS)
bzrobot_pyext_la_LDFLAGS = -module -avoid-version -export-dynamic
bzrobot_pyext_la_SOURCES = \
	$(BUILT_SOURCES) \
	$(apisources) \
	$(apiheaders)

else
libbzrobot =
endif

bzflag.res: $(top_srcdir)/win32/bzflag.rc $(top_srcdir)/win32/bzflag.ico
	$(WINDRES) --include-dir=$(top_srcdir)/win32/ -i $(top_srcdir)/win32/bzflag.rc -o bzflag.res \
	-O coff

EXTRA_DIST = \
	$(python_wrap_files) \
	$(PYTHON_LOADER) \
	OpenGLUtils.cxx	\
	PrintError.cxx \
	PrintError.h \
	bzrobot.i

MAINTAINERCLEANFILES = \
	Makefile.in

CLEANFILES = \
	$(python_wrap_files) \
	bzrobot.py

