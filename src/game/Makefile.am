noinst_LTLIBRARIES = libGame.la

AM_CPPFLAGS = $(LIBCURL_CPPFLAGS) $(CONF_CPPFLAGS) -I$(top_srcdir)/include
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES = \
	Makefile.in

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
	TextureMatrix.cxx


EXTRA_DIST = \
	README
