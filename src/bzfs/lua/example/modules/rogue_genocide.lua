--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

if (WantConfig) then
  return {
    name     = 'rogue_genocide',
    desc     = 'Rogue Genocide -- based on the C++ module',
    author   = 'trepan',
    date     = 'Nov 22, 2008',
    license  = 'LGPL 2.1',
    enabled  = true,
    niceness = 0,
  }
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local noSuicide = true

function CallIn.PlayerDied(victimID, team, killerID,
                           killerTeam, flagType, shotID)
  if (flagType ~= 'G') then
    return -- wrong flag type
  end

  if (team ~= bz.TEAM.ROGUE) then
    return -- wrong team
  end

  if (noSuicide and (victim == killerID)) then
    return -- no suicide
  end

  local players = bz.GetPlayerIDs()
  for i = 1, #players do
    local pid = players[i]
    local team = bz.GetPlayerTeam(pid)
    if (team and (team == bz.TEAM.ROGUE)) then
      if (bz.GetPlayerSpawned(pid)) then
        bz.KillPlayer(pid, false, killerID, 'G')
        bz.SendMessage(bz.PLAYER.SERVER, pid, 'You were a victim of Rogue Genocide')
        if (pid == killerID) then
          bz.SendMessage(bz.PLAYER.SERVER, pid,
                         'You should be more careful with Genocide!')
        end
      end
    end
  end
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
