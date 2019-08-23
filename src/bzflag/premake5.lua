project "bzflag"
  kind "WindowedApp"
  files {
    "*.cxx",
    "*.h",
    "../../include/*.h",
    "../../AUTHORS",
    "../../COPYING*",
    "../../ChangeLog",
    "../../DEVINFO",
    "../../NEWS",
    "../../PORTING",
    "../../README*",
    "../../include/*.h"
  }
  vpaths {
    ["Documentation"] = {
      "../../AUTHORS",
      "../../COPYING*",
      "../../ChangeLog",
      "../../DEVINFO",
      "../../NEWS",
      "../../PORTING",
      "../../README*"
    },
    ["Header Files/include"] = "../../include",
    ["Header Files"] = { "**.h", "../../MSVC/resource.h" },
    ["Resource Files"] = {
      "../../MSVC/bzflag.ico",
      "../../Xcode/BZFlag.icns",
      "../../premake5/"..iif(_ACTION, _ACTION, "").."/BZFlag-Info.plist",
      "../../MSVC/bzflag.rc",
      "../../MSVC/Version.rc"
    },
    ["Source Files"] = "**.cxx"
  }
  links {
    "3D",
    "date",
    "game",
    "mediafile",
    "net",
    "obstacle",
    "ogl",
    "platform",
    "scene",
    "geometry",
    "common",
    "cares",
    "curl",
    "GL",
    "GLEW",
    "GLU",
    "png",
    "z"
  }

  filter "system:windows"
    files {
      "../../MSVC/bzflag.ico",
      "../../MSVC/bzflag.rc",
      "../../MSVC/resource.h",
      "../../MSVC/Version.rc"
    }
    defines "GLEW_STATIC"
    libdirs "$(DXSDK_DIR)/lib/$(PlatformShortName)"
    removelinks { "GL", "GLU", "png", "z" }
    links { "dsound", "glu32.lib", "libpng", "opengl32", "regex", "winmm", "ws2_32", "zlib" }
    dpiawareness "HighPerMonitor"
    postbuildcommands {
      "if not exist ..\\..\\bin_$(Configuration)_$(Platform) mkdir ..\\..\\bin_$(Configuration)_$(Platform)",
      "copy \"$(OutDir)bzflag.exe\" ..\\..\\bin_$(Configuration)_$(Platform)\\",
      "copy \"..\\..\\dependencies\\output-windows-$(Configuration)-$(PlatformShortName)\\bin\\*.dll\" ..\\..\\bin_$(Configuration)_$(Platform)\\",
      "if not exist ..\\..\\bin_$(Configuration)_$(Platform)\\licenses mkdir ..\\..\\bin_$(Configuration)_$(Platform)\\licenses",
      "copy \"..\\..\\dependencies\\licenses\\*\" ..\\..\\bin_$(Configuration)_$(Platform)\\licenses\\\""
    }
  filter { "system:windows", "configurations:Release" }
    removelinks { "curl", "GLEW" }
    links { "libcurl", "glew32s" }
  filter { "system:windows", "configurations:Debug" }
    removelinks { "cares", "curl", "GLEW" }
    links { "caresd", "libcurl_debug", "glew32sd" }
  filter { "system:windows",
           "options:not with-sdl=no",
           "options:not with-sdl=1" }
    links { "SDL2", "SDL2main" }

  filter "system:macosx"
    removelinks { "GL", "GLU" }
    links {
      "AudioToolbox.framework",
      "AudioUnit.framework",
      "Carbon.framework",
      "Cocoa.framework",
      "CoreAudio.framework",
      "CoreVideo.framework",
      "ForceFeedback.framework",
      "iconv",
      "IOKit.framework",
      "Metal.framework",
      "OpenGL.framework"
    }
  filter "action:xcode*"
    if not _OPTIONS["disable-server"] then
      dependson "bzfs"
    end
    if not _OPTIONS["disable-bzadmin"] then
      dependson "bzadmin"
    end
    files {
      "../../Xcode/BZFlag.icns",
      "../../premake5/"..iif(_ACTION, _ACTION, "").."/BZFlag-Info.plist"
    }
    if not _OPTIONS["disable-server"] then
      postbuildcommands "cp \"${CONFIGURATION_BUILD_DIR}/bzfs\" \"${CONFIGURATION_BUILD_DIR}/${EXECUTABLE_FOLDER_PATH}/\""
    end
    if not _OPTIONS["disable-bzadmin"] then
      postbuildcommands "cp \"${CONFIGURATION_BUILD_DIR}/bzadmin\" \"${CONFIGURATION_BUILD_DIR}/${EXECUTABLE_FOLDER_PATH}/\""
    end
    postbuildcommands {
      "mkdir -p \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}\"",
      "cp ../../data/*.png \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/\"",
      "cp ../../data/*.wav \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/\"",
      "mkdir -p \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/fonts\"",
      "cp ../../data/fonts/*.png \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/fonts/\"",
      "cp ../../data/fonts/*.fmt \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/fonts/\"",
      "cp ../../data/fonts/*.License \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/fonts/\"",
      "cp ../../data/fonts/README \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/fonts/\"",
      "mkdir -p \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/l10n\"",
      "cp ../../data/l10n/*.po \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/l10n/\"",
      "cp ../../data/l10n/*.txt \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/l10n/\"",
      "cp ../../AUTHORS \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/\"",
      "cp ../../COPYING \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/\"",
      "cp ../../COPYING.LGPL \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/\"",
      "cp ../../COPYING.MPL \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/\"",
      "cp ../../ChangeLog \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/\"",
      "cp ../../DEVINFO \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/\"",
      "cp ../../PORTING \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/\"",
      "cp ../../README \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/\"",
      "cp ../../README.macOS \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/\"",
      "cp -R ../../dependencies/licenses \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/\""
    }
  filter { "system:macosx",
           "options:not with-sdl=no",
           "options:not with-sdl=1" }
    links "SDL2"
  filter { "system:macosx", "options:with-sdl=1" }
    links "SDL"
  filter { "action:xcode*", "options:not disable-plugins", "options:not disable-server" }
    -- the client needs to have the dependency so the plugins will build
    postbuildcommands {
      "mkdir -p \"${TARGET_BUILD_DIR}/${PLUGINS_FOLDER_PATH}\"",
      "cp ../../plugins/*/*.txt \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/\"",
      "cp ../../plugins/*/*.cfg \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/\"",
      "cp ../../plugins/*/*.bzw \"${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/\""
    }
    pluginDirNames = os.matchdirs("../../plugins/*")
    for _, pluginDirName in ipairs(pluginDirNames) do
      local pluginName = string.sub(pluginDirName, 15, -1)
      if pluginName ~= "plugin_utils" then
        dependson(pluginName)
        postbuildcommands { "cp \"${CONFIGURATION_BUILD_DIR}/"..pluginName..".dylib\" \"${TARGET_BUILD_DIR}/${PLUGINS_FOLDER_PATH}/"..pluginName..".dylib\"" }
      end
    end

  filter "system:linux"
    links "pthread"
  filter { "system:linux",
           "options:not with-sdl=no",
           "options:not with-sdl=1" }
    links "SDL2"
  filter { "system:linux", "options:with-sdl=1" }
    links "SDL"
  filter { "system:linux", "options:with-sdl=no" }
    links { "X11", "Xxf86vm" }
