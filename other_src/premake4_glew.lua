
project   'libglew'
  targetname 'glew'
  hidetarget('true')
  language 'C'
  kind  'StaticLib'
  objdir '.objs_glew'
  includedirs { 'glew/include', '../src/include' }
  files {
    'glew/include/GL/glew.h',
    'glew/src/glew.c',
  }



