
--local python_cflags = os.outputof('python-config --cflags')
--local python_include = python_cflags:match('%-I(%S*)')

if (1 > 0) then
  print()
  print('>>>  SKIPPING BZROBOTS <<<')
  print()
  do return end
end


project 'bzrobots'
  kind 'WindowedApp'
  objdir '.obj'
  files { '*.h', '*.cpp', '../obstacle/ObstacleMgr.cpp' }
  includedirs { '.', '../bzflag', '../clientbase' }
--  buildoptions '`python-config --cflags`'
--  linkoptions  '`python-config --ldflags`'
  excludes { 'PythonScript.cpp' }
  linkoptions { '-export-dynamic' } -- -static' }
  links {
    'libdate',
    'libobstacle',
    'libclientbase',
    'libLuaGame',
    'libLuaClient',
    'libLuaClientGL',
    'libgame',
    'libgeometry',
    'lib3D',
    'libnet',
    'libplatform',
    'libogl',
    'libscene',
    'libmediafile',
    'libcommon',
    'liblua',
    'SDL', 'GLEW', 'GLU', 'GL', 'X11',
    'cares', 'freetype', 'ftgl',
    'curl', 'z', 'dl', 'rt',
  }

configuration 'not windows'
  linkoptions { '-export-dynamic' } -- -static' }

configuration 'debug'
  targetsuffix '-debug'


