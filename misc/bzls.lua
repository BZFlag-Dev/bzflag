#!/usr/bin/env lua

--------------------------------------------------------------------------------
--
--  author:     trepan
--  date:       June 03, 2011
--  copyright:  (c) 2011 trepan
--  license:    LGPL 2.1
--
--  brief:      BZFlag server list console utililty
--
--------------------------------------------------------------------------------

if (_VERSION ~= 'Lua 5.1') then
  print('ERROR: this program requires Lua 5.1')
  os.exit(1)
end

--------------------------------------------------------------------------------

local https = require('ssl.https')

print(os.date())


--------------------------------------------------------------------------------

local urlBase = 'https://my.bzflag.org/db/?action=LIST&listformat=lua'

local protocol = 'BZFS0221'

local gitProtoURL =
  'https://raw.githubusercontent.com/BZFlag-Dev/bzflag/master/src/date/buildDate.cxx'

--------------------------------------------------------------------------------

local ansicodes

do
  local esc = '\027'
  ansicodes = {
    reset      = esc .. '[0m',
    bright     = esc .. '[1m',
    underline  = esc .. '[4m',
    reverse    = esc .. '[7m',
    fg_black   = esc .. '[30m',
    fg_red     = esc .. '[31m',
    fg_green   = esc .. '[32m',
    fg_yellow  = esc .. '[33m',
    fg_blue    = esc .. '[34m',
    fg_purple  = esc .. '[35m',
    fg_cyan    = esc .. '[36m',
    fg_white   = esc .. '[37m',
    fg_default = esc .. '[39m',
  }
end


--------------------------------------------------------------------------------

local protoAll = true
local gitProto = false

local passfile = tostring(os.getenv('HOME')) .. '/.bzf/passfile'
local callsign = nil
local password = nil
local callpass = ''

local limitCount = nil
local minPlayers = nil

local teamCounts = false

local showOwners = false

local gameInfo = false

local debugging = false

--------------------------------------------------------------------------------

local function PrintHelp()
  print('usage: checkbz [options]')
  print(' -h:      print this help')
  print(' -c:      check current protocol')
  print(' -g:      check git master protocol')
  print(' -i:      list using identity / password')
  print(' -l <#>:  maximum number of servers')
  print(' -p <#>:  minimum number of players')
  print(' -t:      team counts')
  print(' -o:      show owners')
--FIXME  print(' -f:      game info')
  print(' -d:      enable debugging')
end


--------------------------------------------------------------------------------

do
  while (arg[1]) do
    local a = arg[1]
    if (a == '-h') then
      PrintHelp()
      os.exit(0)
    elseif (a == '-p') then
      minPlayers = tonumber(arg[2])
      if (not minPlayers) then
        error('bad -p argument')
      end
      table.remove(arg, 1)
    elseif (a == '-l') then
      limitCount = tonumber(arg[2])
      if (not limitCount) then
        error('bad -l argument')
      end
      table.remove(arg, 1)
    elseif (a == '-c') then
      protoAll = false
      gitProto = false
    elseif (a == '-g' or a == '-s') then
      -- "-s" is recognized to keep backwards compatibility with bzls.lua
      -- scripts that received the protocol version from SVN
      protoAll = false
      gitProto = true
    elseif (a == '-o') then
      showOwners = true
    elseif (a == '-t') then
      teamCounts = true
    elseif (a == '-d') then
      debugging = true
    elseif (a == '-i') then
      local cpchunk = loadfile(passfile)
      if (cpchunk) then
        callsign, password = cpchunk()
      end
    elseif (not arg[1]:match('^%-')) then
      break
    else
      PrintHelp()
      os.exit(1)
    end
    table.remove(arg, 1)
  end
end


if (not minPlayers) then
  minPlayers = limitCount and 0 or 1
end


--------------------------------------------------------------------------------

local function UrlEncode(s)
  return (s:gsub('[%s%W]', function(c)
    if (c:match('%s')) then
      return '+'
    else
      return '%' .. string.format('%-2.2X', c:byte())
    end
  end))
end


if (callsign and password) then
  callpass = '&callsign=' .. UrlEncode(callsign) ..
             '&password=' .. UrlEncode(password)
end


--------------------------------------------------------------------------------

local function ParseProtocol(protoCode)
  for line in protoCode:gmatch('[^\n]+') do
    local proto = line:match('#%s*define%s*BZ_PROTO_VERSION%s*"(.-)"')
    if (proto) then
      return 'BZFS' .. proto
    end
  end
  error('could not find the protocol using:\n  ' .. gitProtoURL)
end


if (gitProto) then
  local protoCode = https.request(gitProtoURL)
  protocol = ParseProtocol(protoCode)
end


--------------------------------------------------------------------------------

local serverStr
do
  local protoQuery = ''
  if (not protoAll) then
    protoQuery = '&version=' .. protocol
  end
  local queryURL = urlBase .. protoQuery .. callpass
  if (debugging) then
    print('queryURL = ' .. queryURL)
  end
  serverStr, err = https.request(queryURL)
  if (not serverStr) then
    print('query failed: ' .. tostring(err))
    print('(' .. queryURL .. ')')
    os.exit(1)
  end
  if (debugging) then
    print(serverStr)
  end
end


local function ParseGameInfo(hexstr)
  local packedString = hexstr
  local packedIndex = 1

  if ((#hexstr ~= 54) and   -- length for 2.0.x
      (#hexstr ~= 58)) then -- length for 3.0.x
    return nil -- unknown protocol
  end

  local function UnpackU8()
    local a = packedString:sub(packedIndex, packedIndex + 2 - 1)
    packedIndex = packedIndex + 2
    return tonumber(a, 16)
  end
  local function UnpackU16()
    local a = packedString:sub(packedIndex, packedIndex + 4 - 1)
    packedIndex = packedIndex + 4
    return tonumber(a, 16)
  end

  local t = {}

  if (#hexstr == 58) then -- protocol for 3.0
    t.gameType     = UnpackU16()
  end
  t.gameOptions    = UnpackU16()
  t.maxShots       = UnpackU16()
  t.shakeWins      = UnpackU16()
  t.shakeTimeout   = UnpackU16() * 0.1
  t.maxPlayerScore = UnpackU16()
  t.maxTeamScore   = UnpackU16()
  t.maxTime        = UnpackU16()

  t.maxPlayers     = UnpackU8()
  t.rogueCount     = UnpackU8()
  t.rogueMax       = UnpackU8()
  t.redCount       = UnpackU8()
  t.redMax         = UnpackU8()
  t.greenCount     = UnpackU8()
  t.greenMax       = UnpackU8()
  t.blueCount      = UnpackU8()
  t.blueMax        = UnpackU8()
  t.purpleCount    = UnpackU8()
  t.purpleMax      = UnpackU8()
  t.observerCount  = UnpackU8()
  t.observerMax    = UnpackU8()

--  t.gameType = ParseGameType(t.gameType)

--  t.gameOptions = ParseGameOptions(t.gameOptions)

  return t
end


local playerTeams = {
  rogueCount  = true,
  redCount    = true,
  greenCount  = true,
  blueCount   = true,
  purpleCount = true,
}


local function ParseServerLine(server)
  if (not server.version or
      not server.hexcode or
      not server.addr    or
      not server.ipaddr  or
      not server.title) then
    error('invalid server entry')
  end
  local gt = ParseGameInfo(server.hexcode)
  if (not gt) then
    error('invalid gameinfo hex string')
  end
  local count = 0
  for k, v in pairs(gt) do
    if (playerTeams[k]) then
      count = count + v
    end
  end
  return {
    ipaddr        = server.ipaddr,
    address       = server.addr,
    protocol      = server.version,
    gameinfo      = server.hexcode,
    description   = server.title,
    owner         = server.owner,
    playerCount   = count,
    rogueCount    = gt.rogueCount,
    redCount      = gt.redCount,
    greenCount    = gt.greenCount,
    blueCount     = gt.blueCount,
    purpleCount   = gt.purpleCount,
    observerCount = gt.observerCount,
  }
end


--------------------------------------------------------------------------------

local servers = {}

local serverCount   = 0
local playerCount   = 0
local observerCount = 0


local function MapServers(t)
  local fields = t.fields
  for s = 1, #t.servers do
    local server = {}
    local serverData = t.servers[s]
    for f = 1, #serverData do
      local fieldName = fields[f]
      if (fieldName) then
        server[fieldName] = serverData[f]
      end
    end
    servers[#servers + 1] = server
  end
end


local function GetSafeEnv()
  return {
    type      = type,
    next      = next,
    pairs     = pairs,
    ipairs    = ipairs,
    select    = select,
    unpack    = unpack,
    tonumber  = tonumber,
    tostring  = tostring,
    assert    = assert,
    print     = print,
    math      = math,
    string    = string,
    table     = table,
    coroutine = coroutine,
  }
end


do
  local serverChunk = assert(loadstring(serverStr))
  setfenv(serverChunk, GetSafeEnv())
  local _, serverTable = assert(pcall(serverChunk))

  MapServers(serverTable)

  local inServers = servers
  servers = {}

  for _, server in ipairs(inServers) do
    local server = ParseServerLine(server)
    if (server) then
      serverCount   = serverCount + 1
      playerCount   = playerCount   + server.playerCount
      observerCount = observerCount + server.observerCount
      servers[#servers + 1] = server
    end
  end

  table.sort(servers, function(a, b)
    if (a.protocol ~= b.protocol) then
      return a.protocol < b.protocol
    end
    local va = a.playerCount + (a.observerCount * 0.001)
    local vb = b.playerCount + (b.observerCount * 0.001)
    if (va ~= vb) then
      return va > vb
    end
    return a.address < b.address
  end)

  if (limitCount) then
    for i = (limitCount + 1), #servers do
      servers[i] = nil
    end
  end
end


--------------------------------------------------------------------------------

local function GetTermSize()
  local p = io.popen('stty size', 'r') -- "size" is not a POSIX argument
  if (not p) then
    return nil
  end
  local s = p:read('*a')
  p:close()
  if (not s) then
    return nil
  end
  local rows, cols = s:match('(%d+) (%d+)')
  rows, cols = tonumber(rows), tonumber(cols)
  if (rows and cols) then
    return rows, cols
  end
  return nil
end


--------------------------------------------------------------------------------

local function LimitLength(s, n)
  if (#s > n) then
    assert(n >= 3)
    s = s:sub(1, n - 3) .. '...'
  end
  return s
end


do
  local maxLineLen
  do
    local rows, cols = GetTermSize()
    maxLineLen = cols or 80
  end

  local descLen = 0
  for _, s in ipairs(servers) do
    if (descLen < #s.description) then
      descLen = #s.description
    end
  end

  local addrLen = 0
  for _, s in ipairs(servers) do
    if (addrLen < #s.address) then
      addrLen = #s.address
    end
  end
  local maxAddrLen = addrLen

  local function fmtcount(count)
    return (count == 0) and ' _' or ('%2i'):format(count)
  end

  local tmpServers = {}
  for _, s in ipairs(servers) do
    local pc = s.playerCount + (s.observerCount * 0.001)
    if (pc >= minPlayers) then
      tmpServers[#tmpServers + 1] = s
    end
  end
  servers = tmpServers

  local protoCurr = ''

  for _, s in ipairs(servers) do
     local ac = ansicodes

     if (s.protocol ~= protoCurr) then
       print()
       print(ac.bright .. ac.fg_cyan .. ac.underline .. s.protocol .. ac.reset)
       protoCurr = s.protocol
     end

     local prefix = ''
     if (not teamCounts) then
       prefix = ('[%s%s%s,%s%s%s]')
                :format(ac.fg_yellow, fmtcount(s.playerCount),   ac.reset,
                        ac.fg_cyan,   fmtcount(s.observerCount), ac.reset)
     else
       prefix = ('[%s%s%s,%s%s%s,%s%s%s,%s%s%s,%s%s%s,%s%s%s,%s%s%s]')
                :format(ac.bright,    fmtcount(s.playerCount),   ac.reset,
                        ac.fg_yellow, fmtcount(s.rogueCount),    ac.reset,
                        ac.fg_red,    fmtcount(s.redCount),      ac.reset,
                        ac.fg_green,  fmtcount(s.greenCount),    ac.reset,
                        ac.fg_blue,   fmtcount(s.blueCount),     ac.reset,
                        ac.fg_purple, fmtcount(s.purpleCount),   ac.reset,
                        ac.fg_cyan,   fmtcount(s.observerCount), ac.reset)
     end
     local stripped = prefix:gsub('\027%[[%d;]+%l', '')
     local maxDescLen = maxLineLen - maxAddrLen - (#stripped + 2 + 2)
     if (maxDescLen > descLen) then
       maxDescLen = descLen
     end
     local desc = LimitLength(s.description, maxDescLen)
     local addr = LimitLength(s.address,     maxAddrLen)
     print(('%s  %s%-'..maxDescLen..'s%s  %s%s%s')
           :format(prefix, ac.fg_green, desc, ac.reset,
                           ac.fg_red,   addr, ac.reset))
     if (showOwners and s.owner and (#s.owner > 0)) then
       print('\\__ ' .. s.owner)
     end
  end

  print()
  print('servers   = ' .. serverCount)
  print('observers = ' .. observerCount)
  print('players   = ' .. playerCount)
end


--------------------------------------------------------------------------------
