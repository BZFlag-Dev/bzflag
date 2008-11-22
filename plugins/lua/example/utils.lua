--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  file:    utils.lua
--  author:  Dave Rodgers  (aka: trepan)
--  date:    Nov 22, 2008
--  desc:    utility routines
--  license: LGPL 2.1
--
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

function printf(fmt, ...)
  print(fmt:format(...))
end


function epcall(func, errFunc, ...)
  return xpcall(function(...) return func(...) end, errFunc)
end


function tracepcall(func, ...)
  local traceback = debug.traceback
  return xpcall(function(...) return func(...) end, traceback)
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

function include(fileName, fenv)
  local fn = fileName
  local file, err = io.open(fn, 'r')
  if (not file) then
    fn = BZ.GetLuaDirectory() .. fileName
    file, err = io.open(fn, 'r')
  end
  if (not file) then
    error('include: ' .. err)
  end
  file:close()

  local chunk, err = loadfile(fn)
  if (not chunk) then
    error(err)
  end
  setfenv(chunk, fenv or getfenv(1))

  local retval = { tracepcall(chunk) }
  if (retval[1] ~= true) then
    error(retval[2])
  end

  return unpack(retval, 2)
end
  

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  string additions
--

function string.addslash(s)
  if (s:find('/$')) then
    return
  end
  return s .. '/'
end


function string.tokenize(s)
  local words = {}
  for w in s:gmatch('%S+') do
    words[#words + 1] = w
  end
  return words
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  table additions
--

local printFunc = print

local printed = {}


local function PrintTable(t, indent)
  if (printed[t]) then
    printFunc(indent .. '-- ALREADY PRINTED')
    return
  else
    printed[t] = true
  end

  for k,v in pairs(t) do
    if (type(v) ~= 'table') then
      printFunc(indent .. tostring(k) .. ' = ' .. tostring(v) .. ',')
    else
      printFunc(indent .. tostring(k) .. ' = {')
      PrintTable(v, indent .. '  ')
      printFunc(indent .. '},')
    end
  end
end


function table.print(t, func)
  printed = {}
  printFunc = func or print
  PrintTable(t, '')
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
