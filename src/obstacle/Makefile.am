
noinst_LTLIBRARIES = libObstacle.la

MAINTAINERCLEANFILES =	\
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libObstacle_la_LDFLAGS = -static

gl_files = \
	BaseSceneNodeGenerator.cxx	\
	BoxSceneNodeGenerator.cxx	\
	EmptySceneNodeGenerator.cxx	\
	ObstacleSceneNodeGenerator.cxx	\
	PyramidSceneNodeGenerator.cxx	\
	TeleporterSceneNodeGenerator.cxx\
	WallSceneNodeGenerator.cxx	\
	MeshSceneNodeGenerator.cxx	\
	ArcObstacle.cxx			\
	ConeObstacle.cxx		\
	SphereObstacle.cxx		\
	BaseBuilding.cxx		\
	BoxBuilding.cxx			\
	MeshFace.cxx			\
	MeshObstacle.cxx		\
	MeshUtils.h			\
	Triangulate.h			\
	Triangulate.cxx			\
	MeshDrawInfo.cxx		\
	PyramidBuilding.cxx		\
	Teleporter.cxx			\
	TetraBuilding.cxx		\
	WallObstacle.cxx		\
	Obstacle.cxx			\
	ObstacleModifier.cxx		\
	ObstacleList.cxx		\
	ObstacleMgr.cxx

nogl_files = \
	ArcObstacle.cxx			\
	ConeObstacle.cxx		\
	SphereObstacle.cxx		\
	BaseBuilding.cxx		\
	BoxBuilding.cxx			\
	MeshFace.cxx			\
	MeshObstacle.cxx		\
	MeshUtils.h			\
	Triangulate.h			\
	Triangulate.cxx			\
	MeshDrawInfo.cxx		\
	PyramidBuilding.cxx		\
	Teleporter.cxx			\
	TetraBuilding.cxx		\
	WallObstacle.cxx		\
	Obstacle.cxx			\
	ObstacleModifier.cxx		\
	ObstacleList.cxx		\
	ObstacleMgr.cxx

if CLIENT_INCLUDED
  libObstacle_la_SOURCES = $(gl_files)
else
  if BZROBOTS_INCLUDED
    libObstacle_la_SOURCES = $(gl_files)
  else
    libObstacle_la_SOURCES = $(nogl_files)
  endif
endif

EXTRA_DIST =	\
	README
