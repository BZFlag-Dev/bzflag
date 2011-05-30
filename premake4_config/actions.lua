
tools = {}

function tools.svn_add_files()
  local status = os.outputof('svn status')
  for line in status:gmatch('[^\n]*') do
    local path = line:match('^%?%s+(.*)$')
    if (path) then
      -- plugins/python/.objs: application/x-directory; charset=binary
      local mimetype = os.outputof(('file --mime %q'):format(path))
      print('new file: ' .. (mimetype:gsub('\n', '')))
    end
  end
end

do
  local success = pcall(require, 'svn')
  if (not success) then
    function tools.svn_add_files()
      print('luasvn not available')
    end
  else
    function tools.svn_add_files()
      for k, v in pairs(svn.status()) do
        printf('%q %q', k, v)
      end
    end
  end
end
