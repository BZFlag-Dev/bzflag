
bzlib_project 'lib3D'

  files {
    'Model.cpp',
    'FontManager.cpp',
    'TextureManager.cpp',
  }
  includepackage 'freetype'
  includepackage 'ftgl'

--  includedirs { os.outputof('pkg-config freetype2 --cflags'):match('%-I(.*)$')  }

