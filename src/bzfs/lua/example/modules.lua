--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  file:    plugins.lua
--  author:  Dave Rodgers  (aka: trepan)
--  date:    Dec 05, 2008
--  desc:    bzfsAPI lua plugin manager
--  license: LGPL 2.1
--
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local MODULES_DIR = bz.GetLuaDirectory() .. 'plugins/'

print()
print('-----------')
print('LUA PLUGINS')
print('-----------')


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  ci:  callin
--  mo:  module
--
--  ciData = {
--    name    = string,
--    func    = function,
--    enabled = bool,
--    moFunc  = { func1,   func2,   ... },
--    moData  = { moData1, moData2, ... },
--  }
--
--  moData = {
--    name     = string,
--    source = {
--      type = 'file' or 'world' or 'code',
--      data = 'filename' or function chunk(),
--    }
--    niceness = number,
--    fenv     = { fenv },
--    funcs    = { ciName1 = func1, ciName2 = func2, etc... },
--    unsafe   = bool,
--    enabled  = bool,
--    info     = {
--      filename = string,
--      desc     = string,
--      author   = string,
--      date     = string,
--      license  = string,
--    }
--  }


local modules = {}  --  < moName = moData >  pairs

local callins = {}  --  < ciName = ciData >  pairs


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local function ListModules()
end


local function FindModules()
  local files, dirs = bz.DirList(MODULES_DIR)
  local sources = {}
  if (files) then
    for _, f in ipairs(files) do
      if (f:find('^[^.].*%.lua$')) then
        sources[#sources + 1] = f
      end
    end
  end
  return sources
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local function SetupModule(fileName)
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

do
  for _, f in ipairs(FindModules()) do
    SetupModule(f)
  end
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local function SetCallIn(moName, ciName, ciFunc)
  local moData = modules[moName]
  if (moData == nil) then
    return nil
  end
  
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local function CreateModule(sourceType, sourceData)
  if (sourceType == 'file') then
  elseif ((sourceType == 'code') or
          (sourceType == 'world')) then
  end
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local function EnableModule(name)
  local moData = modules[moName]
  if (moData == nil) then
    return nil
  end
  if (moData.enabled) then
    return false
  end
  return true
end


local function DisableModule(name)
  local moData = modules[moName]
  if (moData == nil) then
    return nil
  end
  if (not moData.enabled) then
    return false
  end
  return true
end


local function ToggleModule(name)
  local moData = modules[moName]
  if (moData == nil) then
    return nil
  end
  if (moData.enabled) then
    return DisableModule(name)
  else
    return EnableModule(name)
  end
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

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


local function GenUnknownSlashCmdFunc(funcList)
  return function(msg, src, dst, team)
    if (msg:find('/lph')) then  -- Lua Plugin Handler
      bz.Print('Lua Plugin Handler')      
      return true
    else
      for i = 1, #funcList do
        if (funcList[i](msg, src, dst, team) == true) then
          return true
        end
      end
    end
  end
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local specialCallIns = {
  UnknownSlashCommand = GenUnknownSlashCmdFunc, --> [bool handled]
  RawChatMessage      = GenFirstStringFunc, --> [string newMsg]
  PlayerCollision     = GenFirstTrueFunc,   --> [bool handled]
  AnointRabbit        = GenFirstNumberFunc, --> [number playerID]
  FlagTransferred     = GenFirstNumberFunc, --> [number action]
  GetAutoTeam         = GenFirstNumberFunc, --> [number team]
  GetPlayerSpawnPos   = GenFirstValidFunc,  --> [px, py, pz][, rot]
  GetPlayerInfo       = GenFirstValidFunc,  --> [bool adm][,bool verified][,bool reg'd]
  AllowSpawn          = GenFirstFalseFunc,  --> [bool allow]
  AllowFlagGrab       = GenFirstFalseFunc,  --> [bool allow]
  AllowKillCommand    = GenFirstFalseFunc,  --> [bool allow]
  AllowPlayer         = GenFirstFalseFunc2, --> [bool allow][, string reason]
  AllowCTFCapture     = GenFirstFalseFunc2, --> [bool allow][, bool killTeam]
}

local ciFuncs = {}
local ciFuncLists = {}
local permanentCallIns = {
  UnknownSlashCommand = true,
}

local function SetupCallIns()
  for ciName in pairs(Script.GetCallInInfo()) do
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

-- setup the global variable
moduleHandler = {
  ListModules   = ListModules,
  CreateModule  = CreateModule,
  EnableModule  = EnableModule,
  DisableModule = DisableModule,
  RaiseModule   = RaiseModule,
  LowerModule   = LowerModule,
}

return moduleHandler

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
