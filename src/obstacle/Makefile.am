
noinst_LIBRARIES = libObstacle.a

MAINTAINERCLEANFILES = \
	Makefile.in

if CLIENT_INCLUDED
libObstacle_a_SOURCES =			\
	BaseSceneNodeGenerator.cxx	\
	BoxSceneNodeGenerator.cxx	\
	EmptySceneNodeGenerator.cxx	\
	ObstacleSceneNodeGenerator.cxx  \
	PyramidSceneNodeGenerator.cxx	\
	TeleporterSceneNodeGenerator.cxx\
	WallSceneNodeGenerator.cxx	\
	BaseBuilding.cxx		\
	BoxBuilding.cxx			\
	Intersect.cxx			\
	Obstacle.cxx			\
	PyramidBuilding.cxx		\
	Teleporter.cxx			\
	WallObstacle.cxx
else
libObstacle_a_SOURCES =			\
	BaseBuilding.cxx		\
	BoxBuilding.cxx			\
	Intersect.cxx			\
	Obstacle.cxx			\
	PyramidBuilding.cxx		\
	Teleporter.cxx			\
	WallObstacle.cxx
endif

EXTRA_DIST = \
	README
