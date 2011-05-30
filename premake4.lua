--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  Useful for MSVC users?
--
--    http://industriousone.com/topic/running-premake-visual-studio
--
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  Make sure that we're using a valid premake executable
--

local MIN_BZFLAG_PREMAKE = 1

if (not _PREMAKE_BZFLAG) then
  print('\n'
  .. 'A customized premake is required to configure the bzflag build.\n\n'
  .. 'The customized premake sources can be found in: ./other_src/premake/.\n'
  )
  os.exit(1)
end

if (_PREMAKE_BZFLAG < MIN_BZFLAG_PREMAKE) then
  print()
  printf('  Your BZFlag version of premake is too old  (%g vs. %g)',
         _PREMAKE_BZFLAG, MIN_BZFLAG_PREMAKE)
  print()
  os.exit(1)
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  Used to reconfigure the build system  (defaults to 'premake4')
--

_PREMAKE_EXEC = 'other_src/premake/bin/release/premake4'

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  Store a couple of useful directories in global variables
--

TOPDIR = os.getcwd()

print(TOPDIR)

BINDIR = os.getcwd() .. '/bin' -- used for binaries (except for gmake)

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  FIXME --  hook the 'files()' call, might be used for installations
--
ALL_FILES = {}
EXTRA_FILES = {}
do
  local orig_files = files
  function files(d)
    local t = (type(d) == 'table') and d or { d }
    for _, f in ipairs(t) do
      ALL_FILES[os.getcwd() .. '/' .. f] = true
    end
    orig_files(d)
  end
  function extrafiles(d)
    local t = (type(d) == 'table') and d or { d }
    for _, f in ipairs(t) do
      EXTRA_FILES[os.getcwd() .. '/' .. f] = true
    end
  end
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  Setup the default premake action
--

function defaultaction(osName, actionName)
  if (_ACTION == nil) then
    if os.is(osName) then
      _ACTION = actionName
    end
--FIXME    print('Default action for ' .. osName .. ' is ' .. actionName)
  end
end

defaultaction('bsd',     'gmake')
defaultaction('linux',   'gmake')
defaultaction('solaris', 'gmake')
defaultaction('windows', 'vs2008')
defaultaction('macosx',  'xcode3')


if (_OPTIONS['help']) then
  _ACTION = 'help'
end

if (_ACTION == 'help') then
  _OPTIONS['help'] = ''
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  Load config.lua
--

CONFIG = {}

local configChunk = assert(loadfile('config.lua'))
setfenv(configChunk, CONFIG) -- global variable accesses use the CONFIG table
--setmetatable(CONFIG, { __index = _G }) -- but fallback to the main environment
configChunk()

if ((CONFIG.BUILD_TYPE ~= 'DEVEL')  and
    (CONFIG.BUILD_TYPE ~= 'STABLE') and
    (CONFIG.BUILD_TYPE ~= 'MAINT')) then
  printf('Bad build type from config.lua:  "%s"', CONFIG.BUILD_TYPE)
  os.exit(1)
end

--------------------------------------------------------------------------------

CONFIG.BUILD_OS = 'FIXME2.1'

CONFIG.CONFIG_DATE = os.date('%Y%m%d')

CONFIG.build_luaexecs = false
CONFIG.BUILD_ARES     = false
CONFIG.BUILD_CURL     = false
CONFIG.BUILD_FREETYPE = false
CONFIG.BUILD_FTGL     = false
CONFIG.BUILD_GLEW     = false
CONFIG.BUILD_REGEX    = false
CONFIG.BUILD_ZLIB     = false

--------------------------------------------------------------------------------

CONFIG.PACKAGE_VERSION =
  CONFIG.MAJOR_VERSION .. '.' .. CONFIG.MINOR_VERSION .. '.' .. CONFIG.REVISION

CONFIG.PACKAGE_STRING =
  CONFIG.PACKAGE_NAME .. ' ' .. CONFIG.PACKAGE_VERSION

if (_OPTIONS['build-luaexecs']) then
  CONFIG.build_luaexecs = true
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

print()
printf('%s  (%s)', CONFIG.PACKAGE_STRING, CONFIG.CONFIG_DATE)
print()

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  Start the premake configuration
--

solution 'bz'

if (_ACTION:match('vs*')) then
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
  defines { '_DEBUG' }
  flags   { 'ExtraWarnings', 'Symbols' }
  targetsuffix '-debug'

configuration { 'debug*', 'not vs*' }
  defines { 'DEBUG', 'DEBUG_RENDERING' } -- FIXME -- stick these in config.h
  flags   { 'FatalWarnings' }

configuration 'vs*'
  defines { 'WIN32' }

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

--------------------------------------------------------------------------------

if (_OPTIONS['build-luaexecs']) then CONFIG.build_luaexecs = true end
if (_OPTIONS['build-ares'])     then CONFIG.BUILD_ARES     = true end
if (_OPTIONS['build-curl'])     then CONFIG.BUILD_CURL     = true end
if (_OPTIONS['build-freetype']) then CONFIG.BUILD_FREETYPE = true end
if (_OPTIONS['build-ftgl'])     then CONFIG.BUILD_FTGL     = true end
if (_OPTIONS['build-glew'])     then CONFIG.BUILD_GLEW     = true end
if (_OPTIONS['build-regex'])    then CONFIG.BUILD_REGEX    = true end
if (_OPTIONS['build-zlib'])     then CONFIG.BUILD_ZLIB     = true end
if (_OPTIONS['build-all'] or _ACTION:match('^vs*')) then
  CONFIG.BUILD_ARES     = true
  CONFIG.BUILD_CURL     = true
  CONFIG.BUILD_FREETYPE = true
  CONFIG.BUILD_FTGL     = true
  CONFIG.BUILD_GLEW     = true
  CONFIG.BUILD_REGEX    = true
  CONFIG.BUILD_ZLIB     = true
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local empty = {}


local function makepackage(name)
  return {
    name         = name,
    links        = name,
    libdirs      = false,
    linkoptions  = false,
    defines      = false,
    includedirs  = false,
    buildoptions = false,
  }
end


PACKAGES = {
  ares     = makepackage('ares'),
  curl     = makepackage('curl'),
  freetype = makepackage('freetype'),
  ftgl     = makepackage('ftgl'),
  SDL      = makepackage('SDL'),
  GLEW     = makepackage('GLEW'),
  GL       = makepackage('GL'),
  GLU      = makepackage('GLU'),
  X11      = makepackage('X11'),
  z        = makepackage('z'),
  dl       = makepackage('dl'),
  rt       = makepackage('rt'),
}


function getpackage(name)
  return assert(PACKAGES[name], 'unknown package: ' .. name)
end


function linkpackage(name)
  local package = PACKAGES[name]
  if (not package) then
    error('Unknown package name: ' .. name)
  end
  if (package.links) then
    links(package.links)
  end
  if (package.libdirs) then
    libdirs(package.libdirs)
  end
  if (package.linkoptions) then
    linkoptions(package.linkoptions)
  end
end


function includepackage(name)
  local package = PACKAGES[name]
  if (not package) then
    error('Unknown package name: ' .. name)
  end
  if (package.defines) then
    defines(package.defines)
  end
  if (package.includedirs) then
    includedirs(package.includedirs)
  end
  if (package.buildoptions) then
    buildoptions(package.buildoptions)
  end
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


if (not _OPTIONS['help']) then

  include 'premake4_config'

  PACKAGES.freetype.includedirs = -- FIXME
    os.outputof('freetype-config --cflags'):match('%-I(.*)$') -- FIXME


  include 'other_src'
  include 'src'

  if (not _OPTIONS['disable-plugins'] and
      not _OPTIONS['disable-bzfs']) then
    include 'plugins'
  end

  include 'man'
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


newaction {
  trigger = 'superclean',
  description = 'make your directories squeaky clean',
  execute = function()
    print('SuperCleaning!')
    print('rmdir', BINDIR)
    os.rmdir(BINDIR)
    for _, dir in ipairs(os.matchdirs('**/.objs')) do
      print('rmdir', dir)
      os.rmdir(dir)
    end
    -- FIXME -- remove all Makefiles  (including <name>.make)
    -- FIXME -- remove all the possible build_<action> dirs
    -- FIXME -- remove all of the targets
  end,
}

newaction {
  trigger = 'install',
  description = 'install [-strip] [rootpath]',
  execute = function() print('"install" not implemented') end,
}

newaction {
  trigger = 'uninstall',
  description = 'uninstall [rootpath]',
  execute = function() print('"uninstall" not implemented') end,
}

newaction {
  trigger = 'debian',
  description = 'create the debian package',
  execute = function() print('"debian" not implemented') end,
}

newaction {
  trigger = 'source_zip',
  description = 'create the source code package',
  execute = function() print('"source_zip" not implemented') end,
}

newaction {
  trigger = 'source_tgz',
  description = 'create the source code package',
  execute = function() print('"source_zip" not implemented') end,
}

newaction {
  trigger = 'source_bz2',
  description = 'create the source code package',
  execute = function() print('"source_zip" not implemented') end,
}

newaction {
  trigger = 'distclean',
  description = 'ablate until it hurts',
  execute = function() print('"distclean" not implemented') end,
}

newaction {
  trigger = 'distcheck',
  description = 'distribution check',
  execute = function() print('"distcheck" not implemented') end,
}

newaction {
  trigger = 'svn_add_files',
  description = 'add files to svn, with svn:mime-type and eol-style props',
  execute = tools and tools.svn_add_files or nil,
}

if (not os.is('windows')) then
  newaction {
    trigger = 'easy_links',
    description = 'add a few soft links in src/ for console jockeys',
    execute = function()
      os.execute('ln -sf bzfs       src/Server')
      os.execute('ln -sf bzflag     src/BZFlag')
      os.execute('ln -sf clientbase src/Clientbase')
      os.execute('ln -sf bzadmin    src/Admin')
      os.execute('ln -sf bzrobots   src/Robots')
    end
  }
end

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

if (-1 > 0) then
  for k in pairs(ALL_FILES) do
    print('ALL_FILES', k)
  end
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

print()
print(('-'):rep(80))
print(('-'):rep(80))
print()

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  Report the configuration
--

if ((not _OPTIONS['help']) and (_ACTION ~= 'clean')) then -- FIXME -- setup isbuildaction()
  _LASTCALL = function ()
    print()
    print(('-'):rep(80))
    print(('-'):rep(80))
    print()

    printf('%s-%s.%s',
           CONFIG.PACKAGE_NAME, CONFIG.PACKAGE_VERSION, CONFIG.CONFIG_DATE)
    print()

    local function libbuildstate(x)
      return x and 'yes' or 'no   (using system)'
    end
    print('  libraries:')
    print()
    print('    build ares ....... ' .. libbuildstate(CONFIG.BUILD_ARES))
    print('    build curl ....... ' .. libbuildstate(CONFIG.BUILD_CURL))
    print('    build freetype ... ' .. libbuildstate(CONFIG.BUILD_FREETYPE))
    print('    build ftgl ....... ' .. libbuildstate(CONFIG.BUILD_FTGL))
    print('    build glew ....... ' .. libbuildstate(CONFIG.BUILD_GLEW))
    print('    build regex ...... ' .. libbuildstate(CONFIG.BUILD_REGEX))
    print('    build zlib ....... ' .. libbuildstate(CONFIG.BUILD_ZLIB))
    print()
    print('  executables:')
    print()
    print('    BZFlag client .... ' .. (_OPTIONS['disable-bzflag']   and 'no' or 'yes'))
    print('    BZFlag server .... ' .. (_OPTIONS['disable-bzfs']     and 'no' or 'yes'))
    print('    BZAdmin client ... ' .. (_OPTIONS['disable-bzadmin']  and 'no' or 'yes'))
    print('    BZRobots client .. ' .. (_OPTIONS['disable-bzrobots'] and 'no' or 'yes'))

    print()
    print(('-'):rep(80))
    print(('-'):rep(80))
  end
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

