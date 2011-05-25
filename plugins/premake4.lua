--------------------------------------------------------------------------------

include 'plugin_utils'

--------------------------------------------------------------------------------

local PLUGINS_DIR      = os.getcwd()
local PLUGIN_LINKS_DIR = PLUGINS_DIR .. '/links'

function plugin_project(name, source_files)
  project('plugin_' .. name)
    kind 'SharedLib'
    targetname(name)
    targetprefix ''
    objdir '.objs'
    files(source_files)
    links 'plugin_utils'
    includedirs '../plugin_utils'
--[[
    -- create soft links to the plugin .so's in plugins/links/
    configuration 'not windows'
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
