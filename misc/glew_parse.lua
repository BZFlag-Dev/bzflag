#!/usr/bin/env lua
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  file:     parse_glew.lua
--  author:   Dave Rodgers (aka: trepan)
--  date:     Jan 15, 2009
--  license:  LGPL v2.1
--  desc:     parses glew.h to generate glew.lua
--
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

if (({['-h']=true,['-help']=true,['--help']=true})[arg[1]] ~= nil) then
  print()
  print('usage:  ['..arg[-1]..'] '..arg[0]..' '..'[path to glew.h]')
  print()
  os.exit(1)
end

-- use the packaged glew.h by default
if (arg[1] == nil) then
  arg[1] = '../src/other/glew/include/GL/glew.h'
  io.stderr:write('using:  '..arg[1]..'\n')
end

local input, err = io.open(arg[1], 'rt')
if (not input) then
  print(err)
  os.exit(1)
end

--[[
local output, err = io.open('glew.lua', 'wt')
if (not output) then
  print(err)
  os.exit(1)
end
--]]
local output = io.stdout

local GL = {}
local headers = true

output:write('--[[\n\n')
for line in input:lines() do
  if (headers and not line:match('^#')) then
    output:write(line..'\n')
  else
    if (headers) then
      output:write('--]]\n')
      headers = false
    end
    -- this excludes the extension presence definitions
    local k, v = line:match('^#define%s*GL_([A-Z0-9_]*)%s*(%S*)\r?$')
    if (k and v) then
      GL[k] = tonumber(v)
    end
  end
end

input:close()

--------------------------------------------------------------------------------

local count = 0
local sorted = {}
local maxLen = 0
for k, v in pairs(GL) do
  count = count + 1
  sorted[count] = { k, v }
  if (#k > maxLen) then
    maxLen = #k
  end
end
table.sort(sorted, function(a, b) return a[1] < b[1] end)

maxLen = maxLen + 4

--------------------------------------------------------------------------------

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
    name = '["'..key..'"]'
  else
    name = '.'..key
  end
  local space = (' '):rep(maxLen - #name)
  local space = '' -- do not align
  output:write(('GL%s%s = 0x%04X\n'):format(name, space, value))
end
output:write('\n')
output:write('return GL\n')

output:close()

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
