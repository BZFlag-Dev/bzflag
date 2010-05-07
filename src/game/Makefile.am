noinst_LTLIBRARIES = libGame.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(LIBCURL_CPPFLAGS) \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libGame_la_LDFLAGS = -static
libGame_la_SOURCES =			\
	global.cxx			\
	BZDBCache.cxx			\
	BZDBNames.cxx			\
	BzMaterial.cxx			\
	CollisionManager.cxx		\
	CommandCompleter.cxx		\
	DirectoryNames.cxx		\
	DynamicColor.cxx		\
	Flag.cxx			\
	Frustum.cxx			\
	GameTime.cxx			\
	Intersect.cxx			\
	LagInfo.cxx			\
	LinkDef.cxx			\
	LinkManager.cxx			\
	LinkPhysics.cxx			\
	MapInfo.cxx			\
	MsgStrings.cxx			\
	MeshTransform.cxx		\
	MotionUtils.cxx			\
	NetHandler.cxx			\
	NetMessage.cxx			\
	PhysicsDriver.cxx		\
	PlayerInfo.cxx			\
	PlayerState.cxx			\
	ServerItem.cxx			\
	ServerList.cxx			\
	ServerListCache.cxx		\
	ServerPing.cxx			\
	ShotManager.cxx			\
	ShotUpdate.cxx			\
	StartupInfo.cxx			\
	Team.cxx			\
	TextureMatrix.cxx		\
	VotingArbiter.cxx		\
	WorldText.cxx

EXTRA_DIST = \
	README
