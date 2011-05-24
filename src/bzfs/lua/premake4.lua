
project 'libLuaServer'
  targetname 'LuaServer'
  kind 'StaticLib'
  files {
    'CallIns.cpp',   'CallIns.h',
    'CallOuts.cpp',  'CallOuts.h',
    'Constants.cpp', 'Constants.h',
    'LuaServer.cpp', 'LuaServer.h',
    'MapObject.cpp', 'MapObject.h',
    'RawLink.cpp',   'RawLink.h',
    'SlashCmd.cpp',  'SlashCmd.h',
  }
  includedirs { '../..', '../../lua' }
  objdir('.obj')


