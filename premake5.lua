#!lua

-- BZFlag
-- Copyright (c) 1993-2016 Tim Riker
--
-- This package is free software;  you can redistribute it and/or
-- modify it under the terms of the license found in the file
-- named COPYING that should have accompanied this file.
--
-- THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
-- IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
-- WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

--[[

TODO:

* support for platforms other than macOS
* install/uninstall actions (for gmake only, with support for --prefix)
* we may need to avoid linking to the OS libraries when building plugins

]]

workspace "BZFlag"
  -- set up command line options
  newoption {
    ["trigger"] = "disable-client",
    ["description"] = "do not build the BZFlag client"
  }
  newoption {
    ["trigger"] = "disable-bzadmin",
    ["description"] = "do not build the text client"
  }
  newoption {
    ["trigger"] = "disable-plugins",
    ["description"] = "do not build server plugins"
  }
  newoption {
    ["trigger"] = "with-sdl",
    ["description"] = "build the client using SDL",
    ["value"] = "VERSION",
    ["allowed"] = {
      { "2", "use SDL 2 (default)" },
      { "1", "use SDL 1.2" },
      { "no", "do not use SDL" }
    }
  }

  -- set up configurations
  configurations { "Debug", "Release" }
  filter "configurations:Debug"
    defines { "DEBUG", "DEBUG_RENDERING" }
    symbols "On"
  filter "configurations:Release"
    optimize "Full"
  filter { }

  -- set up overall build settings
  language "C++"
  basedir "build"
  includedirs { "include/" }
  if not _OPTIONS["disable-client"] then
    startproject "bzflag"
  else
    startproject "bzfs"
  end

  -- set up preprocessor macros
  defines {
    "HAVE_CONFIG_H",
    "INSTALL_LIB_DIR=\\\"/usr/local/lib/bzflag/\\\"",
    "INSTALL_DATA_DIR=\\\"/usr/local/share/bzflag\\\""
  }
  filter { "options:not disable-client", "options:not with-sdl=no" }
    defines { "HAVE_SDL" }
  filter { "options:not disable-client",
	   "options:not with-sdl=no",
	   "options:not with-sdl=1" }
    defines { "HAVE_SDL2", "HAVE_SDL2_SDL_H" }
  filter { "options:not disable-client", "options:with-sdl=1" }
    defines { "HAVE_SDL_SDL_H" }
  filter "options:not disable-plugins"
    defines { "BZ_PLUGINS" }

  -- set up macOS platform specific build settings
  filter "system:macosx"
    includedirs { "/usr/local/include", -- assuming c-ares is in /usr/local
		  "Xcode" }
    libdirs { "/usr/local/lib" } -- same
    frameworkdirs { "$(LOCAL_LIBRARY_DIR)/Frameworks", "/Library/Frameworks" }
    links { "Cocoa.framework" }
    xcodebuildsettings { ["CLANG_CXX_LIBRARY"] = "libc++",
			 ["CLANG_CXX_LANGUAGE_STANDARD"] = "c++0x",
			 ["MACOSX_DEPLOYMENT_TARGET"] = "10.7" }
  filter { "system:macosx", "options:not disable-client" }
    links { "OpenGL.framework" }
  filter { "system:macosx",
	   "options:not disable-client",
	   "options:not with-sdl=no",
	   "options:not with-sdl=1" }
    includedirs { "/Library/Frameworks/SDL2.framework/Headers" }
    links { "SDL2.framework" }
  filter { "system:macosx",
	   "options:not disable-client",
	   "options:with-sdl=1" }
    includedirs { "/Library/Frameworks/SDL.framework/Headers" }
    links { "SDL.framework" }

  -- set up linux platform specific build settings
  filter { "system:linux", "options:not disable-client" }
    links { "GL" }
  filter { "system:linux",
	   "options:not disable-client",
	   "options:not sdl=no",
	   "options:not sdl=1" }
    links { "SDL2" }
  filter { "system:linux", "options:not disable-client", "options:sdl=1" }
    links { "SDL" }
  filter { }

  -- set up the build (build order/dependencies are honored notwithstanding the
  -- listed order here; this order is how we want the projects to show up in
  -- the IDEs since the startproject option isn't fully supported)
  if not _OPTIONS["disable-client"] then
    include "src/bzflag"
  end
  include "src/bzfs"
  if not _OPTIONS["disable-bzadmin"] then
    include "src/bzadmin"
  end
  include "src/3D"
  include "src/common"
  include "src/date"
  include "src/game"
  include "src/geometry"
  include "src/mediafile"
  include "src/net"
  include "src/obstacle"
  include "src/ogl"
  include "src/platform"
  include "src/scene"
  if not _OPTIONS["disable-plugins"] then
    include "plugins"
  end
