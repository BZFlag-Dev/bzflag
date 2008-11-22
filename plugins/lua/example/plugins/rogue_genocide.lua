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

function CallIn.PlayerDieEvent(victimID, team, killerID,
                               killerTeam, flagType, shotID)
  if (flagType ~= 'G') then
    return -- wrong flag type
  end

  if (team ~= BZ.TEAM.ROGUE) then
    return -- wrong team
  end

  if (noSuicide and (victim == killerID)) then
    return -- no suicide
  end

  local players = BZ.GetPlayerIDs()
  for i = 1, #players do
    local pid = players[i]
    local team = BZ.GetPlayerTeam(pid)
    if (team and (team == BZ.TEAM.ROGUE)) then
      if (BZ.GetPlayerSpawned(pid)) then
        BZ.KillPlayer(pid, false, killerID, 'G')
        BZ.SendMessage(BZ.PLAYER.SERVER, pid, 'You were a victim of Rogue Genocide')
        if (pid == killerID) then
          BZ.SendMessage(BZ.PLAYER.SERVER, pid,
                         'You should be more careful with Genocide!')
        end
      end
    end
  end
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
