
noinst_LTLIBRARIES = libObstacle.la

MAINTAINERCLEANFILES =	\
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/src/include @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libObstacle_la_LDFLAGS = -static

gl_files = \
	BaseSceneNodeGenerator.cpp	\
	BoxSceneNodeGenerator.cpp	\
	EmptySceneNodeGenerator.cpp	\
	ObstacleSceneNodeGenerator.cpp	\
	PyramidSceneNodeGenerator.cpp	\
	WallSceneNodeGenerator.cpp	\
	MeshSceneNodeGenerator.cpp	\
	ArcObstacle.cpp			\
	ConeObstacle.cpp		\
	SphereObstacle.cpp		\
	BaseBuilding.cpp		\
	BoxBuilding.cpp			\
	MeshFace.cpp			\
	MeshObstacle.cpp		\
	MeshUtils.h			\
	Triangulate.h			\
	Triangulate.cpp			\
	MeshDrawInfo.cpp		\
	PyramidBuilding.cpp		\
	Teleporter.cpp			\
	WallObstacle.cpp		\
	Obstacle.cpp			\
	ObstacleModifier.cpp		\
	ObstacleList.cpp		\
	ObstacleMgr.cpp

nogl_files = \
	ArcObstacle.cpp			\
	ConeObstacle.cpp		\
	SphereObstacle.cpp		\
	BaseBuilding.cpp		\
	BoxBuilding.cpp			\
	MeshFace.cpp			\
	MeshObstacle.cpp		\
	MeshUtils.h			\
	Triangulate.h			\
	Triangulate.cpp			\
	MeshDrawInfo.cpp		\
	PyramidBuilding.cpp		\
	Teleporter.cpp			\
	WallObstacle.cpp		\
	Obstacle.cpp			\
	ObstacleModifier.cpp		\
	ObstacleList.cpp		\
	ObstacleMgr.cpp

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
