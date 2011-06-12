
if CLIENT_INCLUDED
CLIENT_LIBS = libLuaClient.la libLuaClientGL.la
endif

noinst_LTLIBRARIES = libLuaGame.la $(CLIENT_LIBS)

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/bzflag \
	-I$(top_srcdir)/src/clientbase \
	$(LIBCURL_CPPFLAGS) \
	@CONF_CPPFLAGS@

AM_CXXFLAGS = @CONF_CXXFLAGS@


libLuaGame_la_LDFLAGS = -static
libLuaGame_la_SOURCES =				\
	LuaBitOps.cpp		LuaBitOps.h	\
	LuaBZDB.cpp		LuaBZDB.h	\
	LuaBzMaterial.cpp	LuaBzMaterial.h	\
	LuaDouble.cpp		LuaDouble.h	\
	LuaDynCol.cpp		LuaDynCol.h	\
	LuaExtras.cpp		LuaExtras.h	\
	LuaHTTP.cpp		LuaHTTP.h	\
	LuaObstacle.cpp		LuaObstacle.h	\
	LuaPack.cpp		LuaPack.h	\
	LuaPhyDrv.cpp		LuaPhyDrv.h	\
	LuaScream.cpp		LuaScream.h	\
	LuaServerPing.cpp	LuaServerPing.h	\
	LuaTexMat.cpp		LuaTexMat.h	\
	LuaUtils.cpp		LuaUtils.h	\
	LuaVector.cpp		LuaVector.h	\
	LuaVFS.cpp		LuaVFS.h	\
	LuaZip.cpp		LuaZip.h


if CLIENT_INCLUDED

libLuaClient_la_LDFLAGS = -static
libLuaClient_la_SOURCES =			\
	LuaClientOrder.h			\
	LuaClientScripts.cpp			\
	LuaHandleCallIns.cpp			\
	LuaBzOrg.cpp		LuaBzOrg.h	\
	LuaCallInCheck.cpp	LuaCallInCheck.h \
	LuaCallInDB.cpp		LuaCallInDB.h	\
	LuaCallOuts.cpp		LuaCallOuts.h	\
	LuaConsole.cpp		LuaConsole.h	\
	LuaGameConst.cpp	LuaGameConst.h	\
	LuaControl.cpp		LuaControl.h	\
	LuaHandle.cpp		LuaHandle.h	\
	LuaKeySyms.cpp		LuaKeySyms.h	\
	LuaSpatial.cpp		LuaSpatial.h	\
	LuaUser.cpp		LuaUser.h	\
	LuaRules.cpp		LuaRules.h	\
	LuaWorld.cpp		LuaWorld.h


libLuaClientGL_la_LDFLAGS = -static
libLuaClientGL_la_SOURCES =			\
	LuaGLConst.cpp		LuaGLConst.h	\
	LuaDLists.cpp		LuaDLists.h	\
	LuaFBOs.cpp		LuaFBOs.h	\
	LuaGLBuffers.cpp	LuaGLBuffers.h	\
	LuaGLPointers.cpp	LuaGLPointers.h	\
	LuaGLQueries.cpp	LuaGLQueries.h	\
	LuaOpenGL.cpp		LuaOpenGL.h	\
	LuaRBOs.cpp		LuaRBOs.h	\
	LuaSceneNode.cpp	LuaSceneNode.h	\
	LuaShaders.cpp		LuaShaders.h	\
	LuaTextures.cpp		LuaTextures.h
endif


EXTRA_DIST = \
	README
