-- enumerate the plugins to include (modify this list to add a plugin)
plugin_names = {
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

-- set up the individual plugin projects from the list above and add them as
-- dependencies to bzfs
for index,plugin_name in ipairs(plugin_names) do
  project(plugin_name)
    kind "SharedLib"
    files { plugin_name.."/*.cpp", plugin_name.."/*.h" }
    includedirs "plugin_utils"
    filter "system:macosx"
      linkoptions "-undefined dynamic_lookup"
    filter { }
    links { "plugin_utils" }
  project "bzfs"
    dependson(plugin_name)
end

-- set up a post-build phase to copy the plugins into the application on macOS
if _OS == "macosx" then
  project "bzflag"
    postbuildcommands { "mkdir -p ${TARGET_BUILD_DIR}/${PLUGINS_FOLDER_PATH}" }
    for index,plugin_name in ipairs(plugin_names) do
      postbuildcommands {
        "cp ${CONFIGURATION_BUILD_DIR}/lib"..plugin_name..".dylib ${TARGET_BUILD_DIR}/${PLUGINS_FOLDER_PATH}/"..plugin_name..".dylib",
        "cp ../plugins/*/*.txt ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
        "cp ../plugins/*/*.cfg ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
        "cp ../plugins/*/*.bzw ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/"
      }
    end
end
