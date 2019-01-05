-- set up the plugin_utils project
include "plugin_utils"

group "plugins"

pluginDirNames = os.matchdirs("*")

for _, pluginName in ipairs(pluginDirNames) do
  if pluginName ~= "plugin_utils" then
    include(pluginName)
  end
end

group ""
