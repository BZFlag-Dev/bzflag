
if CLIENT_INCLUDED
noinst_LIBRARIES = libObstacle.a libObstacleGL.a
else
noinst_LIBRARIES = libObstacle.a
endif

MAINTAINERCLEANFILES = \
	Makefile.in

libObstacle_a_SOURCES =			\
	BaseBuilding.cxx		\
	BoxBuilding.cxx			\
	Intersect.cxx			\
	Obstacle.cxx			\
	PyramidBuilding.cxx		\
	Teleporter.cxx			\
	WallObstacle.cxx

if CLIENT_INCLUDED
libObstacleGL_a_SOURCES =		\
	BaseSceneNodeGenerator.cxx	\
	BoxSceneNodeGenerator.cxx	\
	EmptySceneNodeGenerator.cxx	\
	Intersect.cxx			\
	ObstacleSceneNodeGenerator.cxx  \
	PyramidSceneNodeGenerator.cxx	\
	TeleporterSceneNodeGenerator.cxx\
	WallSceneNodeGenerator.cxx
endif

	

EXTRA_DIST = \
	README
	