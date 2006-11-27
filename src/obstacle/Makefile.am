
noinst_LTLIBRARIES = libObstacle.la

MAINTAINERCLEANFILES =	\
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libObstacle_la_LDFLAGS = -static
libObstacle_la_LIBADD = \
	$(top_builddir)/src/common/libCommon.la

# FIXME: this library shouldn't, but does indirectly include/use stuff from lib3D and it should not
#	$(top_builddir)/src/3D/lib3D.la

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
