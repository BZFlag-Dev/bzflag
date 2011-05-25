
project   'liblua'
  targetname 'lua'
  kind  'StaticLib'
  objdir '.objs'
  files {
    'lapi.cpp',     'lapi.h',
    'lauxlib.cpp',  'lauxlib.h',
    'lbaselib.cpp',
    'lcode.cpp',    'lcode.h',
    'ldblib.cpp',
    'ldebug.cpp',   'ldebug.h',
    'ldo.cpp',      'ldo.h',
    'ldump.cpp',
    'lfunc.cpp',    'lfunc.h',
    'lgc.cpp',      'lgc.h',
    'linit.cpp',
    'liolib.cpp',
    'llex.cpp',     'llex.h',
    'llimits.h',
    'lmathlib.cpp',
    'lmem.cpp',     'lmem.h',
    'loadlib.cpp',
    'lobject.cpp',  'lobject.h',
    'lopcodes.cpp', 'lopcodes.h',
    'loslib.cpp',
    'lparser.cpp',  'lparser.h',
    'lpeg.cpp',     'lpeg.h',
    'lstate.cpp',   'lstate.h',
    'lstring.cpp',  'lstring.h',
    'lstrlib.cpp',
    'ltable.cpp',   'ltable.h',
    'ltablib.cpp',
    'ltm.cpp',      'ltm.h',
--    'lua.cpp',               -- see below
    'lua.h',
--    'luac.cpp',              -- see below
    'luaconf.h',
    'lualib.h',
    'lundump.cpp',  'lundump.h',
    'lvm.cpp',      'lvm.h',
    'lzio.cpp',     'lzio.h',
    'print.cpp',
  }

configuration "linux"
  defines { "LUA_USE_POSIX" }

configuration "macosx"
  defines { "LUA_USE_MACOSX" }

--------------------------------------------------------------------------------

if (CONFIG.build_luaexecs) then
  project 'bzlua'
    kind 'ConsoleApp'
    objdir '.objs'
    links 'liblua'
    files 'lua.cpp'
    configuration 'not gmake'
      targetdir(BINDIR)
  project 'bzluac'
    kind 'ConsoleApp'
    objdir '.objs'
    links 'liblua'
    files 'luac.cpp'
    configuration 'not gmake'
      targetdir(BINDIR)
end

--------------------------------------------------------------------------------
