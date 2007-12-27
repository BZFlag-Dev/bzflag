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
	BzMaterial.cxx			\
	CollisionManager.cxx		\
	CommandsStandard.cxx		\
	DirectoryNames.cxx		\
	DynamicColor.cxx		\
	Frustum.cxx			\
	Intersect.cxx			\
	LagInfo.cxx			\
	LinkManager.cxx			\
	MsgStrings.cxx			\
	MeshTransform.cxx		\
	NetHandler.cxx			\
	PhysicsDriver.cxx		\
	PlayerInfo.cxx			\
	Ray.cxx				\
	ServerItem.cxx			\
	ServerList.cxx			\
	ServerListCache.cxx		\
	ServerPing.cxx			\
	StartupInfo.cxx			\
	TextureMatrix.cxx		\
	VotingArbiter.cxx		\
	WorldContentInfo.cxx


EXTRA_DIST = \
	README
