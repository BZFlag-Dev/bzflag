--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

if (WantConfig) then
  return {
    name     = 'debug',
    desc     = 'debug plugin',
    author   = 'trepan',
    date     = 'Nov 22, 2008',
    license  = 'LGPL 2.1',
    enabled  = true,
    niceness = -100, -- not very nice, runs first
  }
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

function CallIn.AllowCTFCapture(teamCapped, teamCapping, playerCapping,
                                px, py, pz, rot)
  print('AllowCTFCapture',
        teamCapped, teamCapping, playerCapping, px, py, pz, rot)
  -- return:  bool allow [, bool killTeam]
end


function CallIn.AllowFlagGrab(playerID, flagID, flagType, shotType, px, py, pz)
  print('AllowFlagGrab',
        playerID, flagID, flagType, shotType, px, py, pz)
  -- return:  bool allow
end


function CallIn.AllowKillCommand(victimID, killerID)
  print('AllowKillCommand', victimID, killerID)
  -- return:  bool allow
end


function CallIn.AllowPlayer(playerID, callsign, ipAddress)
  print('AllowPlayer', playerID, callsign, ipAddress)
  -- return:  bool allow [, string reason]
end


function CallIn.AllowSpawn(playerID, teamID)
  print('AllowSpawn', playerID, teamID)
  -- return:  bool allow
end


function CallIn.AnointRabbit(playerID)
  print('AnointRabbit', playerID)
  -- return:  number playerID
end


function CallIn.Ban(bannerID, duration, reason, banneeID, ipAddress)
  print('Ban', bannerID, duration, reason, banneeID, ipAddress)
end


function CallIn.BZDBChange(key, value)
  print('BZDBChange', key, value)
end


function CallIn.Capture(teamCapped, teamCapping, playerCapping,
                        px, py, pz, rot)
  print('Capture',
        teamCapped, teamCapping, playerCapping, px, py, pz, rot)
end


function CallIn.FilteredChatMessage(msg, from, to, team)
  print('FilteredChatMessage', msg, from, to, team)
end


function CallIn.FlagDropped(playerID, flagID, flagType, px, py, pz)
  print('FlagDropped', playerID, flagID, flagType, px, py, pz)
end


function CallIn.FlagGrabbed(playerID, flagID, flagType, shotType, px, py, pz)
  print('FlagGrabbed', playerID, flagID, flagType, shotType, px, py, pz)
end


function CallIn.FlagReset(flagID, flagType, px, py, pz, teamIsEmpty)
  print('FlagReset', flagID, flagType, px, py, pz, teamIsEmpty)
end


function CallIn.FlagTransferred(srcPlayerID, dstPlayerID, flagType)
  print('FlagTransferred', srcPlayerID, dstPlayerID, flagType)
  -- return:  number action  -- FIXME, need constants
end


function CallIn.GameEnd(duration)
  print('GameEnd', duration)
end


function CallIn.GameStart(duration)
  print('GameStart', duration)
end


function CallIn.GetAutoTeam(playerID, team, callsign)
  print('GetAutoTeam', playerID, team, callsign)
  -- return:  number team
end


function CallIn.GetPlayerInfo(playerID, team, callsign, idAddress,
                              admin, verified, registered)
  print('GetPlayerInfo',
        playerID, team, callsign, idAddress, admin, verified, registered)
  -- return:  bool adm, bool verified, bool reg'd
end


function CallIn.GetPlayerSpawnPos(playerID, team, px, py, pz, rot)
  print('GetPlayerSpawnPos', playerID, team, px, py, pz, rot)
  -- return:  px, py, pz, rot
end


function CallIn.GetWorld(generated, ctf, rabbit, openFFA, worldFile, worldBlob)
  print('GetWorld', generated, ctf, rabbit, openFFA, worldFile)
end


function CallIn.HostBan(bannerID, duration, reason, hostPattern)
  print('HostBan', bannerID, duration, reason, hostPattern)
end


function CallIn.IdBan(bannerID, duration, reason, banneeID, bzID)
  print('IdBan', bannerID, duration, reason, banneeID, bzID)
end


function CallIn.IdleNewNonPlayerConnection(connID, connData)
  print('IdleNewNonPlayerConnection', connID, #connData)
end


function CallIn.Kick(kickerID, kickedID, reason)
  print('Kick', kickerID, kickedID, reason)
end


function CallIn.Kill(killerID, killedID, reason)
  print('Kill', killerID, killedID, reason)
end


function CallIn.ListServerUpdate(addr, desc, groups)
  print('ListServerUpdate', addr, desc, groups)
end


function CallIn.Logging(msg, level)
  print('Logging', level, msg)
end


function CallIn.MessageFiltered(playerID, rawMsg, filteredMsg)
  print('MessageFiltered', playerID, rawMsg, filteredMsg)
end


function CallIn.NetDataReceive(playerID, udp, data)
  print('NetDataReceive', playerID, udp, #data)
end


function CallIn.NetDataSend(playerID, udp, data)
  print('NetDataSend', playerID, udp, #data)
end


function CallIn.NewNonPlayerConnection(playerID, data)
  print('NewNonPlayerConnection', playerID, #data)
end


function CallIn.NewRabbit(playerID)
  print('NewRabbit', playerID)
end


function CallIn.PlayerAuth(playerID, password, globalAuth)
  print('PlayerAuth', playerID, password, globalAuth)
end


function CallIn.PlayerCollision(playerID1, playerID2, px, py, pz)
  print('PlayerCollision', playerID1, playerID2, px, py, pz)
  -- return:  bool handled
end


function CallIn.PlayerCustomDataChanged(playerID, key, data)
  print('PlayerCustomDataChanged', playerID, key, #data)
end


function CallIn.PlayerDied(victimID, team, killerID, killerTeam,
                           flagType, shotID, px, py, pz, rot)
  print('PlayerDied',
        playerID, team, killerID, killerTeam, flagType, shotID, px, py, pz, rot)
end


function CallIn.PlayerJoined(playerID, team, callsign)
  print('PlayerJoined', playerID, team, callsign)
end


function CallIn.PlayerParted(playerID, team, callsign, reason)
  print('PlayerParted', playerID, team, callsign, reason)
end


function CallIn.PlayerPaused(playerID, paused)
  print('PlayerPaused', playerID, paused)
end


function CallIn.PlayerSentCustomData(playerID, key, data)
  print('PlayerSentCustomData', playerID, key, #data)
end


function CallIn.PlayerSpawned(playerID, team, px, py, pz, rot)
  print('PlayerSpawned', playerID, team, px, py, pz, rot)
end


function CallIn.PlayerUpdate(playerID, status, phydrv,
                             falling, crossingWall, inPhantomZone,
                             px, py, pz, rot, vx, vy, vz, angvel)
  print('PlayerUpdate', playerID, status, phydrv,
        falling, crossingWall, inPhantomZone,
        px, py, pz, rot, vx, vy, vz, angvel)
end


function CallIn.RawChatMessage(msg, from, to, team)
  print('RawChatMessage', msg, from, to, team)
  -- return:  string newMsg
end


function CallIn.Reload(playerID)
  print('Reload', playerID)
end


function CallIn.ReportFiled(playerID, msg)
  print('ReportFiled', playerID, msg)
end


function CallIn.ServerMsg(to, team, msg)
  print('ServerMsg', to, team, msg)
end


function CallIn.ShotEnded(playerID, shotID)
  print('ShotEnded', playerID, shotID)
end


function CallIn.ShotFired(playerID, shotType, px, py, pz)
  print('ShotFired', playerID, shotType, px, py, pz)
end


function CallIn.Shutdown()
  print('Shutdown')
end


function CallIn.SlashCommand(msg, from)
  print('SlashCommand', msg, from)
end


function CallIn.Teleport(playerID, src, dst)
  print('Teleport', playerID, src, dst)
end


function CallIn.Tick()
  print('Tick')
end


function CallIn.UnknownSlashCommand(msg, from)
  print('UnknownSlashCommand', msg, from)
  -- return:  bool handled
end


function CallIn.WorldFinalized()
  print('WorldFinalized')
end


function CallIn.ZoneEntry()
  print('ZoneEntry')
end


function CallIn.ZoneExit()
  print('ZoneExit')
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
