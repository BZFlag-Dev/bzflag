
bzlib_project 'lib3D'

  files {
    'Model.cpp',
    'FontManager.cpp',
    'TextureManager.cpp',
  }

if (not _ACTION:match('^vs')) then
  includedirs { os.outputof('freetype-config --cflags'):match('%-I(.*)$') }
end
--  includedirs { os.outputof('pkg-config freetype2 --cflags'):match('%-I(.*)$')  }

