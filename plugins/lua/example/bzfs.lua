--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  Test code for the bzfs lua plugin
--


do
  -- replace the function to make this value permanent
  local pluginDir = BZ.GetPluginDirectory()
  BZ.GetPluginDirectory = function() return pluginDir end
end


do
  local chunk, err = loadfile(BZ.GetLuaDirectory() .. 'utils.lua')
  if (not chunk) then
    error(err)
  end
  chunk()
end


function BZ.Print(...)
  print(...)
  local table = {...}
  local msg = ''
  for i = 1, #table do
    if (i ~= 1) then msg = msg .. '\t' end
    msg = msg .. tostring(table[i])
  end
  BZ.SendMessage(BZ.PLAYER.SERVER, BZ.PLAYER.ALL, msg)
end


BZ.Print('-- bzfs.lua --')


local pluginDir = BZ.GetPluginDirectory()

BZ.Print('luaDir    = ' .. BZ.GetLuaDirectory())
BZ.Print('pluginDir = ' .. BZ.GetPluginDirectory())


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
-- print everything in the global table
--

if (false) then
  print()
  print(string.rep('-', 80))
  table.print(_G, BZ.Print)
  print(string.rep('-', 80))
  print()
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  Call-Ins
--


function RawChatMessageEvent(msg, src, dst, team)
  print('bzfs.lua', 'RawChatMessageEvent', msg, src, dst, team)
  return msg .. ' -- lua tagged'
end


function FilteredChatMessageEvent(msg, src, dst, team)
end


function GetPlayerSpawnPosEvent(pid, team, px, py, pz, r)
  print('GetPlayerSpawnPosEvent', pid, team, px, py, pz, r)
  return 0, 0, 10, 0
end



local function ExecuteLine(line)
  print('LUA STDIN: ' .. line)
  local chunk, err = loadstring(line, 'doline')
  if (chunk == nil) then
    print('COMPILE ERROR: ' .. err)
  else
    local success, err = tracepcall(chunk)
    if (not success) then
      print('CALL ERROR: ' .. err)
    end
  end
end


function TickEvent()

  BZ.SetMaxWaitTime('luaTick', 0.05)

  local data = BZ.ReadStdin()
  if (data) then
    for line in data:gmatch('[^\n]+') do
      print()
      ExecuteLine(line)
      print()
    end
  end    

  if (false) then
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
for name, code in pairs(BZ.GetCallIns()) do
  if (type(_G[name]) == 'function') then
    BZ.UpdateCallIn(name, _G[name])
  elseif (not blockedEvents[name]) then 
    BZ.UpdateCallIn(name, function(...)
      print('bzfs.lua', name, ...)
    end)
  end
end


--BZ.UpdateCallIn('TickEvent', nil) -- annoying, but leave the function defined


-- print the current call-in map
if (false) then
  for name, state in pairs(BZ.GetCallIns()) do
    print(name, state)
  end
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

BZ.UpdateCallIn('AllowFlagGrabEvent',
  function(playerID, flagID, flagType, shotType, px, py, pz)
    if (BZ.GetPlayerTeam(playerID) == BZ.TEAM.RED) then
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


local lua_block_env = {}
setmetatable(lua_block_env, { __index = _G })

local function CustomMapObject(name, data)
  print('CustomMapObject:  ' .. name)
  for d = 1, #data do
    print('CustomMapObject:    ' .. data[d])
  end

  if (name == 'lua_block') then
    local text = ''
    for d = 1, #data do
      text = text .. data[d] .. '\n'
    end
    local chunk, err = loadstring(text, 'lua_block')
    if (not chunk) then
      print(err)
    else
      setfenv(chunk, lua_block_env)
      local success, mapText = pcall(chunk)
      if (not success) then
        print(err)
      else
        if (type(mapText) == 'string') then
          print('MAPTEXT: ' .. tostring(mapText))
        elseif (type(mapText) == 'table') then
          print('MAPTEXT: ' .. table.concat(mapText, '\n'))
        end
        return mapText
      end
    end
  end
end


BZ.AttachMapObject('custom_block',  CustomMapObject)
BZ.AttachMapObject('custom_block1', CustomMapObject)
BZ.AttachMapObject('custom_block2', CustomMapObject)
BZ.AttachMapObject('custom_block3', CustomMapObject)
BZ.AttachMapObject('lua_plugin',    CustomMapObject)
BZ.AttachMapObject('lua_block',     CustomMapObject)


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

if (false) then
  for _, name in pairs(BZ.DB.GetList()) do
    print(name, 
          BZ.DB.GetInt(name),
          BZ.DB.GetBool(name),
          BZ.DB.GetFloat(name),
          BZ.DB.GetString(name))
  end
end

BZ.DB.SetString('_mirror', 'black 0.5')
BZ.DB.SetString('_skyColor', 'red')
BZ.DB.SetFloat('_tankSpeed', '50.0')



BZ.AttachSlashCommand('luabzfs', 'bzfs lua plugin command',
function(playerID, cmd, msg)
  print('luabzfs command received: '..playerID..' '..cmd..' '..msg)
end)


include('plugins.lua')


--BZ.UpdateCallIn('GetWorldEvent',
--  function(mode)
