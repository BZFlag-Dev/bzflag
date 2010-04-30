
if CLIENT_INCLUDED
LUA_LIBS = libLuaGame.la libLuaClient.la libLuaClientGL.la
else
LUA_LIBS = libLuaGame.la
endif

noinst_LTLIBRARIES = $(LUA_LIBS)

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/bzflag \
	-I$(top_srcdir)/src/clientbase \
	$(LIBCURL_CPPFLAGS) \
	@CONF_CPPFLAGS@

AM_CXXFLAGS = @CONF_CXXFLAGS@


libLuaGame_la_LDFLAGS = -static
libLuaGame_la_SOURCES =                           \
	LuaBitOps.cxx         LuaBitOps.h         \
	LuaBZDB.cxx           LuaBZDB.h           \
	LuaBzMaterial.cxx     LuaBzMaterial.h     \
	LuaDouble.cxx         LuaDouble.h         \
	LuaDynCol.cxx         LuaDynCol.h         \
	LuaExtras.cxx         LuaExtras.h         \
	LuaHTTP.cxx           LuaHTTP.h           \
	LuaObstacle.cxx       LuaObstacle.h       \
	LuaPack.cxx           LuaPack.h           \
	LuaParser.cxx         LuaParser.h         \
	LuaPhyDrv.cxx         LuaPhyDrv.h         \
	LuaScream.cxx         LuaScream.h         \
	LuaServerPing.cxx     LuaServerPing.h     \
	LuaTexMat.cxx         LuaTexMat.h         \
	LuaUtils.cxx          LuaUtils.h          \
	LuaVector.cxx         LuaVector.h         \
	LuaVFS.cxx            LuaVFS.h            \
	LuaZip.cxx            LuaZip.h


if CLIENT_INCLUDED

libLuaClient_la_LDFLAGS = -static
libLuaClient_la_SOURCES =                         \
	LuaClientOrder.h                          \
	LuaClientScripts.cxx                      \
	LuaHandleCallIns.cxx                      \
	LuaBzOrg.cxx          LuaBzOrg.h          \
	LuaCallInCheck.cxx    LuaCallInCheck.h    \
	LuaCallInDB.cxx       LuaCallInDB.h       \
	LuaCallOuts.cxx       LuaCallOuts.h       \
	LuaConsole.cxx        LuaConsole.h        \
	LuaGameConst.cxx      LuaGameConst.h      \
	LuaControl.cxx        LuaControl.h        \
	LuaHandle.cxx         LuaHandle.h         \
	LuaKeySyms.cxx        LuaKeySyms.h        \
	LuaSpatial.cxx        LuaSpatial.h        \
	LuaUser.cxx           LuaUser.h           \
	LuaWorld.cxx          LuaWorld.h


libLuaClientGL_la_LDFLAGS = -static
libLuaClientGL_la_SOURCES =                       \
	LuaGLConst.cxx        LuaGLConst.h        \
	LuaDLists.cxx         LuaDLists.h         \
	LuaFBOs.cxx           LuaFBOs.h           \
	LuaGLBuffers.cxx      LuaGLBuffers.h      \
	LuaGLPointers.cxx     LuaGLPointers.h     \
	LuaGLQueries.cxx      LuaGLQueries.h      \
	LuaOpenGL.cxx         LuaOpenGL.h         \
	LuaRBOs.cxx           LuaRBOs.h           \
	LuaShaders.cxx        LuaShaders.h        \
	LuaTextures.cxx       LuaTextures.h
endif


EXTRA_DIST = \
	README
