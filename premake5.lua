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

* make one config.h for all, or just put the defs in premake?
* make plugins work on macOS
* wrap lines correctly (where it's too long, not after each option)
* get rid of "lib" in front of plugin names
* install/uninstall actions (for gmake only, with support for --prefix)
* support for windows, solaris, and bsd?

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
  configurations { "Release", "Debug" }
  filter "configurations:Debug"
    defines { "DEBUG", "DEBUG_RENDERING" }
    symbols "On"
  filter "configurations:Release"
    optimize "On"
  filter { }

  -- set up overall workspace settings
  language "C++"
  basedir "build"
  if not _OPTIONS["disable-client"] then
    startproject "bzflag"
  else
    startproject "bzfs"
  end

  -- set up overall build settings
  defines {
    "HAVE_CONFIG_H"
  }
  includedirs { "include/" }

  -- set up SDL
  filter { "options:not disable-client",
	   "options:not with-sdl=no",
	   "options:not with-sdl=1" }
    defines { "HAVE_SDL", "HAVE_SDL2", "HAVE_SDL2_SDL_H" }
  filter { "options:not disable-client", "options:with-sdl=1" }
    defines { "HAVE_SDL", "HAVE_SDL_SDL_H" }

  -- set up overall platform specific build settings
  filter "system:macosx"
    defines {
      "INSTALL_LIB_DIR=\\\"/usr/local/lib/bzflag\\\"",
      "INSTALL_DATA_DIR=\\\"/usr/local/share/bzflag\\\""
    }
    includedirs { "/usr/local/include", -- for c-ares
		  "Xcode" }
    libdirs { "/usr/local/lib" } -- same
    frameworkdirs { "$(LOCAL_LIBRARY_DIR)/Frameworks", "/Library/Frameworks" }
    xcodebuildsettings { ["CLANG_CXX_LIBRARY"] = "libc++",
			 ["CLANG_CXX_LANGUAGE_STANDARD"] = "c++0x",
			 ["MACOSX_DEPLOYMENT_TARGET"] = "10.7" }

  filter { "system:linux" }
    defines {
      "INSTALL_LIB_DIR=\"/usr/local/lib/bzflag\"",
      "INSTALL_DATA_DIR=\"/usr/local/share/bzflag\""
    }
    buildoptions { "-Wall", "-Wextra", "-Wcast-qual", "-Wredundant-decls",
		   "-Wshadow", "-Wundef", "-pedantic" }
  filter { }

  -- set up the build (build order/dependencies are honored notwithstanding the
  -- listed order here; this order is how we want the projects to show up in
  -- the IDEs since the startproject option isn't fully supported)
  if not _OPTIONS["disable-client"] then include "src/bzflag" end
  include "src/bzfs"
  if not _OPTIONS["disable-bzadmin"] then include "src/bzadmin" end
  if not _OPTIONS["disable-client"] then include "src/3D" end
  include "src/common"
  include "src/date"
  include "src/game"
  if not _OPTIONS["disable-client"] then include "src/geometry" end
  if not _OPTIONS["disable-client"] then include "src/mediafile" end
  include "src/net"
  include "src/obstacle"
  if not _OPTIONS["disable-client"] then include "src/ogl" end
  if not _OPTIONS["disable-client"] then include "src/platform" end
  if not _OPTIONS["disable-client"] then include "src/scene" end
  if not _OPTIONS["disable-plugins"] then include "plugins" end
