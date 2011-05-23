
project   'liblua'
  targetname 'lua'
  kind  'StaticLib'
  objdir '.obj'
  files { 'src/*.h', 'src/*.cpp' }

if (os.is('linux')) then
  defines { 'LUA_USE_POSIX' }
end


