--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

function bzexec_project(name)

  project(name)
    kind('ConsoleApp')
    language 'C++'
    includedirs(TOPDIR .. '/src/include')
    objdir('.objs')

  if (not _ACTION:match('^vs')) then
    buildoptions '-Wextra -Wundef -Wshadow -Wno-long-long -ansi -pedantic'
  end

  configuration 'not gmake'
    targetdir(BINDIR)

  project(name) -- clear the configuration state
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

function bzlib_project(name, options)
  assert(name:match('^lib'), 'poorly named library project: ' .. name)

  project(name)
    hidetarget('true')
    kind('StaticLib')
    targetname(name:sub(4)) -- strip the 'lib' part
    language 'C++'
    includedirs(TOPDIR .. '/src/include')
    objdir('.objs')

  if (not _ACTION:match('^vs')) then
    buildoptions '-Wextra -Wundef -Wshadow -Wno-long-long -ansi -pedantic'
  end

    configuration 'vs*'
      defines { '_LIB' }

  project(name) -- clear the configuration state
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

include 'include'
include 'common'
include 'date'
include 'net'

include 'game'
include 'clientbase'
include 'lua'
include 'obstacle'

include 'mediafile'
include '3D'
include 'geometry'
include 'ogl'
include 'platform'
include 'scene'

if (not _OPTIONS['disable-bzfs'])     then include 'bzfs'     end
if (not _OPTIONS['disable-bzflag'])   then include 'bzflag'   end
if (not _OPTIONS['disable-bzadmin'])  then include 'bzadmin'  end
if (not _OPTIONS['disable-bzrobots']) then include 'bzrobots' end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

--[[
configuration 'not vs*'
  prebuildcommands {
    'ln -sf bzfs    Server',
    'ln -sf bzflag  Client',
    'ln -sf bzadmin Admin',
  }
--]]