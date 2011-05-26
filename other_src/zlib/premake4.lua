
project   'libz'
  targetname 'z'
  language 'C'
  kind  'StaticLib'
  objdir '.objs'
  files {
    'adler32.c',
    'compress.c',
    'crc32.c',    'crc32.h',
    'deflate.c',  'deflate.h',
    'gzclose.c',
    'gzlib.c',
    'gzread.c',
    'gzwrite.c',
    'infback.c',
    'inffast.c',  'inffast.h',
    'inflate.c',  'inflate.h',
    'inftrees.c', 'inftrees.h',
    'trees.c',    'trees.h',
    'uncompr.c',
    'zutil.c',    'zutil.h',
    'gzguts.h',
    'inffixed.h',
    'zconf.h',
    'zlib.h',
  }

