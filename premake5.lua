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

-- To the greatest extent possible, please use the following order of options
-- when adding projects to the configuration:
--
-- kind
-- language
-- targetprefix
-- include
-- files
-- defines
-- includedirs
-- sysincludedirs
-- buildoptions
-- libdirs
-- frameworkdirs
-- linkoptions
-- links
-- dependson
-- <various other settings>
-- prebuildcommands
-- postbuildcommands
--
-- Generic (unfiltered) options should be specified first, followed by any
-- configuration-specific, system-specific, or otherwise filtered options.
-- Arguments to these options should be given in alphabetical order whenever
-- possible. For links, try to list local libraries alphabetically followed
-- by system libraries alphabetically, but because the order of libraries
-- matters to some linkers, we will sometimes have to deviate from this.
--
-- TODO:
--
-- figure out dependencies between executables (mac .app needs more than windows)
  -- link dependencies
  -- on Windows, the installer
  -- on macOS, bzflag.app
-- refactor plugin scripts
--
-- install/uninstall actions (for gmake only, with support for --prefix)
-- man files need to be generated on Linux and maybe on macOS
--
-- finish removing remnants of old build system
--
-- check support for solaris and bsd, perhaps just under SDL 1.2/2
-- check for FIXMEs (especially the preprocessor definitions)

-- set up workspace
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
    ["trigger"] = "disable-installer",
    ["description"] = "do not build the Windows installer"
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

  -- custom actions
  newaction {
    ["trigger"] = "clean",
    ["description"] = "Delete generated project and build files",
    ["onStart"] =
      function()
	print "Cleaning source tree..."
      end,
    ["execute"] =
      function()
	if _OS == "windows" then
	  os.execute("IF EXIST build ( RMDIR /S /Q build )")
	  os.execute("IF EXIST bin_Debug_Win32 ( RMDIR /S /Q bin_Debug_Win32 )")
	  os.execute("IF EXIST bin_Release_Win32 ( RMDIR /S /Q bin_Release_Win32 )")
	else
	  os.execute("rm -rf build")
	end
      end,
    ["onEnd"] =
      function()
	print "Done."
      end
  }

  -- set up configurations
  configurations { "Release", "Debug" }
  filter "configurations:Release"
    optimize "On"
  filter "configurations:Debug"
    defines { "DEBUG", "_DEBUG", "DEBUG_RENDERING" }
    symbols "On"
  filter { }

  -- set up overall workspace settings
  language "C++"
  flags "C++11"
  warnings "Default"
  basedir "build"
  if not _OPTIONS["disable-client"] then
    startproject "bzflag"
  else
    startproject "bzfs"
  end

  -- set up workspace build settings
  filter { "options:not disable-client",
	   "options:not with-sdl=no",
	   "options:not with-sdl=1" }
    defines { "HAVE_SDL", "HAVE_SDL2", "HAVE_SDL2_SDL_H" }
  filter { "options:not disable-client", "options:with-sdl=1" }
    defines { "HAVE_SDL", "HAVE_SDL_SDL_H" }
  filter { }

  defines {
    -- // FIXME: all of this needs to be dynamic or eliminated
    -- #define BUILD_DATE "2016-11-13"
    -- #define BZFLAG_DATA "/usr/local/share/bzflag"
    -- #define LT_OBJDIR ".libs/"
    -- #define PACKAGE "bzflag"
    -- #define PACKAGE_BUGREPORT "http://BZFlag.org/"
    -- #define PACKAGE_NAME "BZFlag"
    -- #define PACKAGE_STRING "BZFlag 2.4.9"
    -- #define PACKAGE_TARNAME "bzflag"
    -- #define PACKAGE_URL ""
    -- #define PACKAGE_VERSION "2.4.9"
    -- #define VERSION "2.4.9"
    -- #define NDEBUG 1

    "HAVE_REGEX_H",
    "ROBOT",
    "HAVE_STD__COUNT",
    "HAVE_ASINF",
    "HAVE_ATANF",
    "HAVE_ATAN2F",
    "HAVE_COSF",
    "HAVE_EXPF",
    "HAVE_FABSF",
    "HAVE_FLOORF",
    "HAVE_FMODF",
    "HAVE_LOGF",
    "HAVE_POWF",
    "HAVE_SINF",
    "HAVE_SQRTF",
    "HAVE_TANF",
    "HAVE_LOG10F",
    "HAVE_STD__MIN",
    "HAVE_STD__MAX",
    "HAVE_ARES_LIBRARY_INIT"
  }
  includedirs "include/"

  filter "system:not windows"
    defines {
      "INSTALL_LIB_DIR=\"/usr/local/lib/bzflag\"",
      "INSTALL_DATA_DIR=\"/usr/local/share/bzflag\""
    }

  filter "system:windows"
    defines {
      "HAVE_WAITFORSINGLEOBJECT",
      "_CRT_SECURE_NO_DEPRECATE",
      "_CRT_NONSTDC_NO_DEPRECATE",
      "HAVE_STRICMP",
      "HAVE__STRICMP",
      "HAVE__STRNICMP",
      "HAVE__VSNPRINTF"
    }
    includedirs "buildsupport/Windows"
    sysincludedirs "$(BZ_DEPS)/output-$(Configuration)-$(PlatformShortName)/include"
    libdirs "$(BZ_DEPS)/output-$(Configuration)-$(PlatformShortName)/lib"
    characterset "MBCS"

  filter "system:macosx"
    defines "HAVE_CGLGETCURRENTCONTEXT"
    sysincludedirs "/usr/local/include" -- for c-ares
    libdirs "/usr/local/lib" -- same
    frameworkdirs "$(LOCAL_LIBRARY_DIR)/Frameworks"
    xcodebuildsettings { ["CLANG_CXX_LIBRARY"] = "libc++",
			 ["MACOSX_DEPLOYMENT_TARGET"] = "10.7",
			 ["LD_RUNPATH_SEARCH_PATHS"] = "@executable_path/../Frameworks" }

  filter "system:linux"
    defines {
      "HALF_RATE_AUDIO",
      "HAVE_FF_EFFECT_DIRECTIONAL",
      "HAVE_FF_EFFECT_RUMBLE",
      "HAVE_LIMITS_H",
      "HAVE_LINUX_INPUT_H",
      "HAVE_SCHED_SETAFFINITY",
      "HAVE_STDCXX_0X",
      "HAVE_VALUES_H",
      "HAVE_X11_EXTENSIONS_XF86VMODE_H",
      "LIBCURL_FEATURE_ASYNCHDNS",
      "LIBCURL_FEATURE_IDN",
      "LIBCURL_PROTOCOL_IMAP",
      "LIBCURL_PROTOCOL_POP3",
      "LIBCURL_PROTOCOL_RTSP",
      "LIBCURL_PROTOCOL_SMTP",
      "XF86VIDMODE_EXT"
    }

  filter "system:windows or macosx"
    defines "HAVE_SLEEP"

  filter "system:macosx or linux"
    defines {
      "HAVE_ACOSF",
      "HAVE_ATEXIT",
      "HAVE_CMATH",
      "HAVE_CSTDIO",
      "HAVE_CSTDLIB",
      "HAVE_CSTRING",
      "HAVE_DLFCN_H",
      "HAVE_FCNTL_H",
      "HAVE_HSTRERROR",
      "HAVE_HYPOTF",
      "HAVE_INTTYPES_H",
      "HAVE_LIBCURL",
      "HAVE_LIBM",
      "HAVE_MEMORY_H",
      "HAVE_NCURSES_H",
      "HAVE_NETDB_H",
      "HAVE_PTHREADS",
      "HAVE_SCHED_H",
      "HAVE_SELECT",
      "HAVE_SOCKLEN_T",
      "HAVE_STDINT_H",
      "HAVE_STDLIB_H",
      "HAVE_STD__ISNAN",
      "HAVE_STRINGS_H",
      "HAVE_STRING_H",
      "HAVE_SYS_PARAM_H",
      "HAVE_SYS_SOCKET_H",
      "HAVE_SYS_STAT_H",
      "HAVE_SYS_TYPES_H",
      "HAVE_UNISTD_H",
      "HAVE_USLEEP",
      "HAVE_VSNPRINTF",
      "LIBCURL_FEATURE_IPV6",
      "LIBCURL_FEATURE_LIBZ",
      "LIBCURL_FEATURE_NTLM",
      "LIBCURL_FEATURE_SSL",
      "LIBCURL_PROTOCOL_DICT",
      "LIBCURL_PROTOCOL_FILE",
      "LIBCURL_PROTOCOL_FTP",
      "LIBCURL_PROTOCOL_FTPS",
      "LIBCURL_PROTOCOL_HTTP",
      "LIBCURL_PROTOCOL_HTTPS",
      "LIBCURL_PROTOCOL_LDAP",
      "LIBCURL_PROTOCOL_TELNET",
      "LIBCURL_PROTOCOL_TFTP",
      "STDC_HEADERS",
      "_REENTRANT"
    }
  filter { }

  if _OS == "windows" then
    defines { "BZ_BUILD_OS=\"Win\"" }
  elseif _OS == "macosx" then
    defines { "BZ_BUILD_OS=\"Mac\"" }
  elseif _OS == "linux" then
    defines { "BZ_BUILD_OS=\"Linux\"" }
  elseif _OS == "bsd" then
    defines { "BZ_BUILD_OS=\"BSD\"" }
  elseif _OS == "solaris" then
    defines { "BZ_BUILD_OS=\"Solaris\"" }
  else
    defines { "BZ_BUILD_OS=\"Unknown\"" }
  end

  if _ACTION then
    filter "action:vs*"
      defines { "BZ_COMPILER_VS_VERSION=\""..string.sub(_ACTION, 3).."\"" }
    filter "action:xcode*"
      defines { "BZ_COMPILER_XCODE_VERSION=\"${XCODE_VERSION_ACTUAL}\"" }
    filter { }
  end

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

  -- set up the installer on windows
  if _OS == "windows" and
     not _OPTIONS["disable-client"] and
     not _OPTIONS["disable-bzadmin"] and
     not _OPTIONS["disable-plugins"] and
     not _OPTIONS["disable-installer"] then
	project "man2html"
	  kind "ConsoleApp"
	  language "C"
	  files "misc/man2html.c"

	project "makehtml"
	  kind "Utility"
	  files "man/*.in"
	  dependson "man2html"
	  postbuildcommands {
	    "if not exist \"$(SolutionDir)..\\bin_$(Configuration)_$(Platform)\\docs\" mkdir \"$(SolutionDir)..\\bin_$(Configuration)_$(Platform)\\docs\"",
	    "$(OutDir)man2html.exe < ..\\man\\bzadmin.6.in > \"$(SolutionDir)..\\bin_$(Configuration)_$(Platform)\\docs\\bzadmin.html\"",
	    "$(OutDir)man2html.exe < ..\\man\\bzflag.6.in > \"$(SolutionDir)..\\bin_$(Configuration)_$(Platform)\\docs\\bzflag.html\"",
	    "$(OutDir)man2html.exe < ..\\man\\bzfs.6.in > \"$(SolutionDir)..\\bin_$(Configuration)_$(Platform)\\docs\\bzfs.html\"",
	    "$(OutDir)man2html.exe < ..\\man\\bzw.5.in > \"$(SolutionDir)..\\bin_$(Configuration)_$(Platform)\\docs\\bzw.html\""
	  }

	project "installer"
	  kind "Utility"
	  files "buildsupport/windows/BZFlag.nsi"
	  dependson { "bzflag", "makehtml" }
	  filter "configurations:Release"
	    postbuildcommands "\"$(ProgramFiles)\\nsis\\makensis.exe\" \"..\\buildsupport\\windows\\BZFlag.nsi\""
	  filter { }
  end
