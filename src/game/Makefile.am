noinst_LTLIBRARIES = libGame.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include @CURL_CFLAGS@ @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libGame_la_LDFLAGS = -static
libGame_la_SOURCES =			\
	BzMaterial.cxx			\
	CacheManager.cxx		\
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
	StartupInfo.cxx			\
	TextureMatrix.cxx		\
	VotingArbiter.cxx


EXTRA_DIST = \
	README
