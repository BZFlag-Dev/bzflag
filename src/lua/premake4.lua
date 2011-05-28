
bzlib_project 'libLuaGame'

  targetname 'LuaGame'

  includedirs { '..', '../bzflag', '../clientbase' }

  files {
    'LuaBitOps.cpp',     'LuaBitOps.h',
    'LuaBZDB.cpp',       'LuaBZDB.h',
    'LuaBzMaterial.cpp', 'LuaBzMaterial.h',
    'LuaDouble.cpp',     'LuaDouble.h',
    'LuaDynCol.cpp',     'LuaDynCol.h',
    'LuaExtras.cpp',     'LuaExtras.h',
    'LuaHTTP.cpp',       'LuaHTTP.h',
    'LuaObstacle.cpp',   'LuaObstacle.h',
    'LuaPack.cpp',       'LuaPack.h',
    'LuaParser.cpp',     'LuaParser.h',
    'LuaPhyDrv.cpp',     'LuaPhyDrv.h',
    'LuaScream.cpp',     'LuaScream.h',
    'LuaServerPing.cpp', 'LuaServerPing.h',
    'LuaTexMat.cpp',     'LuaTexMat.h',
    'LuaUtils.cpp',      'LuaUtils.h',
    'LuaVector.cpp',     'LuaVector.h',
    'LuaVFS.cpp',        'LuaVFS.h',
    'LuaZip.cpp',        'LuaZip.h',
  }


bzlib_project 'libLuaClient'

  includedirs { '..', '../bzflag', '../clientbase' }

  files {
    'LuaClientOrder.h',
    'LuaClientScripts.cpp',
    'LuaHandleCallIns.cpp',
    'LuaBzOrg.cpp',       'LuaBzOrg.h',
    'LuaCallInCheck.cpp', 'LuaCallInCheck.h',
    'LuaCallInDB.cpp',    'LuaCallInDB.h',
    'LuaCallOuts.cpp',    'LuaCallOuts.h',
    'LuaConsole.cpp',     'LuaConsole.h',
    'LuaControl.cpp',     'LuaControl.h',
    'LuaGameConst.cpp',   'LuaGameConst.h',
    'LuaHandle.cpp',      'LuaHandle.h',
    'LuaKeySyms.cpp',     'LuaKeySyms.h',
    'LuaRules.cpp',       'LuaRules.h',
    'LuaSpatial.cpp',     'LuaSpatial.h',
    'LuaUser.cpp',        'LuaUser.h',
    'LuaWorld.cpp',       'LuaWorld.h',
  }


bzlib_project 'libLuaClientGL'

  includedirs { '..', '../bzflag', '../clientbase' }

  files {
    'LuaGLConst.cpp',    'LuaGLConst.h',
    'LuaDLists.cpp',     'LuaDLists.h',
    'LuaFBOs.cpp',       'LuaFBOs.h',
    'LuaGLBuffers.cpp',  'LuaGLBuffers.h',
    'LuaGLPointers.cpp', 'LuaGLPointers.h',
    'LuaGLQueries.cpp',  'LuaGLQueries.h',
    'LuaOpenGL.cpp',     'LuaOpenGL.h',
    'LuaRBOs.cpp',       'LuaRBOs.h',
    'LuaSceneNode.cpp',  'LuaSceneNode.h',
    'LuaShaders.cpp',    'LuaShaders.h',
    'LuaTextures.cpp',   'LuaTextures.h',
  }


