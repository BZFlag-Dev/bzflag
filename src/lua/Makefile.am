noinst_LTLIBRARIES = libBzLua.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libBzLua_la_LDFLAGS = -static

libBzLua_la_SOURCES =                       \
	LuaInclude.h                              \
	LuaClientOrder.h                          \
	LuaClientScripts.cxx                      \
	LuaHandleCallIns.cxx                      \
	LuaHashString.h                           \
	LuaBitOps.cxx         LuaBitOps.h         \
	LuaBZDB.cxx           LuaBZDB.h           \
	LuaBzMaterial.cxx     LuaBzMaterial.h     \
	LuaBzOrg.cxx          LuaBzOrg.h          \
	LuaCallInCheck.cxx    LuaCallInCheck.h    \
	LuaCallInDB.cxx       LuaCallInDB.h       \
	LuaCallOuts.cxx       LuaCallOuts.h       \
	LuaConstGame.cxx      LuaConstGame.h      \
	LuaConstGL.cxx        LuaConstGL.h        \
	LuaDouble.cxx         LuaDouble.h         \
	LuaDLists.cxx         LuaDLists.h         \
	LuaDynCol.cxx         LuaDynCol.h         \
	LuaExtras.cxx         LuaExtras.h         \
	LuaFBOs.cxx           LuaFBOs.h           \
	LuaFontTexture.cxx    LuaFontTexture.h    \
	LuaGLBuffers.cxx      LuaGLBuffers.h      \
	LuaGLPointers.cxx     LuaGLPointers.h     \
	LuaGLQueries.cxx      LuaGLQueries.h      \
	LuaHandle.cxx         LuaHandle.h         \
	LuaKeySyms.cxx        LuaKeySyms.h        \
	LuaObstacle.cxx       LuaObstacle.h       \
	LuaOpenGL.cxx         LuaOpenGL.h         \
	LuaPack.cxx           LuaPack.h           \
	LuaParser.cxx         LuaParser.h         \
	LuaPhyDrv.cxx         LuaPhyDrv.h         \
	LuaRBOs.cxx           LuaRBOs.h           \
	LuaScream.cxx         LuaScream.h         \
	LuaShaders.cxx        LuaShaders.h        \
	LuaSpatial.cxx        LuaSpatial.h        \
	LuaTexMat.cxx         LuaTexMat.h         \
	LuaTextures.cxx       LuaTextures.h       \
	LuaUser.cxx           LuaUser.h           \
	LuaUtils.cxx          LuaUtils.h          \
	LuaURL.cxx            LuaURL.h            \
	LuaVector.cxx         LuaVector.h         \
	LuaVFS.cxx            LuaVFS.h            \
	LuaWorld.cxx          LuaWorld.h          \
	LuaZip.cxx            LuaZip.h

#	LuaLink.cxx           LuaLink.h           \#
#	LuaServerList.cxx     LuaServerList.h     \#
#	LuaArrays.cxx         LuaArrays.h         \#

EXTRA_DIST = \
	README
