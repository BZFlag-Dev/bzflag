
project 'bzflag'
  kind 'WindowedApp'
  objdir '.obj'
  files { '*.h', '*.cpp' }
  includedirs { '.', '../clientbase' }
  links {
    'libdate',
    'libobstacle',
    'libLuaGame',
    'libLuaClient',
    'libLuaClientGL',
    'libclientbase',
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

configuration 'debug'
  targetsuffix '-debug'
  