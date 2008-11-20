--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

if (WantConfig) then
  return {
    name     = 'air_spawn',
    desc     = 'Air Spawn -- based on the C++ module',
    author   = 'trepan',
    date     = 'Nov 22, 2008',
    license  = 'LGPL 2.1',
    enabled  = true,
    niceness = 0,
  }
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


local height = 10

local random = math.random


local function SetHeight()
  height = BZ.DB.GetFloat('_airSpawnHeight')
  if (height < 0.001) then
    height = 10.0
  end
end


function CallIn.BZDBChange(key, value)
  if (key == '_airSpawnHeight') then
    SetHeight()
  end
end


function CallIn.GetPlayerSpawnPosEvent(playerID, team, px, py, pz, rot)
  py = py + (height * random())
  return px, py, pz, rot
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
