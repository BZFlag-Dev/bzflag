--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

if (WantConfig) then
  return {
    name     = 'debug',
    desc     = 'debug plugin',
    author   = 'trepan',
    date     = 'Nov 22, 2008',
    license  = 'LGPL 2.0',
    enabled  = true,
    niceness = -100, -- not very nice
  }
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

function plugin.AllowCTFCaptureEvent(teamCapped, teamCapping, playerCapping,
                                     px, py, pz, rot)
  print('AllowCTFCaptureEvent',
        teamCapped, teamCapping, playerCapping, px, py, pz, rot)
end


function plugin.AllowFlagGrabEvent(playerID, flagID, flagType, shotType,
                                   px, py, pz)
  print('AllowFlagGrabEvent',
        playerID, flagID, flagType, shotType, px, py, pz)
end


function plugin.AllowKillCommandEvent(victimID, killerID)
  print('AllowKillCommandEvent', victimID, killerID)
end


function plugin.AllowPlayer(playerID, callsign, ipAddress)
  print('AllowPlayer', playerID, callsign, ipAddress)
end


function plugin.AllowSpawn(playerID, teamID)
  print('AllowSpawn', playerID, teamID)
end


function plugin.AnointRabbitEvent(playerID)
  print('AnointRabbitEvent', playerID)
end


function plugin.BanEvent(bannerID, duration, reason, banneeID, ipAddress)
  print('BanEvent', bannerID, duration, reason, banneeID, ipAddress)
end


function plugin.CaptureEvent(teamCapped, teamCapping, playerCapping,
                             px, py, pz, rot)
  print('CaptureEvent',
        teamCapped, teamCapping, playerCapping, px, py, pz, rot)
end


function plugin.CustomMapObject(objName, data)
  print('CustomMapObject', objName)
  for d = 1, #data do
    print('  ' .. data[d])
  end
end


function plugin.FilteredChatMessageEvent(msg, from, to, team)
  print('FilteredChatMessageEvent', msg, from, to, team)
end


function plugin.FlagDroppedEvent(playerID, flagID, flagType, px, py, pz)
  print('FlagDroppedEvent', playerID, flagID, flagType, px, py, pz)
end


function plugin.FlagGrabbedEvent(playerID, flagID, flagType, shotType,
                                 px, py, pz)
  print('FlagGrabbedEvent', playerID, flagID, flagType, shotType, px, py, pz)
end


function plugin.FlagResetEvent(flagID, flagType, px, py, pz, teamIsEmpty)
  print('FlagResetEvent', flagID, flagType, px, py, pz, teamIsEmpty)
end


function plugin.FlagTransferredEvent(srcPlayerID, dstPlayerID, flagType)
  print('FlagTransferredEvent', srcPlayerID, dstPlayerID, flagType)
end


function plugin.GameEndEvent(duration)
  print('GameEndEvent', duration)
end


function plugin.GameStartEvent(duration)
  print('GameStartEvent', duration)
end


function plugin.GetAutoTeamEvent(playerID, team, callsign)
  print('GetAutoTeamEvent', playerID, team, callsign)
end


function plugin.GetPlayerInfoEvent(playerID, team, callsign, idAddress,
                                   admin, verified, registered)
  print('GetPlayerInfoEvent',
        playerID, team, callsign, idAddress, admin, verified, registered)
end


function plugin.GetPlayerSpawnPosEvent(playerID, team, px, py, pz, rot)
  print('GetPlayerSpawnPosEvent', playerID, team, px, py, pz, rot)
end


function plugin.GetWorldEvent(generated, ctf, rabbit, openFFA,
                              worldFile, worldBlob)
  print('GetWorldEvent', generated, ctf, rabbit, openFFA, worldFile)
end


function plugin.HostBanModifyEvent(bannerID, duration, reason, hostPattern)
  print('HostBanModifyEvent', bannerID, duration, reason, hostPattern)
end


function plugin.HostBanNotifyEvent(bannerID, duration, reason, hostPattern)
  print('HostBanNotifyEvent', bannerID, duration, reason, hostPattern)
end


function plugin.IdBanEvent(bannerID, duration, reason, banneeID, bzID)
  print('IdBanEvent', bannerID, duration, reason, banneeID, bzID)
end


function plugin.IdleNewNonPlayerConnection(connID, connData)
  print('IdleNewNonPlayerConnection', connID, #connData)
end


function plugin.KickEvent(kickerID, kickedID, reason)
  print('KickEvent', kickerID, kickedID, reason)
end


function plugin.KillEvent(killerID, killedID, reason)
  print('KillEvent', killerID, killedID, reason)
end


function plugin.ListServerUpdateEvent(addr, desc, groups)
  print('ListServerUpdateEvent', addr, desc, groups)
end


function plugin.LoggingEvent(msg, level)
  print('LoggingEvent', level, msg)
end


function plugin.MessageFilteredEvent(playerID, rawMsg, filteredMsg)
  print('MessageFilteredEvent', playerID, rawMsg, filteredMsg))
end


function plugin.NetDataReceiveEvent(playerID, udp, data)
  print('NetDataReceiveEvent', playerID, udp, #data)
end


function plugin.NetDataSendEvent(playerID, udp, data)
  print('NetDataSendEvent', playerID, udp, #data)
end


function plugin.NewNonPlayerConnection(playerID, data)
  print('NewNonPlayerConnection', playerID, #data)
end


function plugin.NewRabbitEvent(playerID)
  print('NewRabbitEvent', playerID)
end


function plugin.PlayerAuthEvent(playerID, password, globalAuth)
  print('PlayerAuthEvent', playerID, password, globalAuth)
end


function plugin.PlayerCollision(playerID1, playerID2, px, py, pz)
  print('PlayerCollision', playerID1, playerID2, px, py, pz)
end


function plugin.PlayerCustomDataChanged(playerID, key, data)
  print('PlayerCustomDataChanged', playerID, key, #data)
end


function plugin.PlayerDieEvent(playerID, team, killerID, killerTeam,
                               flagType, shotID)
  print('PlayerDieEvent',
        playerID, team, killerID, killerTeam, flagType, shotID)
end


function plugin.PlayerJoinEvent(playerID, team, callsign)
  print('PlayerJoinEvent', playerID, team, callsign)
end


function plugin.PlayerPartEvent(playerID, team, callsign, reason)
  print('PlayerPartEvent', playerID, team, callsign, reason)
end


function plugin.PlayerPausedEvent(playerID, paused)
  print('PlayerPausedEvent', playerID, paused)
end


function plugin.PlayerSentCustomData(playerID, key, data)
  print('PlayerSentCustomData', playerID, key, #data)
end


function plugin.PlayerSpawnEvent(playerID, team)
  print('PlayerSpawnEvent', playerID, team)
end


function plugin.PlayerUpdateEvent(playerID)
  print('PlayerUpdateEvent', playerID)
end


function plugin.RawChatMessageEvent(msg, from, to, team)
  print('RawChatMessageEvent', msg, from, to, team)
end


function plugin.ReloadEvent(playerID)
  print('ReloadEvent', playerID)
end


function plugin.ReportFiledEvent(playerID, msg)
  print('ReportFiledEvent', playerID, msg)
end


function plugin.ServerMsgEvent(to, team, msg)
  print('ServerMsgEvent', to, team, msg)
end


function plugin.ShotEndedEvent(playerID, shotID)
  print('ShotEndedEvent', playerID, shotID)
end


function plugin.ShotFiredEvent(playerID, shotType, px, py, pz)
  print('ShotFiredEvent', playerID, shotType, px, py, pz)
end


function plugin.SlashCommandEvent(msg, from)
  print('SlashCommandEvent', msg, from)
end


function plugin.TeleportEvent(playerID, src, dst)
  print('TeleportEvent', playerID, src, dst)
end


function plugin.TickEvent()
  print('TickEvent')
end


function plugin.UnknownSlashCommand(msg, from)
  print('UnknownSlashCommand', msg, from)
end


function plugin.WorldFinalized()
  print('WorldFinalized')
end


function plugin.ZoneEntryEvent()
  print('ZoneEntryEvent')
end


function plugin.ZoneExitEvent()
  print('ZoneExitEvent')
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
