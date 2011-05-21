SUBDIRS = models

noinst_LTLIBRARIES = libGeometry.la

AM_CPPFLAGS = -I$(top_srcdir)/src/include @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libGeometry_la_LDFLAGS = -static
libGeometry_la_SOURCES  =		\
	AnimatedTreads.cpp		\
	BillboardSceneNode.cpp		\
	BoltSceneNode.cpp		\
	EighthDBaseSceneNode.cpp	\
	EighthDBoxSceneNode.cpp		\
	EighthDimSceneNode.cpp		\
	EighthDimShellNode.cpp		\
	EighthDPyrSceneNode.cpp		\
	FlagSceneNode.cpp		\
	FlagWarpSceneNode.cpp		\
	LaserSceneNode.cpp		\
	MeshDrawMgr.cpp			\
	MeshFragSceneNode.cpp		\
	MeshPolySceneNode.cpp		\
	MeshRenderNode.cpp		\
	MeshRenderNode.h		\
	MeshSceneNode.cpp		\
	OccluderSceneNode.cpp		\
	PolyWallSceneNode.cpp		\
	QuadWallSceneNode.cpp		\
	SceneNode.cpp			\
	ShellSceneNode.cpp		\
	SphereSceneNode.cpp		\
	TankGeometryMgr.cpp		\
	TankSceneNode.cpp		\
	TextSceneNode.cpp		\
	TriWallSceneNode.cpp		\
	ViewFrustum.cpp			\
	WallSceneNode.cpp

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

MAINTAINERCLEANFILES = \
	Makefile.in
