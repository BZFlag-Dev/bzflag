--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local type = type

function isnil(x)      return type(x) == 'nil'      end
function isstring(x)   return type(x) == 'string'   end
function isboolean(x)  return type(x) == 'boolean'  end
function isnumber(x)   return type(x) == 'number'   end
function isuserdata(x) return type(x) == 'userdata' end
function istable(x)    return type(x) == 'table'    end
function isthread(x)   return type(x) == 'thread'   end
function isfunction(x) return type(x) == 'function' end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

function os.outputof(cmd)
  local p, err = io.popen(cmd, 'r')
  if (not p) then
    return nil, err
  end
  local t = p:read('*a')
  p:close()
  return t
end

--------------------------------------------------------------------------------

function os.testcode(t)
  if (isstring(t)) then
    t = { code = t }
  end

  assert(isstring(t.code), 'missing  parameter')

  local source = ''

  local includes = t.includes or ''
  local code = t.code
  code = code:match('\n$') and code or (code .. '\n')

  source = source .. includes
  source = source .. 'int main(int argc, char** argv) {\n'
  source = source .. code
  source = source .. '}\n'

  local f, err = io.open('lasttest.cpp', 'w')
  if (not f) then
    print(err)
    return false
  end
  f:write(source)
  f:close()


  local result = os.execute(premake.gcc.cxx
                            .. ' ' .. 'lasttest.cpp'
                            .. ' ' .. '-o junk'
                            .. ' ' .. (t.incflags or '')
                            .. ' ' .. (t.linkflags or ''))
  os.remove('junk')
  os.remove('lasttest.cpp')
  return result
end

--------------------------------------------------------------------------------

function os.testtypesize(typeName)
  assert(isstring(typeName), 'missing code parameter')

end

--------------------------------------------------------------------------------

if (-1 > 0) then
  for _,code in ipairs {
    'int b = 1;',
    'void v = 12;',
  } do
    print('code = ' .. code)
    print('result = ' .. os.testcode(code))
  end
end

--------------------------------------------------------------------------------
