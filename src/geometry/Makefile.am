SUBDIRS = models

MAINTAINERCLEANFILES = \
	Makefile.in

if CLIENT_INCLUDED
noinst_LIBRARIES = libGeometry.a
endif

AM_CPPFLAGS = -I$(top_srcdir)/include

libGeometry_a_SOURCES  =		\
	ViewFrustum.cxx			\
	SceneNode.cxx			\
	WallSceneNode.cxx		\
	QuadWallSceneNode.cxx		\
	TriWallSceneNode.cxx		\
	MeshPolySceneNode.cxx		\
	PolyWallSceneNode.cxx		\
	EighthDimSceneNode.cxx		\
	EighthDBaseSceneNode.cxx	\
	EighthDBoxSceneNode.cxx		\
	EighthDPyrSceneNode.cxx		\
	EighthDTetraSceneNode.cxx	\
	ShellSceneNode.cxx		\
	TracerSceneNode.cxx		\
	PTSceneNode.cxx			\
	BoltSceneNode.cxx		\
	LaserSceneNode.cxx		\
	SphereSceneNode.cxx		\
	FlagSceneNode.cxx		\
	FlagWarpSceneNode.cxx		\
	TankSceneNode.cxx		\
	BillboardSceneNode.cxx

libGeometry_a_LIBADD =	\
	models/tank/high_barrel.o	\
	models/tank/high_body.o		\
	models/tank/high_ltread.o	\
	models/tank/high_rtread.o	\
	models/tank/high_turret.o	\
	models/tank/low_barrel.o	\
	models/tank/low_body.o		\
	models/tank/low_ltread.o	\
	models/tank/low_rtread.o	\
	models/tank/low_turret.o	\
	models/tank/medium_barrel.o	\
	models/tank/medium_body.o	\
	models/tank/medium_ltread.o	\
	models/tank/medium_rtread.o	\
	models/tank/medium_turret.o

EXTRA_DIST = \
	README