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
    niceness = -100, -- not very nice
  }
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

function CallIn.AllowCTFCaptureEvent(teamCapped, teamCapping, playerCapping,
                                     px, py, pz, rot)
  print('AllowCTFCaptureEvent',
        teamCapped, teamCapping, playerCapping, px, py, pz, rot)
  -- return:  bool allow [, killTeam]
end


function CallIn.AllowFlagGrabEvent(playerID, flagID, flagType, shotType,
                                   px, py, pz)
  print('AllowFlagGrabEvent',
        playerID, flagID, flagType, shotType, px, py, pz)
  -- return:  bool allow
end


function CallIn.AllowKillCommandEvent(victimID, killerID)
  print('AllowKillCommandEvent', victimID, killerID)
  -- return:  bool allow
end


function CallIn.AllowPlayer(playerID, callsign, ipAddress)
  print('AllowPlayer', playerID, callsign, ipAddress)
  -- return:  string reason
end


function CallIn.AllowSpawn(playerID, teamID)
  print('AllowSpawn', playerID, teamID)
  -- return:  bool allow
end


function CallIn.AnointRabbitEvent(playerID)
  print('AnointRabbitEvent', playerID)
  -- return:  number playerID
end


function CallIn.BanEvent(bannerID, duration, reason, banneeID, ipAddress)
  print('BanEvent', bannerID, duration, reason, banneeID, ipAddress)
end


function CallIn.BZDBChange(key, value)
  print('BZDBChange', key, value)
end


function CallIn.CaptureEvent(teamCapped, teamCapping, playerCapping,
                             px, py, pz, rot)
  print('CaptureEvent',
        teamCapped, teamCapping, playerCapping, px, py, pz, rot)
end


function CallIn.CustomMapObject(objName, data)
  print('CustomMapObject', objName)
  for d = 1, #data do
    print('  ' .. data[d])
  end
end


function CallIn.FilteredChatMessageEvent(msg, from, to, team)
  print('FilteredChatMessageEvent', msg, from, to, team)
end


function CallIn.FlagDroppedEvent(playerID, flagID, flagType, px, py, pz)
  print('FlagDroppedEvent', playerID, flagID, flagType, px, py, pz)
end


function CallIn.FlagGrabbedEvent(playerID, flagID, flagType, shotType,
                                 px, py, pz)
  print('FlagGrabbedEvent', playerID, flagID, flagType, shotType, px, py, pz)
end


function CallIn.FlagResetEvent(flagID, flagType, px, py, pz, teamIsEmpty)
  print('FlagResetEvent', flagID, flagType, px, py, pz, teamIsEmpty)
end


function CallIn.FlagTransferredEvent(srcPlayerID, dstPlayerID, flagType)
  print('FlagTransferredEvent', srcPlayerID, dstPlayerID, flagType)
  -- return:  number action  -- FIXME, need constants
end


function CallIn.GameEndEvent(duration)
  print('GameEndEvent', duration)
end


function CallIn.GameStartEvent(duration)
  print('GameStartEvent', duration)
end


function CallIn.GetAutoTeamEvent(playerID, team, callsign)
  print('GetAutoTeamEvent', playerID, team, callsign)
  -- return:  number team
end


function CallIn.GetPlayerInfoEvent(playerID, team, callsign, idAddress,
                                   admin, verified, registered)
  print('GetPlayerInfoEvent',
        playerID, team, callsign, idAddress, admin, verified, registered)
  -- return:  bool adm, bool verified, bool reg'd
end


function CallIn.GetPlayerSpawnPosEvent(playerID, team, px, py, pz, rot)
  print('GetPlayerSpawnPosEvent', playerID, team, px, py, pz, rot)
  -- return:  px, py, pz, rot
end


function CallIn.GetWorldEvent(generated, ctf, rabbit, openFFA,
                              worldFile, worldBlob)
  print('GetWorldEvent', generated, ctf, rabbit, openFFA, worldFile)
end


function CallIn.HostBanModifyEvent(bannerID, duration, reason, hostPattern)
  print('HostBanModifyEvent', bannerID, duration, reason, hostPattern)
end


function CallIn.HostBanNotifyEvent(bannerID, duration, reason, hostPattern)
  print('HostBanNotifyEvent', bannerID, duration, reason, hostPattern)
end


function CallIn.IdBanEvent(bannerID, duration, reason, banneeID, bzID)
  print('IdBanEvent', bannerID, duration, reason, banneeID, bzID)
end


function CallIn.IdleNewNonPlayerConnection(connID, connData)
  print('IdleNewNonPlayerConnection', connID, #connData)
end


function CallIn.KickEvent(kickerID, kickedID, reason)
  print('KickEvent', kickerID, kickedID, reason)
end


function CallIn.KillEvent(killerID, killedID, reason)
  print('KillEvent', killerID, killedID, reason)
end


function CallIn.ListServerUpdateEvent(addr, desc, groups)
  print('ListServerUpdateEvent', addr, desc, groups)
end


function CallIn.LoggingEvent(msg, level)
  print('LoggingEvent', level, msg)
end


function CallIn.MessageFilteredEvent(playerID, rawMsg, filteredMsg)
  print('MessageFilteredEvent', playerID, rawMsg, filteredMsg)
end


function CallIn.NetDataReceiveEvent(playerID, udp, data)
  print('NetDataReceiveEvent', playerID, udp, #data)
end


function CallIn.NetDataSendEvent(playerID, udp, data)
  print('NetDataSendEvent', playerID, udp, #data)
end


function CallIn.NewNonPlayerConnection(playerID, data)
  print('NewNonPlayerConnection', playerID, #data)
end


function CallIn.NewRabbitEvent(playerID)
  print('NewRabbitEvent', playerID)
end


function CallIn.PlayerAuthEvent(playerID, password, globalAuth)
  print('PlayerAuthEvent', playerID, password, globalAuth)
end


function CallIn.PlayerCollision(playerID1, playerID2, px, py, pz)
  print('PlayerCollision', playerID1, playerID2, px, py, pz)
  -- return:  bool handled
end


function CallIn.PlayerCustomDataChanged(playerID, key, data)
  print('PlayerCustomDataChanged', playerID, key, #data)
end


function CallIn.PlayerDieEvent(victimID, team, killerID, killerTeam,
                               flagType, shotID, px, py, pz, rot)
  print('PlayerDieEvent',
        playerID, team, killerID, killerTeam, flagType, shotID, px, py, pz, rot)
end


function CallIn.PlayerJoinEvent(playerID, team, callsign)
  print('PlayerJoinEvent', playerID, team, callsign)
end


function CallIn.PlayerPartEvent(playerID, team, callsign, reason)
  print('PlayerPartEvent', playerID, team, callsign, reason)
end


function CallIn.PlayerPausedEvent(playerID, paused)
  print('PlayerPausedEvent', playerID, paused)
end


function CallIn.PlayerSentCustomData(playerID, key, data)
  print('PlayerSentCustomData', playerID, key, #data)
end


function CallIn.PlayerSpawnEvent(playerID, team)
  print('PlayerSpawnEvent', playerID, team)
end


function CallIn.PlayerUpdateEvent(playerID, status, phydrv,
                                  falling, crossingWall, inPhantomZone,
                                  px, py, pz, rot, vx, vy, vz, angvel)
  print('PlayerUpdateEvent', playerID, status, phydrv,
        falling, crossingWall, inPhantomZone,
        px, py, pz, rot, vx, vy, vz, angvel)
end


function CallIn.RawChatMessageEvent(msg, from, to, team)
  print('RawChatMessageEvent', msg, from, to, team)
  -- return:  string newMsg
end


function CallIn.ReloadEvent(playerID)
  print('ReloadEvent', playerID)
end


function CallIn.ReportFiledEvent(playerID, msg)
  print('ReportFiledEvent', playerID, msg)
end


function CallIn.ServerMsgEvent(to, team, msg)
  print('ServerMsgEvent', to, team, msg)
end


function CallIn.ShotEndedEvent(playerID, shotID)
  print('ShotEndedEvent', playerID, shotID)
end


function CallIn.ShotFiredEvent(playerID, shotType, px, py, pz)
  print('ShotFiredEvent', playerID, shotType, px, py, pz)
end


function CallIn.SlashCommandEvent(msg, from)
  print('SlashCommandEvent', msg, from)
end


function CallIn.TeleportEvent(playerID, src, dst)
  print('TeleportEvent', playerID, src, dst)
end


function CallIn.TickEvent()
  print('TickEvent')
end


function CallIn.UnknownSlashCommand(msg, from)
  print('UnknownSlashCommand', msg, from)
  -- return:  bool handled
end


function CallIn.WorldFinalized()
  print('WorldFinalized')
end


function CallIn.ZoneEntryEvent()
  print('ZoneEntryEvent')
end


function CallIn.ZoneExitEvent()
  print('ZoneExitEvent')
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
