
noinst_LTLIBRARIES = libObstacle.la

MAINTAINERCLEANFILES =	\
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include

libObstacle_la_LDFLAGS = -static

if CLIENT_INCLUDED
libObstacle_la_SOURCES =		\
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
else
libObstacle_la_SOURCES =		\
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
endif

EXTRA_DIST =	\
	README
