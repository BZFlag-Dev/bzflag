AUTOMAKE_OPTIONS = subdir-objects

noinst_SUBDIRS = models

MAINTAINERCLEANFILES = \
	Makefile.in

if CLIENT_INCLUDED
noinst_LIBRARIES = libGeometry.a
endif

libGeometry_a_SOURCES  =		\
	ViewFrustum.cxx			\
	Ray.cxx				\
	SceneNode.cxx			\
	WallSceneNode.cxx		\
	QuadWallSceneNode.cxx		\
	TriWallSceneNode.cxx		\
	PolyWallSceneNode.cxx		\
	EighthDimSceneNode.cxx		\
	EighthDBaseSceneNode.cxx	\
	EighthDBoxSceneNode.cxx		\
	EighthDPyrSceneNode.cxx		\
	ShellSceneNode.cxx		\
	TracerSceneNode.cxx		\
	PTSceneNode.cxx			\
	BoltSceneNode.cxx		\
	LaserSceneNode.cxx		\
	SphereSceneNode.cxx		\
	FlagSceneNode.cxx		\
	FlagWarpSceneNode.cxx		\
	TankSceneNode.cxx		\
	BillboardSceneNode.cxx		\
	models/lowtank/barrel.cxx	\
	models/lowtank/body.cxx		\
	models/lowtank/ltread.cxx	\
	models/lowtank/rtread.cxx	\
	models/lowtank/turret.cxx	\
	models/medtank/barrel.cxx	\
	models/medtank/body.cxx		\
	models/medtank/ltread.cxx	\
	models/medtank/rtread.cxx	\
	models/medtank/turret.cxx	\
	models/hitank/barrel.cxx	\
	models/hitank/body.cxx		\
	models/hitank/ltread.cxx	\
	models/hitank/rtread.cxx	\
	models/hitank/turret.cxx
