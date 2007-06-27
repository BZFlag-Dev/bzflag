
noinst_LTLIBRARIES = libObstacle.la

MAINTAINERCLEANFILES =	\
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libObstacle_la_LDFLAGS = -static
libObstacle_la_SOURCES =		\
	ArcObstacle.cxx			\
	BaseBuilding.cxx		\
	BaseSceneNodeGenerator.cxx	\
	BoxBuilding.cxx			\
	BoxSceneNodeGenerator.cxx	\
	ConeObstacle.cxx		\
	EmptySceneNodeGenerator.cxx	\
	MeshDrawInfo.cxx		\
	MeshFace.cxx			\
	MeshObstacle.cxx		\
	MeshSceneNodeGenerator.cxx	\
	MeshUtils.h			\
	Obstacle.cxx			\
	ObstacleList.cxx		\
	ObstacleMgr.cxx			\
	ObstacleModifier.cxx		\
	ObstacleSceneNodeGenerator.cxx	\
	PyramidBuilding.cxx		\
	PyramidSceneNodeGenerator.cxx	\
	SphereObstacle.cxx		\
	Teleporter.cxx			\
	TeleporterSceneNodeGenerator.cxx\
	TetraBuilding.cxx		\
	Triangulate.cxx			\
	Triangulate.h			\
	WallObstacle.cxx		\
	WallSceneNodeGenerator.cxx

EXTRA_DIST =	\
	README
