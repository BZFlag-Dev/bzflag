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
	BZDBCache.cxx			\
	BufferedNetworkMessage.cxx	\
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
	LinkManager.cxx			\
	MapInfo.cxx			\
	MsgStrings.cxx			\
	MeshTransform.cxx		\
	MotionUtils.cxx			\
	NetHandler.cxx			\
	PhysicsDriver.cxx		\
	PlayerInfo.cxx			\
	PlayerState.cxx			\
	Ray.cxx				\
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
	WorldContentInfo.cxx


EXTRA_DIST = \
	README
