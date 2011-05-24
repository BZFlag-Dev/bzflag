
project 'bzadmin'
  kind 'ConsoleApp'
  objdir '.objs'
  defines { 'BUILDING_BZADMIN' }
  includedirs { '.', '../bzflag', '../clientbase' }
  links {
    'libGame',
    'libNet',
    'libCommon',
    'libDate',
    'curses', 'cares', 'curl', 'z', 'dl', 'rt',
  }
  files {
    '../clientbase/ServerLink.cpp',
    'BZAdminClient.cpp', 'BZAdminClient.h',
    'bzadmin.cpp',
    'BZAdminUI.cpp',     'BZAdminUI.h',
    'colors.h',
    'CursesMenu.cpp',    'CursesMenu.h',
    'CursesUI.cpp',      'CursesUI.h',
    'curses_wrapper.h',
    'OptionParser.cpp',  'OptionParser.h',
    'PlayerInfo.h',
    'StdBothUI.cpp',     'StdBothUI.h',
    'StdInUI.cpp',       'StdInUI.h',
    'StdOutUI.cpp',      'StdOutUI.h',
    'UIMap.cpp',         'UIMap.h',
  }

configuration 'not gmake'
  targetdir(BINDIR)
