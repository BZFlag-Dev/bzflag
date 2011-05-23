
include 'lua'

project 'bzfs'
--  language 'C++'
  kind 'ConsoleApp'
--  includedirs '../include'
  files { '*.h', '*.cpp' }
  objdir '.obj'
  links {
    'libdate',
    'libgame',
    'libobstacle',
    'libnet',
    'libLuaServer',
    'libLuaGame',
    'liblua',
    'libcommon',
    'curl', 'z', 'dl', 'cares',
  }

configuration 'not windows'
  linkoptions { '-export-dynamic' } -- -static' }

configuration 'debug'
  targetsuffix '-debug'

