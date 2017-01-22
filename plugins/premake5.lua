group "plugins"

pluginNames = os.matchdirs("*")

-- set up the plugin_utils project
include "plugin_utils"

-- set up the individual plugin projects
for _, pluginName in ipairs(pluginNames) do
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
      libdirs "../build/bin/$(Configuration)" -- FIXME: any better way? maybe $(OutDir)?
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
    filter { }
    if _OS == "macosx" and not _OPTIONS["disable-client"] then
      project "bzflag" -- the .app needs to bundle the plugins with it
	dependson(pluginName)
    end
end

-- set up a post-build phase to copy the plugins into the application on macOS
if _OS == "macosx" and not _OPTIONS["disable-client"] then
  project "bzflag"
    postbuildcommands { "mkdir -p ${TARGET_BUILD_DIR}/${PLUGINS_FOLDER_PATH}" }
    for _, pluginName in ipairs(pluginNames) do
      postbuildcommands {
        "cp ${CONFIGURATION_BUILD_DIR}/"..pluginName..".dylib ${TARGET_BUILD_DIR}/${PLUGINS_FOLDER_PATH}/"..pluginName..".dylib",
        "cp ../plugins/*/*.txt ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
        "cp ../plugins/*/*.cfg ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
        "cp ../plugins/*/*.bzw ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/"
      }
    end
end

group ""
