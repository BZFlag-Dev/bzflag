--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

if (WantConfig) then
  return {
    name     = 'phoenix',
    desc     = 'Phoenix -- based on the C++ module',
    author   = 'trepan',
    date     = 'Nov 22, 2008',
    license  = 'LGPL 2.1',
    enabled  = true,
    niceness = 0,
  }
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local prevSpawns = {}  --  < playerID = { px, py, pz, rot } >  pairs


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

function CallIn.GetPlayerSpawnPos(playerID, team, px, py, pz, rot)
  local state = prevSpawns[playerID]
  if (state) then
    return unpack(state)
  end
end


function CallIn.PlayerParted(playerID, team, callsign, reason)
  prevSpawns[playerID] = nil
end


function CallIn.PlayerDied(playerID, team, killerID, killerTeam,
                           flagType, shotID, px, py, pz, rot)
  prevSpawns[playerID] = { px, py, pz, rot }
end


function CallIn.Capture(teamCapped, teamCapping, playerCapping, px, py, pz, rot)
  -- clear previous spawns for capped team members
  for _, pid in ipairs(BZ.GetPlayerIDs()) do
    if (prevSpawns[pid]) then
      local team = BZ.GetPlayerTeam(pid)
      if (team and (team == teamCapped)) then
        prevSpawns[pid] = nil
      end
    end
  end
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
