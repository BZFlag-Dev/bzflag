--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  Test code for the bzfs lua plugin
--


print('-- bzfs.lua --')


local pluginDir = BZ.GetPluginDirectory()

print('pluginDir = ' .. pluginDir)


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
-- print everything in the global table
--

local printed = {} -- tables that have already been printed (avoid looping)

local function PrintTable(t, indent)
  if (printed[t]) then
    print(indent .. '-- ALREADY PRINTED')
    return
  else
    printed[t] = true
  end

  indent = indent or ''
  for k,v in pairs(t) do
    if (type(v) ~= 'table') then
      print(indent .. tostring(k) .. ' = ' .. tostring(v) .. ',')
    else
      print(indent .. tostring(k) .. ' = {')
      PrintTable(v, indent .. '  ')
      print(indent .. '},')
    end
  end
end

print()
print(string.rep('-', 80))
PrintTable(_G)
print(string.rep('-', 80))
print()


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  Call-Ins
--


function RawChatMessageEvent(msg, from, to, team)
  print('bzfs.lua', msg, from, to, team)
  return msg .. ' -- lua tagged'
end


function GetPlayerSpawnPosEvent(pid, team, px, py, pz, r)
  print('GetPlayerSpawnPosEvent', pid, team, px, py, pz, r)
  return 0, 0, 10, 0
end


function TickEvent()
--  do return end -- FIXME
  for _, pid in ipairs(BZ.GetPlayerIDs()) do
    print(pid)
    print(BZ.GetPlayerName(pid))
    print(BZ.GetPlayerStatus(pid))
    print(BZ.GetPlayerPosition(pid))
    print(BZ.GetPlayerVelocity(pid))
    print(BZ.GetPlayerRotation(pid))
    print(BZ.GetPlayerAngVel(pid))
    print()
  end
end


function UnknownSlashCommand(msg, playerID)
  print('bzfs.lua', 'UnknownSlashCommand', playerID, msg)
  local _, _, cmd = msg:find('/run%s+(.*)')
  if (cmd) then
    local chunk, err = loadstring(cmd, 'doline')
    if (chunk == nil) then
      print('doline error: ' .. err)
    else
      local success, err = pcall(chunk)
      if (not success) then
        print(err)
      end
    end
    return true
  end    
  return false
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

-- setup the blocked event map
local blockedEvents = {
  'TickEvent',
  'LoggingEvent',
  'PlayerUpdateEvent',
  'NetDataSendEvent',
  'NetDataReceiveEvent',
}
local tmpSet = {}
for _, name in ipairs(blockedEvents) do
  tmpSet[name] = true
end
blockedEvents = tmpSet


-- update the desired call-ins
for name, code in pairs(BZ.GetCallInList()) do
  if (type(_G[name]) == 'function') then
    BZ.UpdateCallIn(name, _G[name])
  elseif (not blockedEvents[name]) then 
    BZ.UpdateCallIn(name, function(...)
      print('bzfs.lua', name, ...)
    end)
  end
end


BZ.UpdateCallIn('TickEvent', nil) -- annoying, but leave the function defined


-- print the current call-in map
for name, state in pairs(BZ.GetCallInList()) do
  print(name, state)
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

BZ.UpdateCallIn('AllowFlagGrabEvent',
  function(playerID, flagID, flagType, shotType, px, py, pz)
    if (BZ.GetPlayerTeam(playerID) == Constants.Teams.red) then
      return false
    end
  end
)


BZ.UpdateCallIn('BZDBChange',
  function(key, value)
    print('BZDBChange: ' .. key .. ' = ' .. value)
  end
)


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

BZ.CustomObject('custom_block')
BZ.CustomObject('custom_block1')
BZ.CustomObject('custom_block2')
BZ.CustomObject('custom_block3')

BZ.UpdateCallIn('CustomMapObject',
  function(name, data)
    print('CustomMapObject:  ' .. name)
    for d = 1, #data do
      print('CustomMapObject:    ' .. data[d])
    end
  end
)


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

for _, name in pairs(BZ.DB.GetList()) do
  print(name, 
        BZ.DB.GetInt(name),
        BZ.DB.GetBool(name),
        BZ.DB.GetFloat(name),
        BZ.DB.GetString(name))
end

BZ.DB.SetString('_mirror', 'black 0.5')
BZ.DB.SetString('_skyColor', 'red')
BZ.DB.SetFloat('_tankSpeed', '50.0')



for k,v in lfs.dir('.') do
  print(k,v)
end


require('plugins')