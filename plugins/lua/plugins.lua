--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  file:    plugins.lua
--  author:  Dave Rodgers  (aka: trepan)
--  date:    Nov 22, 2008
--  desc:    bzfsAPI lua plugin manager
--  license: LGPL 2.1
--
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local PLUGINS_DIR = PLUGINS_DIR or './plugins/'

print()
print('-----------')
print('LUA PLUGINS')
print('-----------')


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
-- ci:  callin
-- pi:  plugin
--

local piNames = {}  --  < piName = { name = '',
local ciFuncs = {}  --  < ciName = { ciFunc, ciFunc, ... } >  pairs


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  Wrap lfs.dir() to capture errors
--
local function safe_dir(dirPath)
  local success, func, state, init = pcall(lfs.dir, dirPath)
  if (success) then
    return func, state, init
  else
    print('WARNING: ' .. func)
    return function() end, nil, nil
  end
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local function SetupPlugin(fileName)
  print('loading plugin: ' .. fileName)
  local chunk, err = loadfile(fileName)
  if (not chunk) then
    print('  error: ' .. err)
    return
  end
--[[
  local plugin = {}
  plugins[plugin] = {}
  plugins[plugin] = 
  piChunks[plugin] = chunk
  setfenv(chunk, plugin)
--]]
end

for k, v in safe_dir(PLUGINS_DIR) do
  if (k:find('%.lua$') and not k:find('^%.')) then
    SetupPlugin(PLUGINS_DIR .. k)
  end
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local function UpdateCallIn(plugin, ciName, ciFunc)
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local function EnablePlugin(pluginName)
end


local function DisablePlugin(pluginName)
end


local function TogglePlugin(pluginName)
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local function GenSlashCommandFunc(funcList)
  return function(...)
    for i = 1, #funcList do
      funcList[i](...)
    end
  end
end


local function GenAllFunc(funcList)
  return function(...)
    for i = 1, #funcList do
      funcList[i](...)
    end
  end
end


local function GenFirstFalseFunc(funcList)
  return function(...)
    for i = 1, #funcList do
      if (funcList[i](...) == false) then
        return false
      end
    end
  end
end


local function GenFirstFalseFunc2(funcList)
  return function(...)
    for i = 1, #funcList do
      local value, p2 = funcList[i](...)
      if (value == false) then
        return false, p2
      end
    end
  end
end


local function GenFirstTrueFunc(funcList)
  return function(...)
    for i = 1, #funcList do
      local value = funcList[i](...)
      if (value == true) then
        return value
      end
    end
  end
end


local function GenFirstStringFunc(funcList)
  return function(...)
    for i = 1, #funcList do
      local value = funcList[i](...)
      if (type(value) == 'string') then
        return value
      end
    end
  end
end


local function GenFirstNumberFunc(funcList)
  return function(...)
    for i = 1, #funcList do
      local value = funcList[i](...)
      if (type(value) == 'number') then
        return value
      end
    end
  end
end


local function GenFirstValidFunc(funcList)
  return function(...)
    for i = 1, #funcList do
      local p1, p2, p3, p4 = funcList[i](...)
      if (p1) then
        return p1, p2, p3, p4
      end
    end
  end
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local specialCallIns = {
  RawChatMessageEvent    = GenFirstStringFunc, --> [string newMsg]
  UnknownSlashCommand    = GenFirstTrueFunc,   --> [bool handled]
  PlayerCollision        = GenFirstTrueFunc,   --> [bool handled]
  AnointRabbitEvent      = GenFirstNumberFunc, --> [number playerID]
  FlagTransferredEvent   = GenFirstNumberFunc, --> [number action]
  GetAutoTeamEvent       = GenFirstNumberFunc, --> [number team]
  GetPlayerSpawnPosEvent = GenFirstValidFunc,  --> [px, py, pz] [, rot]
  GetPlayerInfoEvent     = GenFirstValidFunc,  --> [bool adm][,bool verified][,bool reg'd]
  AllowPlayer            = GenFirstStringFunc, --> [string reason]
  AllowSpawn             = GenFirstFalseFunc,  --> [bool allow]
  AllowFlagGrabEvent     = GenFirstFalseFunc,  --> [bool allow]
  AllowKillCommandEvent  = GenFirstFalseFunc,  --> [bool allow]
  AllowCTFCaptureEvent   = GenFirstFalseFunc2, --> [bool allow][, bool killTeam]
}

local ciFuncs = {}
local ciFuncLists = {}

local function SetupCallIns()
  for ciName in pairs(BZ.GetCallInList()) do
    local genFunc = specialCallIns[ciName] or GenAllFunc
    local funcList = {}
    ciFuncLists[ciName] = funcList
    local ciFunc = genFunc(funcList)
    ciFuncs[ciName] = ciFunc
  end
end


SetupCallIns()

print()

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
