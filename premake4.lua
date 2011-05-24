--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  Useful for MSVC users?
--
--    http://industriousone.com/topic/running-premake-visual-studio
--
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

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

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

include 'premake4_config'

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

solution 'bz'

if (os.is('windows')) then
  platforms { 'x32', 'x64' }
end

if (_OPTIONS['enable-debug']) then
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

newopt('disable-bzfs',     'disable the bzfs build')
newopt('disable-bzflag',   'disable the bzflag build')
newopt('disable-bzadmin',  'disable the bzadmin build')
newopt('disable-bzrobots', 'disable the bzrobots build')
newopt('disable-plugins',  'disable the bzfs plugins')
newopt('enable-debug',     'enable debugging')


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

include 'other_src'
include 'src'
include 'man'

if (not _OPTIONS['disable-plugins']) then
  include 'plugins'
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
