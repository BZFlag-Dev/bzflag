project "bzflag"
  kind "WindowedApp"
  files {
    "*.cxx",
    "*.h",
    "../../include/*.h", -- you access include/ from this project
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
    ["Header Files"] = { "**.h", "../../buildsupport/windows/resource.h" },
    ["Resource Files"] = {
      "../../buildsupport/windows/bzflag.ico",
      "../../buildsupport/macos/BZFlag.icns",
      "../../build/BZFlag-Info.plist",
      "../../build/bzflag.rc"
    },
    ["Source Files"] = "**.cxx"
  }
  removefiles { "../../include/GLCollect.h" }
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
    "z"
  }

  filter "system:windows"
    files {
      "../../buildsupport/windows/bzflag.ico",
      "../../build/bzflag.rc",
      "../../buildsupport/windows/resource.h"
    }
    libdirs "$(DXSDK_DIR)/lib/$(PlatformShortName)"
    removelinks "z"
    links { "dsound", "glu32", "opengl32", "regex", "winmm", "ws2_32", "zlib" }
    postbuildcommands {
      "if not exist ..\\bin_$(Configuration)_$(Platform) mkdir ..\\bin_$(Configuration)_$(Platform)",
      "copy \"$(OutDir)bzflag.exe\" ..\\bin_$(Configuration)_$(Platform)\\",
      "copy \"$(BZ_DEPS)\\output-$(Configuration)-$(PlatformShortName)\\bin\\*.dll\" ..\\bin_$(Configuration)_$(Platform)\\"
    }
  filter { "system:windows", "configurations:Release" }
    removelinks "curl"
    links "libcurl"
  filter { "system:windows", "configurations:Debug" }
    removelinks { "cares", "curl" }
    links { "caresd", "libcurl_debug" }
  filter { "system:windows",
	   "options:not with-sdl=no",
	   "options:not with-sdl=1" }
    links { "SDL2", "SDL2main" }

  filter "system:macosx"
    links { "Cocoa.framework", "OpenGL.framework" }
  filter "action:xcode*"
    dependson "bzfs"
    if not _OPTIONS["disable-bzadmin"] then
      dependson "bzadmin"
    end
    files {
      "../../buildsupport/macos/BZFlag.icns",
      "../../build/BZFlag-Info.plist"
    }
    postbuildcommands {
      "cp ${CONFIGURATION_BUILD_DIR}/bzfs ${CONFIGURATION_BUILD_DIR}/${EXECUTABLE_FOLDER_PATH}/",
      "cp ${CONFIGURATION_BUILD_DIR}/bzadmin ${CONFIGURATION_BUILD_DIR}/${EXECUTABLE_FOLDER_PATH}/",
      "mkdir -p ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}",
      "cp ../data/*.png ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
      "cp ../data/*.wav ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
      "mkdir -p ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/fonts",
      "cp ../data/fonts/*.png ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/fonts/",
      "cp ../data/fonts/*.fmt ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/fonts/",
      "cp ../data/fonts/*.License ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/fonts/",
      "cp ../data/fonts/README ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/fonts/",
      "mkdir -p ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/l10n",
      "cp ../data/l10n/*.po ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/l10n/",
      "cp ../data/l10n/*.txt ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/l10n/",
      "cp ../AUTHORS ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
      "cp ../COPYING ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
      "cp ../COPYING.LGPL ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
      "cp ../COPYING.MPL ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
      "cp ../ChangeLog ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
      "cp ../DEVINFO ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
      "cp ../PORTING ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
      "cp ../README ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
      "cp ../README.MacOSX ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
      "mkdir -p ${TARGET_BUILD_DIR}/${FRAMEWORKS_FOLDER_PATH}"
    }
  filter { "system:macosx",
	   "options:not with-sdl=no",
	   "options:not with-sdl=1" }
    links "SDL2.framework"
  filter { "system:macosx", "options:with-sdl=1" }
    links "SDL.framework"
  filter { "action:xcode*",
	   "options:not with-sdl=no",
	   "options:not with-sdl=1" }
    postbuildcommands {
      "if [[ ! -d ${TARGET_BUILD_DIR}/${FRAMEWORKS_FOLDER_PATH}/SDL2.framework ]] ; then",
      "cp -R /Library/Frameworks/SDL2.framework ${TARGET_BUILD_DIR}/${FRAMEWORKS_FOLDER_PATH}/",
      "fi"
    }
  filter { "action:xcode*", "options:with-sdl=1" }
    postbuildcommands {
      "if [[ ! -d ${TARGET_BUILD_DIR}/${FRAMEWORKS_FOLDER_PATH}/SDL.framework ]] ; then",
      "cp -R /Library/Frameworks/SDL.framework ${TARGET_BUILD_DIR}/${FRAMEWORKS_FOLDER_PATH}/",
      "fi"
    }
  filter { "action:xcode*", "options:not disable-plugins" }
    -- the client needs to have the dependency so the plugins will build
    postbuildcommands { "mkdir -p ${TARGET_BUILD_DIR}/${PLUGINS_FOLDER_PATH}" }
    pluginDirNames = os.matchdirs("../../plugins/*")
    for _, pluginDirName in ipairs(pluginDirNames) do
      local pluginName = string.sub(pluginDirName, 15, -1)
      if pluginName ~= "plugin_utils" then
	dependson(pluginName)
	postbuildcommands {
	  "cp ${CONFIGURATION_BUILD_DIR}/"..pluginName..".dylib ${TARGET_BUILD_DIR}/${PLUGINS_FOLDER_PATH}/"..pluginName..".dylib",
	  "cp ../plugins/*/*.txt ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
	  "cp ../plugins/*/*.cfg ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
	  "cp ../plugins/*/*.bzw ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/"
	}
      end
    end

  filter "system:linux"
    links { "GL", "GLU", "pthread" }
  filter { "system:linux",
	   "options:not with-sdl=no",
	   "options:not with-sdl=1" }
    links "SDL2"
  filter { "system:linux", "options:with-sdl=1" }
    links "SDL"
  filter { "system:linux", "options:with-sdl=no" }
    links { "X11", "Xxf86vm" }
