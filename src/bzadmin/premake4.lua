
bzexec_project 'bzadmin'

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

  defines { 'BUILDING_BZADMIN' }

  includedirs { '.', '../bzflag', '../clientbase' }

  links {
    'libGame',
    'libNet',
    'libCommon',
    'libDate',
  }
  linkpackage('curses')
  linkpackage('curl')
  linkpackage('ares')
  linkpackage('regex')
  linkpackage('zlib')
  linkpackage('dl')
  linkpackage('rt')

