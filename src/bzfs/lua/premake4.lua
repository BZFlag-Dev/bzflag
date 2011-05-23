
project 'libLuaServer'
  targetname 'LuaServer'
  kind 'StaticLib'
  files { '*.h', '*.cpp' }
  includedirs { '../..', '../../lua' }
  objdir('.obj')


