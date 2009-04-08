#!/usr/bin/env lua
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  file:     glew_parse.lua
--  author:   Dave Rodgers (aka: trepan)
--  date:     Jan 15, 2009
--  license:  LGPL 2.1
--  desc:     generates glew.lua from glew.h and the constants
--            being used by the source files in the current directory
--
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local wantAll = false

local wantSpaces = false

local sortNumeric = false

local scanDir = '.'

local glew_h = '/usr/include/GL/glew.h'

local outputName

local desired = {}


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  parse the command line arguments
--

local function PrintHelp(message)
  if (type(message) == 'string') then
    print()
    print(message)
    print()
  end
  print('usage:  ' .. arg[-1] .. ' ' .. arg[0] .. ' [options]')
  print('  -h         : this help')
  print('  -g <path>  : the path to glew.h')
  print('  -o <path>  : the output filename')
  print('  -d <path>  : the scan directory')
  print('  -a         : extract all header definitions')
  print('  -s         : align the output text using spaces')
  print('  -n         : numeric sort')
end


local i = 1

while (i <= #arg) do
  local a = arg[i]
  if ((a == '-h') or (a == '-help') or (a == '--help')) then
    PrintHelp()
    os.exit(1)
  elseif (a == '-g') then
    i = i + 1
    if (arg[i]) then
      glew_h = arg[i]
    else
      print('missing ' .. a .. ' parameter')
      os.exit(1)
    end
  elseif (a == '-o') then
    i = i + 1
    if (arg[i]) then
      outputName = arg[i]
    else
      print('missing ' .. a .. ' parameter')
      os.exit(1)
    end
  elseif (a == '-d') then
    i = i + 1
    if (arg[i]) then
      scanDir = arg[i]
      if (not scanDir:match('/$')) then
        scanDir = scanDir .. '/'
      end
    else
      print('missing ' .. a .. ' parameter')
      os.exit(1)
    end
  elseif (a == '-a') then
    wantAll = true
  elseif (a == '-s') then
    wantSpaces = true
  elseif (a == '-n') then
    sortNumeric = true
  else
    PrintHelp('unknown option: "' .. a .. '"')
    os.exit(1)
  end
  
  i = i + 1
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  find the GL.xxx tags in the code, with recursion
--

if (not wantAll) then

  require('lfs')

  local function DesiredFile(file)
    local f = io.open(file, 'r')
    if (not f) then
      return
    end
    for line in f:lines() do
      for enum in line:gmatch('[^%w]GL%.([%w_]+)') do
        desired[enum] = true
      end
    end
  end

  local function lfs_safedir(dir)
    local success, func, state, var = pcall(lfs.dir, dir)
    if (not success) then
      return function() end
    end
    return func, state, var
  end

  local function DesiredDir(dir)
    for file in lfs_safedir(dir) do
      if (not file:match('^%.')) then
        local fullPath = dir .. file
        local mode = lfs.attributes(fullPath, 'mode')
        if (mode == 'directory') then
          DesiredDir(fullPath .. '/')
        elseif( mode == 'file') then
          local lower = file:lower()
          if (lower:match('.+%.lua$') and (lower ~= 'glew.lua')) then
            DesiredFile(fullPath)
          end
        end
      end
    end
  end

  if (not scanDir:match('/$')) then
    scanDir = scanDir .. '/'
  end

  DesiredDir(scanDir)
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  open the header file and the output file
--

local input, err = io.open(glew_h, 'rt')
if (not input) then
  PrintHelp(err)
  os.exit(1)
end


local output = io.stdout
if (type(outputName) == 'string') then
  local err
  output, err = io.open(outputName, 'wt')
  if (not output) then
    PrintHelp(err)
    os.exit(1)
  end
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  parse glew.h, and copy the header
--

local GL = {}
local headers = true

output:write('--[[\n\n')
for line in input:lines() do
  if (headers and not line:match('^#')) then
    output:write(line:gsub('\r','') .. '\n')
  else
    if (headers) then
      output:write('--]]\n')
      headers = false
    end
    -- this excludes the extension presence definitions
    local k, v = line:match('^#define%s+GL_([%u%d_]*)%s+(%S+)\r?$')
    if (k and v and (wantAll or desired[k])) then
      GL[k] = tonumber(v)
    end
  end
end

input:close()


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  sort by name, and find the longest name
--

local count = 0
local sorted = {}
local maxLen = 0
for k, v in pairs(GL) do
  count = count + 1
  sorted[count] = { k, v }
  local len = #k
  if (k:match('^[0-9]')) then
    len = len + 4 -- [""]
  end
  if (len > maxLen) then
    maxLen = len
  end
end
if (sortNumeric) then
  table.sort(sorted, function(a, b)
    return tonumber(a[2]) < tonumber(b[2])
  end)
else
  table.sort(sorted, function(a, b) return a[1] < b[1] end)
end

maxLen = maxLen + 1


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  write the definitions
--

output:write('\n')
output:write('--\n')
output:write('--  Generated by ' .. arg[0] .. '\n')
output:write('--\n')
output:write('--  date:    ' .. os.date('%Y-%m-%d/%H:%M:%S') .. '\n')
output:write('--\n')
output:write('--  header:  ' .. glew_h .. '\n')
output:write('--\n')
output:write('\n')
output:write('GL = GL or {}\n')
output:write('\n')
output:write('local GL = GL\n')
output:write('\n')
for i = 1, count do
  local def = sorted[i]
  local key   = def[1]
  local value = def[2]
  local space = ''
  if (key:match('^[0-9]')) then
    name = '["' .. key .. '"]'
  else
    name = '.' .. key
  end
  local space = ''
  if (wantSpaces) then
    space = (' '):rep(maxLen - #name)
  end
  output:write(('GL%s%s = 0x%04X\n'):format(name, space, value))
end
output:write('\n')
output:write('return GL\n')
output:write('\n')

output:close()


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
