noinst_LTLIBRARIES = libGame.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/include \
	$(LIBCURL_CPPFLAGS) \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libGame_la_LDFLAGS = -static
libGame_la_SOURCES =			\
	global.cpp			\
	BZDBCache.cpp			\
	BZDBNames.cpp			\
	BzMaterial.cpp			\
	CollisionManager.cpp		\
	CommandCompleter.cpp		\
	DirectoryNames.cpp		\
	DynamicColor.cpp		\
	Flag.cpp			\
	Frustum.cpp			\
	GameTime.cpp			\
	Intersect.cpp			\
	LagInfo.cpp			\
	LinkDef.cpp			\
	LinkManager.cpp			\
	LinkPhysics.cpp			\
	MapInfo.cpp			\
	MsgStrings.cpp			\
	MeshTransform.cpp		\
	MotionUtils.cpp			\
	NetHandler.cpp			\
	NetMessage.cpp			\
	PhysicsDriver.cpp		\
	PlayerInfo.cpp			\
	PlayerState.cpp			\
	ServerItem.cpp			\
	ServerList.cpp			\
	ServerListCache.cpp		\
	ServerPing.cpp			\
	ShotManager.cpp			\
	ShotUpdate.cpp			\
	StartupInfo.cpp			\
	Team.cpp			\
	TextureMatrix.cpp		\
	VotingArbiter.cpp		\
	WorldText.cpp

EXTRA_DIST = \
	README
