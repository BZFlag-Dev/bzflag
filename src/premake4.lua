
solution 'bz'

language    'C++'
includedirs 'include'

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


--[[
configuration 'not vs*'
  prebuildcommands {
    'ln -sf bzfs    Server',
    'ln -sf bzflag  Client',
    'ln -sf bzadmin Admin',
  }
--]]