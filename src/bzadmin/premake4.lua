
project 'bzadmin'
  kind 'ConsoleApp'
  objdir '.obj'
  files { '*.h', '*.cpp', '../clientbase/ServerLink.cpp' }
  includedirs { '.', '../bzflag', '../clientbase' }
  defines { 'BUILDING_BZADMIN' }
  links {
    'libdate',
    'libgame',
    'libnet',
    'libcommon',
    'curses', 'cares', 'curl', 'z', 'dl', 'rt',
  }

configuration 'debug'
  targetsuffix '-debug'
