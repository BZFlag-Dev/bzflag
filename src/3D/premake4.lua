
project   'lib3D'
  targetname '3D'
  kind  'StaticLib'
  objdir '.objs'
  files {
    'Model.cpp',
    'FontManager.cpp',
    'TextureManager.cpp',
  }

if (not os.is('windows')) then
  includedirs { os.outputof('freetype-config --cflags'):match('%-I(.*)$') }
end
--  includedirs { os.outputof('pkg-config freetype2 --cflags'):match('%-I(.*)$')  }

