project "bzflag"
  kind "WindowedApp"
  files {
    "*.cxx",
    "*.h",
    "../../include/*.h", -- you access include/ from this project
    "../../AUTHORS",
    "../../COPYING",
    "../../COPYING.LGPL",
    "../../COPYING.MPL",
    "../../ChangeLog",
    "../../DEVINFO",
    "../../NEWS",
    "../../PORTING",
    "../../README",
    "../../README.BeOS",
    "../../README.IRIX",
    "../../README.Linux",
    "../../README.MINGW32",
    "../../README.MacOSX",
    "../../README.SDL",
    "../../README.SOLARIS",
    "../../README.WINDOWS"
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
  dependson "bzfs"
  if not _OPTIONS["disable-bzadmin"] then
    dependson "bzadmin"
  end

  filter "system:windows"
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
    files "../../build/BZFlag-Info.plist"
    links { "Cocoa.framework", "OpenGL.framework" }
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
      "cp BZFlag.icns ${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/",
      "mkdir -p ${TARGET_BUILD_DIR}/${FRAMEWORKS_FOLDER_PATH}"
    }
  filter { "system:macosx",
	   "options:not with-sdl=no",
	   "options:not with-sdl=1" }
    links "SDL2.framework"
    postbuildcommands {
      "if [[ ! -d ${TARGET_BUILD_DIR}/${FRAMEWORKS_FOLDER_PATH}/SDL2.framework ]] ; then",
      "cp -R /Library/Frameworks/SDL2.framework ${TARGET_BUILD_DIR}/${FRAMEWORKS_FOLDER_PATH}/",
      "fi"
    }
  filter { "system:macosx", "options:with-sdl=1" }
    links "SDL.framework"
    postbuildcommands {
      "if [[ ! -d ${TARGET_BUILD_DIR}/${FRAMEWORKS_FOLDER_PATH}/SDL.framework ]] ; then",
      "cp -R /Library/Frameworks/SDL.framework ${TARGET_BUILD_DIR}/${FRAMEWORKS_FOLDER_PATH}/",
      "fi"
    }

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
