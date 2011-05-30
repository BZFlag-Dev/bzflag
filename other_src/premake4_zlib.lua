
project   'libz'
  targetname 'z'
  hidetarget('true')
  kind  'StaticLib'
  language 'C'
  objdir '.objs_zlib'
  includedirs { 'zlib' }
  files {
    'zlib/adler32.c',
    'zlib/compress.c',
    'zlib/crc32.c',    'zlib/crc32.h',
    'zlib/deflate.c',  'zlib/deflate.h',
    'zlib/gzclose.c',
    'zlib/gzguts.h',
    'zlib/gzlib.c',
    'zlib/gzread.c',
    'zlib/gzwrite.c',
    'zlib/infback.c',
    'zlib/inffast.c',  'zlib/inffast.h',
    'zlib/inffixed.h',
    'zlib/inflate.c',  'zlib/inflate.h',
    'zlib/inftrees.c', 'zlib/inftrees.h',
    'zlib/trees.c',    'zlib/trees.h',
    'zlib/uncompr.c',
    'zlib/zconf.h',
    'zlib/zlib.h',
    'zlib/zutil.c',    'zlib/zutil.h',
  }
  configuration { 'vs*' }
    defines { 'WIN32', '_LIB', '_CRT_SECURE_NO_WARNINGS' }
  configuration { 'vs*', 'debug*' }
    defines { '_CRT_NONSTDC_NO_DEPRECATE' }
