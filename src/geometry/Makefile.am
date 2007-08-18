include $(top_srcdir)/Makefile.common

SUBDIRS = models

MAINTAINERCLEANFILES = \
	Makefile.in

if CLIENT_INCLUDED
noinst_LTLIBRARIES = libGeometry.la
endif

libGeometry_la_LDFLAGS = -static
libGeometry_la_SOURCES  =		\
	AnimatedTreads.cxx		\
	BillboardSceneNode.cxx		\
	BoltSceneNode.cxx		\
	EighthDBaseSceneNode.cxx	\
	EighthDBoxSceneNode.cxx		\
	EighthDimSceneNode.cxx		\
	EighthDimShellNode.cxx		\
	EighthDPyrSceneNode.cxx		\
	FlagSceneNode.cxx		\
	FlagWarpSceneNode.cxx		\
	LaserSceneNode.cxx		\
	MeshDrawMgr.cxx			\
	MeshFragSceneNode.cxx		\
	MeshPolySceneNode.cxx		\
	MeshRenderNode.cxx		\
	MeshRenderNode.h		\
	MeshSceneNode.cxx		\
	OccluderSceneNode.cxx		\
	PolyWallSceneNode.cxx		\
	PTSceneNode.cxx			\
	QuadWallSceneNode.cxx		\
	SceneNode.cxx			\
	ShellSceneNode.cxx		\
	SphereSceneNode.cxx		\
	TankGeometryMgr.cxx		\
	TankSceneNode.cxx		\
	TracerSceneNode.cxx		\
	TriWallSceneNode.cxx		\
	ViewFrustum.cxx			\
	WallSceneNode.cxx

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
