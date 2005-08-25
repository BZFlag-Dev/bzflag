SUBDIRS = models

MAINTAINERCLEANFILES = \
	Makefile.in

if CLIENT_INCLUDED
noinst_LTLIBRARIES = libGeometry.la
endif

AM_CPPFLAGS = -I$(top_srcdir)/include

libGeometry_la_LDFLAGS = -static
libGeometry_la_SOURCES  =		\
	ViewFrustum.cxx			\
	SceneNode.cxx			\
	WallSceneNode.cxx		\
	QuadWallSceneNode.cxx		\
	TriWallSceneNode.cxx		\
	MeshFragSceneNode.cxx		\
	MeshPolySceneNode.cxx		\
	PolyWallSceneNode.cxx		\
	EighthDimSceneNode.cxx		\
	EighthDimShellNode.cxx		\
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
	AnimatedTreads.cxx		\
	TankSceneNode.cxx		\
	TankGeometryMgr.cxx		\
	BillboardSceneNode.cxx

libGeometry_la_LIBADD =	\
	models/tank/high_barrel.lo	\
	models/tank/high_body.lo	\
	models/tank/high_ltread.lo	\
	models/tank/high_rtread.lo	\
	models/tank/high_turret.lo	\
	models/tank/low_barrel.lo	\
	models/tank/low_body.lo		\
	models/tank/low_ltread.lo	\
	models/tank/low_rtread.lo	\
	models/tank/low_turret.lo	\
	models/tank/medium_barrel.lo	\
	models/tank/medium_body.lo	\
	models/tank/medium_ltread.lo	\
	models/tank/medium_rtread.lo	\
	models/tank/medium_turret.lo

EXTRA_DIST = \
	README
