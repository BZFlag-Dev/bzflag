--------------------------------------------------------------------------------

include 'plugin_utils'

--------------------------------------------------------------------------------

local PLUGINS_DIR      = os.getcwd()
local PLUGIN_LINKS_DIR = PLUGINS_DIR .. '/links'

function plugin_project(name, source_files)
  project('plugin_' .. name)
    hidetarget('true')
    kind 'SharedLib'
    language 'C++'
    targetname(name)
    targetprefix ''
    objdir '.objs'
    files(source_files)
    links 'plugin_utils'
    includedirs { '../../src/bzfs', '../plugin_utils' }

    if (not _ACTION:match('^vs')) then
      buildoptions '-Wextra -Wundef -Wshadow -Wno-long-long -ansi -pedantic'
    end

    -- create missing .def files for windows plugins
    if (os.is('vs*')) then
      local defName = name .. '.def'
      if (not os.isfile(defName)) then
        print('Generating ' .. os.getcwd() .. '/' .. defName)
        local defFile = assert(io.open(defName, 'wb'))
        defFile:write(''
          .. "LIBRARY\t"..name.."\r\n"
          .. "DESCRIPTION  '"..name.." Windows Dynamic Link Library'\r\n"
          .. "\r\n"
          .. "EXPORTS\r\n"
          .. "    ; Explicit exports can go here\r\n"
          .. "\r\n"
          .. "bz_GetVersion\r\n"
          .. "bz_Load\r\n"
          .. "bz_Unload\r\n"
        )
        defFile:close()
      end
    end

--[[
    -- create soft links to the plugin .so's in plugins/links/
    configuration 'not vs*'
      prelinkcommands {
        ('@if [ ! -d %q ]; then mkdir %q; fi')
        :format(PLUGIN_LINKS_DIR, PLUGIN_LINKS_DIR),
        ('@ln -sf %q %q')
        :format(os.getcwd()..'/'..name..'.so', PLUGIN_LINKS_DIR),
      }
--]]
end

--------------------------------------------------------------------------------

include 'SAMPLE_PLUGIN'
include 'airspawn'
include 'bzfscron'
include 'chathistory'
include 'chatlog'
include 'customflagsample'
include 'fastmap'
include 'flagStay'
include 'hiddenAdmin'
include 'HoldTheFlag'
include 'HTTPServer'
include 'httpTest'
include 'keepaway'
include 'killall'
include 'koth'
include 'logDetail'
include 'mapchange'
include 'nagware'
include 'Phoenix'
include 'playHistoryTracker'
--include 'python'
include 'rabidRabbit'
include 'recordmatch'
include 'regFlag'
include 'RogueGenocide'
include 'serverControl'
include 'serverSideBotSample'
include 'shockwaveDeath'
include 'soundTest'
include 'teamflagreset'
include 'thiefControl'
include 'timedctf'
include 'unrealCTF'
include 'weaponArena'
include 'webadmin'
include 'webReport'
include 'webstats'
include 'wwzones'

--------------------------------------------------------------------------------

plugin_project = nil

--------------------------------------------------------------------------------
