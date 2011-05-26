--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  Useful for MSVC users?
--
--    http://industriousone.com/topic/running-premake-visual-studio
--
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

assert(_PREMAKE_BZFLAG and (_PREMAKE_BZFLAG >= 1),
  '\n  A customized premake is required to configure the bzflag build.'..
  '\n  The customized premake sources can be found in:  ./other_src/premake/.')

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

_PREMAKE_EXEC = 'other_src/premake/bin/release/premake4'

ACTION  = _ACTION
OPTIONS = _OPTIONS

function defaultaction(osName, actionName)
  if (_ACTION == nil) then
    if os.is(osName) then
      _ACTION = actionName
    end
    print('Default action for ' .. osName .. ' is ' .. actionName)
  end
end

defaultaction('bsd',     'gmake')
defaultaction('linux',   'gmake')
defaultaction('solaris', 'gmake')
defaultaction('windows', 'vs2010')
defaultaction('macosx',  'xcode3')


if (_OPTIONS['help']) then
  _ACTION = 'help'
end

if (_ACTION == 'help') then
  _OPTIONS['help'] = ''
  print('HELPING')
end



--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

TOPDIR = os.getcwd()
print(TOPDIR)

BINDIR = os.getcwd() .. '/bin' -- used for binaries (except for gmake)

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

CONFIG = {

  package_name  = 'BZFlag',

  major_version = 2,
  minor_version = 99,
  revision      = 60,

  protocol      = '0118',

  build_type    = 'DEVEL', -- or 'STABLE' or 'MAINT'

  copyright     = 'Copyright (c) 1993-2010 Tim Riker',

  package_url   = 'http://BZFlag.org',
  bugreport_url = 'http://BZFlag.org',

  config_date   = os.date(),

  build_os = nil,

  curses = { buildoptions = nil, linkoptions = nil  },
  opengl = { buildoptions = nil, linkoptions = nil  },
  sdl    = { buildoptions = nil, linkoptions = nil  },

  ares     = { need_build = nil, buildoptions = nil, linkoptions = nil  },
  curl     = { need_build = nil, buildoptions = nil, linkoptions = nil  },
  glew     = { need_build = nil, buildoptions = nil, linkoptions = nil  },
  freetype = { need_build = nil, buildoptions = nil, linkoptions = nil  },
  ftgl     = { need_build = nil, buildoptions = nil, linkoptions = nil  },
  glew     = { need_build = nil, buildoptions = nil, linkoptions = nil  },
  regex    = { need_build = nil, buildoptions = nil, linkoptions = nil  },
  zlib     = { need_build = nil, buildoptions = nil, linkoptions = nil  },

  -- other CONFIG[] values can be found in premake4_config/config_h.lua
}


CONFIG.package_version =
  CONFIG.major_version .. '.' ..
  CONFIG.minor_version .. '.' ..
  CONFIG.revision

CONFIG.package_string =
  CONFIG.package_name .. ' ' .. CONFIG.package_version


print(CONFIG.package_string)

if (_OPTIONS['build-luaexecs']) then
  CONFIG.build_luaexecs = true
end


--FIXME: one of these is causing a Make Bomb
--CONFIG.ares.need_build     = true -- FIXME
--CONFIG.curl.need_build     = true -- FIXME
--CONFIG.freetype.need_build = true -- FIXME
--CONFIG.type.need_build     = true -- FIXME
--CONFIG.zlib.need_build     = true -- FIXME
--CONFIG.glew.need_build     = true -- FIXME
--CONFIG.regex.need_build    = true -- FIXME


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

solution 'bz'

if (os.is('windows')) then
  platforms { 'x32', 'x64' }
end

if (_OPTIONS['debugging']) then
  configurations { 'debug', 'release' }
else
  configurations { 'release', 'debug' }
end

if (_ACTION and (_ACTION ~= 'gmake')) then
  location('build_' .. _ACTION)
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

configuration 'release*'
  defines { 'NDEBUG' }
  flags   { 'ExtraWarnings', 'OptimizeSpeed' }

configuration 'debug*'
  defines { 'DEBUG', '_DEBUG', 'DEBUG_RENDERING' }
  flags   { 'ExtraWarnings', 'FatalWarnings', 'Symbols' }
  targetsuffix '-debug'

configuration 'linux'
  buildoptions '-Wextra -Wundef -Wshadow -Wno-long-long -ansi -pedantic'

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local function newopt(trigger, description)
  newoption { trigger = trigger, description = description }
end

newopt('debugging',        'default to the "debug" configuration')
newopt('disable-bzfs',     'disable the bzfs build')
newopt('disable-bzflag',   'disable the bzflag build')
newopt('disable-bzadmin',  'disable the bzadmin build')
newopt('disable-bzrobots', 'disable the bzrobots build')
newopt('disable-plugins',  'disable the bzfs plugins')
newopt('build-all',        'build all the included libraries')
newopt('build-ares',       'build the ares library')
newopt('build-curl',       'build the curl library')
newopt('build-freetype',   'build the freetype library')
newopt('build-ftgl',       'build the ftgl library')
newopt('build-glew',       'build the glew library')
newopt('build-regex',      'build the regex library')
newopt('build-zlib',       'build the zlib library')
newopt('build-luaexecs',   'build the bzlua and bzluac executables')

if (_OPTIONS['build-luaexecs']) then CONFIG.build_luaexecs      = true end
if (_OPTIONS['build-ares'])     then CONFIG.ares.need_build     = true end
if (_OPTIONS['build-curl'])     then CONFIG.curl.need_build     = true end
if (_OPTIONS['build-freetype']) then CONFIG.freetype.need_build = true end
if (_OPTIONS['build-ftgl'])     then CONFIG.ftgl.need_build     = true end
if (_OPTIONS['build-glew'])     then CONFIG.glew.need_build     = true end
if (_OPTIONS['build-regex'])    then CONFIG.regex.need_build    = true end
if (_OPTIONS['build-zlib'])     then CONFIG.zlib.need_build     = true end
if (_OPTIONS['build-all']) then
  CONFIG.build_luaexecs      = true
  CONFIG.ares.need_build     = true
  CONFIG.curl.need_build     = true
  CONFIG.freetype.need_build = true
  CONFIG.ftgl.need_build     = true
  CONFIG.glew.need_build     = true
  CONFIG.regex.need_build    = true
  CONFIG.zlib.need_build     = true
end


--[[
if (_OPTIONS['disable-bzfs']) then
  _OPTIONS['disable-plugins'] = {
    value = 'true',
    trigger = 'trigger',
    description = 'desc',
  }
end
--]]


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


if (not _OPTIONS['help']) then
  include 'premake4_config'

  include 'other_src'
  include 'src'

  if (not _OPTIONS['disable-plugins']) then
    include 'plugins'
  end

  include 'man'
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

--[[
newaction {
  trigger = 'install',
  description = 'install stuff',
  execute = function()
    os.execute('echo installing some stuff')
  end
}
--]]

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

if (-1 > 0) then
  print('os.get()', os.get())
  print('premake.gcc.cxx', premake.gcc.cxx)
  premake.gcc.cxx = 'ccache g++'
  print('premake.gcc.cxx', premake.gcc.cxx)
end

if (-1 > 0) then
  print()
  print(('-'):rep(80))
  for k, v in pairs(_G) do print('_G', k, tostring(v)) end
  for k, v in pairs(_MAKE) do print('_MAKE', k, tostring(v)) end
  for k, v in pairs(_OPTIONS) do print('_OPTIONS', k, tostring(v)) end
  for k, v in pairs(_VS) do print('_VS', k, tostring(v)) end
  for k, v in pairs(_ARGS) do print('_ARGS', k, tostring(v)) end
  print()
  print(('-'):rep(80))
  print()
end

if (-1 > 0) then
  print('22premake.gcc.cxx', premake.gcc.cxx)
  premake.gcc.cxx = 'ccache g++'
  print('22premake.gcc.cxx', premake.gcc.cxx)
end

print()
print(('-'):rep(80))
print(('-'):rep(80))
print()

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
