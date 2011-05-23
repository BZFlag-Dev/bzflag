
project   'lib3D'
  targetname '3D'
  kind  'StaticLib'
  objdir '.obj'
  files { '*.h', '*.cpp' }

configuration 'not windows'
  includedirs { os.outputof('freetype-config --cflags'):match('%-I(.*)$') }
--  includedirs { os.outputof('pkg-config freetype2 --cflags'):match('%-I(.*)$')  }



