-- enumerate the plugins to include (modify this list to add a plugin)
pluginnames = {
  "CustomZoneSample",
  "HoldTheFlag",
  "Phoenix",
  "RogueGenocide",
  "SAMPLE_PLUGIN",
  "TimeLimit",
  "airspawn",
  "autoFlagReset",
  "bzfscron",
  "chathistory",
  "customflagsample",
  "fairCTF",
  "fastmap",
  "flagStay",
  "keepaway",
  "killall",
  "koth",
  "logDetail",
  "nagware",
  "playHistoryTracker",
  "pushstats",
  "rabbitTimer",
  "rabidRabbit",
  "recordmatch",
  "regFlag",
  "serverControl",
  "serverSidePlayerSample",
  "shockwaveDeath",
  "superUser",
  "teamflagreset",
  "thiefControl",
  "timedctf",
  "wwzones"
}

-- set up the plugin_utils project
include "plugin_utils"

-- set up the individual plugin projects from the list above
for _, pluginname in ipairs(pluginnames) do
  project(pluginname)
    kind "SharedLib"
    files { pluginname.."/*.cpp", pluginname.."/*.h" }
    includedirs "plugin_utils"
    links "plugin_utils"

    filter "system:windows"
      defines {
	"WIN32",
	"_USE_MATH_DEFINES",
	"_WINDOWS",
	"_USRDLL",
	pluginname.."_EXPORTS"
      }
      files { pluginname.."/"..pluginname..".def" }
      libdirs "../build/bin/$(Configuration)" -- FIXME: any better way?
      links "bzfs.lib" -- ".lib" required to distinguish from the executable
	dependson "bzfs"
      postbuildcommands {
	"if not exist ..\\bin_$(Configuration)_$(Platform) mkdir ..\\bin_$(Configuration)_$(Platform)",
	"if not exist ..\\bin_$(Configuration)_$(Platform)\\plugins mkdir ..\\bin_$(Configuration)_$(Platform)\\plugins",
	"copy $(OutDir)"..pluginname..".dll ..\\bin_$(Configuration)_$(Platform)\\plugins\\",
	"copy ..\\plugins\\"..pluginname.."\\*.txt ..\\bin_$(Configuration)_$(Platform)\\plugins\\"
      }

    filter "system:macosx"
      linkoptions "-undefined dynamic_lookup"
    filter { "system:macosx", "options:not disable-client" }
--      project "bzflag" -- the .app needs to bundle the plugins with it
--	dependson(pluginname)
      -- FIXME: workaround for a premake Xcode bug (remove when it's fixed and
      -- uncomment above)
      -- issue link: https://github.com/premake/premake-core/issues/631
      project "build_plugins"
	kind "ConsoleApp"
	links(pluginname)
      project "bzflag"
	dependson "build_plugins"
      -- end workaround
    filter { }
end

-- set up a post-build phase to copy the plugins into the application on macOS
if _OS == "macosx" and not _OPTIONS["disable-client"] then
  project "bzflag"
    postbuildcommands { "mkdir -p ${TARGET_BUILD_DIR}/${PLUGINS_FOLDER_PATH}" }
    for _, pluginname in ipairs(pluginnames) do
      postbuildcommands {
        "cp ${CONFIGURATION_BUILD_DIR}/lib"..pluginname..".dylib ${TARGET_BUILD_DIR}/${PLUGINS_FOLDER_PATH}/"..pluginname..".dylib",
        "cp ../plugins/*/*.txt ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
        "cp ../plugins/*/*.cfg ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
        "cp ../plugins/*/*.bzw ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/"
      }
    end
end
