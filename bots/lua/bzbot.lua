--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  BZBOT.LUA
--
--  author:  Dave Rodgers (aka: trepan)
--  date:    Aug 14, 2009
--  brief:   a test script for the lua bzrobots interface
--  license: LGPL v2.1
--
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


print('Hello from bzbot.lua')


local callIns = {
  'BattleEnded',
  'BulletHit',
  'BulletMissed',
  'Death',
  'HitByBullet',
  'HitWall',
  'RobotDeath',
  'ScannedRobot',
  'Spawn',
  'Status',
  'Win',
}

-- assign the callins to a print function
for _,name in ipairs(callIns) do
  _G[name] = function(...)
    print('LUAROBOT', name, ...)
  end
end

-- remove noisy call-ins
Status = nil
ScannedRobot = nil

-- noisy test code
function ScannedRobotFIXME(time, priority, info)
  print('Scanned')
  for k,v in pairs(info) do
    print('', k, v)
  end
end


--------------------------------------------------------------------------------

local function DoReadLine()
  if (not bz.ReadStdin) then
    return
  end
  
  local line = bz.ReadStdin()
  if (not line) then
    return
  end
  
  print('ReadStdin(): ' .. line:gsub('\n', ''))

  local chunk, err = loadstring(line)
  if (not chunk) then
    print(err)
    return
  end
  
  local success, err = pcall(chunk)
  if (not success) then
    print(err)
  end
end


--------------------------------------------------------------------------------

function Run()
  while (true) do
    DoReadLine()
    bz.DoNothing()
  end
end


--------------------------------------------------------------------------------

do -- protect Run() from errors
  local OrigRun = Run
  Run = function()
    while (true) do
      local success, err = xpcall(OrigRun, debug.traceback)
      if (not success) then
        print('Run() error:' .. err)
      end
    end
  end
end


--------------------------------------------------------------------------------
