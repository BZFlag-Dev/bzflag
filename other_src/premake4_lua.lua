
project   'liblua'
  targetname 'lua'
  kind  'StaticLib'
  language 'C++'
  objdir '.objs_lua'
  files {
    'lua/src/lapi.cpp',     'lua/src/lapi.h',
    'lua/src/lauxlib.cpp',  'lua/src/lauxlib.h',
    'lua/src/lbaselib.cpp',
    'lua/src/lcode.cpp',    'lua/src/lcode.h',
    'lua/src/ldblib.cpp',
    'lua/src/ldebug.cpp',   'lua/src/ldebug.h',
    'lua/src/ldo.cpp',      'lua/src/ldo.h',
    'lua/src/ldump.cpp',
    'lua/src/lfunc.cpp',    'lua/src/lfunc.h',
    'lua/src/lgc.cpp',      'lua/src/lgc.h',
    'lua/src/linit.cpp',
    'lua/src/liolib.cpp',
    'lua/src/llex.cpp',     'lua/src/llex.h',
    'lua/src/llimits.h',
    'lua/src/lmathlib.cpp',
    'lua/src/lmem.cpp',     'lua/src/lmem.h',
    'lua/src/loadlib.cpp',
    'lua/src/lobject.cpp',  'lua/src/lobject.h',
    'lua/src/lopcodes.cpp', 'lua/src/lopcodes.h',
    'lua/src/loslib.cpp',
    'lua/src/lparser.cpp',  'lua/src/lparser.h',
    'lua/src/lpeg.cpp',     'lua/src/lpeg.h',
    'lua/src/lstate.cpp',   'lua/src/lstate.h',
    'lua/src/lstring.cpp',  'lua/src/lstring.h',
    'lua/src/lstrlib.cpp',
    'lua/src/ltable.cpp',   'lua/src/ltable.h',
    'lua/src/ltablib.cpp',
    'lua/src/ltm.cpp',      'lua/src/ltm.h',
--    'lua/src/lua.cpp',               -- see below
    'lua/src/lua.h',
--    'lua/src/luac.cpp',              -- see below
    'lua/src/luaconf.h',
    'lua/src/lualib.h',
    'lua/src/lundump.cpp',  'lua/src/lundump.h',
    'lua/src/lvm.cpp',      'lua/src/lvm.h',
    'lua/src/lzio.cpp',     'lua/src/lzio.h',
    'lua/src/print.cpp',
  }

configuration 'linux'
  defines { 'LUA_USE_POSIX' }

configuration 'macosx'
  defines { 'LUA_USE_MACOSX' }

--------------------------------------------------------------------------------

if (CONFIG.build_luaexecs) then
  project 'bzlua'
    kind 'ConsoleApp'
    objdir '.objs_lua'
    links 'liblua'
    files 'lua/src/lua.cpp'
    configuration 'not gmake'
      targetdir(BINDIR)
  project 'bzluac'
    kind 'ConsoleApp'
    objdir '.objs_lua'
    links 'liblua'
    files 'lua/src/luac.cpp'
    configuration 'not gmake'
      targetdir(BINDIR)
end

--------------------------------------------------------------------------------
