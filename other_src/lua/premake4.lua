
project   'liblua'
  targetname 'lua'
  kind  'StaticLib'
  objdir '.objs'
  files {
    'src/lapi.cpp',     'src/lapi.h',
    'src/lauxlib.cpp',  'src/lauxlib.h',
    'src/lbaselib.cpp',
    'src/lcode.cpp',    'src/lcode.h',
    'src/ldblib.cpp',
    'src/ldebug.cpp',   'src/ldebug.h',
    'src/ldo.cpp',      'src/ldo.h',
    'src/ldump.cpp',
    'src/lfunc.cpp',    'src/lfunc.h',
    'src/lgc.cpp',      'src/lgc.h',
    'src/linit.cpp',
    'src/liolib.cpp',
    'src/llex.cpp',     'src/llex.h',
    'src/llimits.h',
    'src/lmathlib.cpp',
    'src/lmem.cpp',     'src/lmem.h',
    'src/loadlib.cpp',
    'src/lobject.cpp',  'src/lobject.h',
    'src/lopcodes.cpp', 'src/lopcodes.h',
    'src/loslib.cpp',
    'src/lparser.cpp',  'src/lparser.h',
    'src/lpeg.cpp',     'src/lpeg.h',
    'src/lstate.cpp',   'src/lstate.h',
    'src/lstring.cpp',  'src/lstring.h',
    'src/lstrlib.cpp',
    'src/ltable.cpp',   'src/ltable.h',
    'src/ltablib.cpp',
    'src/ltm.cpp',      'src/ltm.h',
--    'src/lua.cpp',
    'src/lua.h',
--    'src/luac.cpp',
    'src/luaconf.h',
    'src/lualib.h',
    'src/lundump.cpp',  'src/lundump.h',
    'src/lvm.cpp',      'src/lvm.h',
    'src/lzio.cpp',     'src/lzio.h',
    'src/print.cpp',
  }

configuration "linux"
  defines { "LUA_USE_POSIX" }

configuration "macosx"
  defines { "LUA_USE_MACOSX" }
