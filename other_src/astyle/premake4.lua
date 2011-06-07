
project 'astyle'
  kind       'ConsoleApp'
  language   'C++'
  targetname 'astyle-bzflag'
  targetdir  '../../misc'
  objdir     '.objs'
  files {
    'src/astyle.h',
    'src/astyle_main.cpp',  'src/astyle_main.h',
    'src/ASBeautifier.cpp',
    'src/ASEnhancer.cpp',
    'src/ASFormatter.cpp',
    'src/ASLocalizer.cpp',  'src/ASLocalizer.h',
    'src/ASResource.cpp',
  }
