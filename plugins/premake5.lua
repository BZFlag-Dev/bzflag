group "plugins"

-- set up the plugin_utils project
include "plugin_utils"

pluginDirNames = os.matchdirs("*")

-- set up the individual plugin projects
for _, pluginName in ipairs(pluginDirNames) do
  if pluginName ~= "plugin_utils" then
    project(pluginName)
      kind "SharedLib"
      targetprefix ""
      files { pluginName.."/*.cpp", pluginName.."/*.h" }
      includedirs "plugin_utils"
      links "plugin_utils"

      filter "system:windows"
	defines {
	  "WIN32",
	  "_USE_MATH_DEFINES",
	  "_WINDOWS",
	  "_USRDLL",
	  pluginName.."_EXPORTS"
        }
	libdirs "$(OutDir)"
	links "bzfs.lib" -- ".lib" required to distinguish from the executable
	dependson "bzfs"
	postbuildcommands {
	  "if not exist ..\\bin_$(Configuration)_$(Platform) mkdir ..\\bin_$(Configuration)_$(Platform)",
	  "if not exist ..\\bin_$(Configuration)_$(Platform)\\plugins mkdir ..\\bin_$(Configuration)_$(Platform)\\plugins",
	  "copy $(OutDir)"..pluginName..".dll ..\\bin_$(Configuration)_$(Platform)\\plugins\\",
	  "copy ..\\plugins\\"..pluginName.."\\*.txt ..\\bin_$(Configuration)_$(Platform)\\plugins\\",
	  "if exist ..\\plugins\\"..pluginName.."\\*.cfg ( copy ..\\plugins\\"..pluginName.."\\*.cfg ..\\bin_$(configuration)_$(Platform)\\plugins\\ )"
	}

      filter "system:macosx"
	linkoptions "-undefined dynamic_lookup"

      filter "system:linux"
	symbols "On"
      filter { }
  end
end

group ""
