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
-- finish removing remnants of old build system
-- check support for solaris and bsd, perhaps just under SDL 1.2/2

-- game version (this is the only place in the code where this should be set)
local bzVersion = {
  ["major"] = 2,
  ["minor"] = 4,
  ["revision"] = 9,
  ["buildType"] = "DEVEL", -- DEVEL | RC# | STABLE | MAINT
  ["protocol"] = "0221", -- increment on protocol incompatibility
  ["configFileVersion"] = 5
}

local bzBuildDate = os.date("%Y-%m-%d")

if bzVersion.buildType == "STABLE" or bzVersion.buildType == "MAINT" then
  bzVersion.winInstallerType = "release"
  bzVersion.winInstallerRevision = ""
elseif string.find(bzVersion.buildType, "RC", 0) then
  bzVersion.winInstallerType = "RC"
  bzVersion.winInstallerRevision = string.sub(bzVersion.buildType, 3)
else
  bzVersion.winInstallerType = string.lower(bzVersion.buildType)
  bzVersion.winInstallerRevision = ""
end

-- set up files for installation action
local bzInstallFiles = {
  -- programs
  { ["outDir"] = "bin", ["file"] = "bzadmin" },
  { ["outDir"] = "bin", ["file"] = "bzflag" },
  { ["outDir"] = "bin", ["file"] = "bzfs" },

  -- plugins
  { ["outDir"] = "lib/bzflag", ["outDirDelete"] = true,
    ["file"] = iif(_OS == "macosx", "*.dylib", "*.so") },

  -- man files
  { ["inDir"] = "build/man", ["outDir"] = "share/man/man5",
    ["file"] = "bzw.5" },
  { ["inDir"] = "build/man", ["outDir"] = "share/man/man6",
    ["file"] = "bzadmin.6" },
  { ["inDir"] = "build/man", ["outDir"] = "share/man/man6",
    ["file"] = "bzflag.6" },
  { ["inDir"] = "build/man", ["outDir"] = "share/man/man6",
    ["file"] = "bzfs.6" },

  -- fonts
  { ["inDir"] = "data/fonts", ["outDir"] = "share/bzflag/fonts",
    ["outDirDelete"] = true, ["file"] = "*.License" },
  { ["inDir"] = "data/fonts", ["outDir"] = "share/bzflag/fonts",
    ["outDirDelete"] = true, ["file"] = "*.fmt" },
  { ["inDir"] = "data/fonts", ["outDir"] = "share/bzflag/fonts",
    ["outDirDelete"] = true, ["file"] = "*.png" },
  { ["inDir"] = "data/fonts", ["outDir"] = "share/bzflag/fonts",
    ["outDirDelete"] = true, ["file"] = "readme" },

  -- l10n files
  { ["inDir"] = "data/l10n", ["outDir"] = "share/bzflag/l10n",
    ["outDirDelete"] = true, ["file"] = "bzflag_cs_CZ.po" },
  { ["inDir"] = "data/l10n", ["outDir"] = "share/bzflag/l10n",
    ["outDirDelete"] = true, ["file"] = "bzflag_da.po" },
  { ["inDir"] = "data/l10n", ["outDir"] = "share/bzflag/l10n",
    ["outDirDelete"] = true, ["file"] = "bzflag_de.po" },
  { ["inDir"] = "data/l10n", ["outDir"] = "share/bzflag/l10n",
    ["outDirDelete"] = true, ["file"] = "bzflag_en_US_l33t.po" },
  { ["inDir"] = "data/l10n", ["outDir"] = "share/bzflag/l10n",
    ["outDirDelete"] = true, ["file"] = "bzflag_en_US_redneck.po" },
  { ["inDir"] = "data/l10n", ["outDir"] = "share/bzflag/l10n",
    ["outDirDelete"] = true, ["file"] = "bzflag_es.po" },
  { ["inDir"] = "data/l10n", ["outDir"] = "share/bzflag/l10n",
    ["outDirDelete"] = true, ["file"] = "bzflag_fr.po" },
  { ["inDir"] = "data/l10n", ["outDir"] = "share/bzflag/l10n",
    ["outDirDelete"] = true, ["file"] = "bzflag_it.po" },
  { ["inDir"] = "data/l10n", ["outDir"] = "share/bzflag/l10n",
    ["outDirDelete"] = true, ["file"] = "bzflag_kg.po" },
  { ["inDir"] = "data/l10n", ["outDir"] = "share/bzflag/l10n",
    ["outDirDelete"] = true, ["file"] = "bzflag_lt.po" },
  { ["inDir"] = "data/l10n", ["outDir"] = "share/bzflag/l10n",
    ["outDirDelete"] = true, ["file"] = "bzflag_nl.po" },
  { ["inDir"] = "data/l10n", ["outDir"] = "share/bzflag/l10n",
    ["outDirDelete"] = true, ["file"] = "bzflag_pt.po" },
  { ["inDir"] = "data/l10n", ["outDir"] = "share/bzflag/l10n",
    ["outDirDelete"] = true, ["file"] = "bzflag_ru.po" },
  { ["inDir"] = "data/l10n", ["outDir"] = "share/bzflag/l10n",
    ["outDirDelete"] = true, ["file"] = "bzflag_sv.po" },

 -- other data files
  { ["inDir"] = "data", ["outDir"] = "share/bzflag",
    ["outDirDelete"] = true, ["file"] = "*.png" },
  { ["inDir"] = "data", ["outDir"] = "share/bzflag",
    ["outDirDelete"] = true, ["file"] = "*.wav" },
  { ["inDir"] = "data", ["outDir"] = "share/bzflag",
    ["outDirDelete"] = true, ["file"] = "bzflag.desktop" },
  { ["inDir"] = "data", ["outDir"] = "share/bzflag",
    ["outDirDelete"] = true, ["file"] = "bzflag-32x32.xpm" }
}

-- set up the install prefix
local bzInstallPrefix = "/usr/local"
if _ACTION == "gmake" then
  if _OPTIONS["prefix"] then
    -- the gmake action sets the prefix used by install and uninstall also
    if string.find(_OPTIONS["prefix"], "~/", 0) then
      print "Error: please use an absolute path when specifying --prefix."
      os.exit(1)
    end
    bzInstallPrefix = _OPTIONS["prefix"]
    print "Generated build/installPrefix.txt..."
    io.writefile("build/installPrefix.txt", bzInstallPrefix)
  else
    os.remove("build/installPrefix.txt")
  end
elseif _ACTION == "install" or _ACTION == "uninstall" then
  -- install/uninstall can't override it, because the paths are already hard-
  -- coded into the binaries
  local prefixFromFile = io.readfile("build/installPrefix.txt")
  if prefixFromFile then
    bzInstallPrefix = prefixFromFile
  end
end

-- set up workspace
workspace "BZFlag"
  -- set up command line options
  newoption {
    ["trigger"] = "disable-client",
    ["description"] = "Do not build the BZFlag client"
  }
  newoption {
    ["trigger"] = "disable-bzadmin",
    ["description"] = "Do not build the text client"
  }
  newoption {
    ["trigger"] = "disable-plugins",
    ["description"] = "Do not build server plugins"
  }
  newoption {
    ["trigger"] = "disable-installer",
    ["description"] = "Do not build the Windows installer"
  }
  newoption {
    ["trigger"] = "with-sdl",
    ["description"] = "Build the client using SDL",
    ["value"] = "VERSION",
    ["allowed"] = {
      { "2", "use SDL 2 (default)" },
      { "1", "use SDL 1.2" },
      { "no", "do not use SDL" }
    }
  }
  newoption {
    ["trigger"] = "prefix",
    ["value"] = "PATH",
    ["description"] = "Set a prefix for the gmake installation path; default is '/usr/local'"
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
  newaction {
    ["trigger"] = "install",
    ["description"] = "Install program files built by gmake action",
    ["onStart"] =
      function()
	print "Installing..."
      end,
    ["execute"] =
      function()
        if _OS == "windows" then
	  print "Error: the install action is not supported on Windows."
	  return
	end
	if _OPTIONS["prefix"] then
	  print "Error: cannot override the --prefix setting for the 'install' action."
	  return
	end

	for _, installFile in ipairs(bzInstallFiles) do
	  if #os.matchfiles((installFile.inDir or "build/bin/Release").."/"..
			    installFile.file) > 0 then
            print("Installing "..bzInstallPrefix.."/"..installFile.outDir..
		  "/"..installFile.file)
	    os.execute("mkdir -p "..bzInstallPrefix.."/"..installFile.outDir)
	    os.execute("cp "..(installFile.inDir or "build/bin/Release")..
		       "/"..installFile.file.." "..bzInstallPrefix.."/"..
		       installFile.outDir.."/")
	  end
	end
      end,
    ["onEnd"] =
      function()
	print "Done."
      end
  }
  newaction {
    ["trigger"] = "uninstall",
    ["description"] = "Uninstall program files built by gmake action",
    ["onStart"] =
      function()
	print "Uninstalling..."
      end,
    ["execute"] =
      function()
        if _OS == "windows" then
	  print "Error: the uninstall action is not supported on Windows."
	  return
	end
	if _OPTIONS["prefix"] then
	  print "Error: cannot override the --prefix setting for the 'uninstall' action."
	  return
	end

	for _, installFile in ipairs(bzInstallFiles) do
	  if #os.matchfiles(bzInstallPrefix.."/"..installFile.outDir.."/"..
			    installFile.file) > 0 then
	    print("Uninstalling "..bzInstallPrefix.."/"..installFile.outDir..
		  "/"..installFile.file)
	    os.execute("rm "..bzInstallPrefix.."/"..installFile.outDir.."/"..
		       installFile.file)

	    if installFile.outDirDelete then
	      if #os.matchfiles(bzInstallPrefix.."/"..installFile.outDir..
				"/*") == 0 then
		print("Deleting directory "..bzInstallPrefix.."/"..
		      installFile.outDir)
		os.execute("rmdir "..bzInstallPrefix.."/"..installFile.outDir)
	      end
	    end
	  end
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
    "BZ_MAJOR_VERSION="..bzVersion.major,
    "BZ_MINOR_VERSION="..bzVersion.minor,
    "BZ_REV="..bzVersion.revision,
    "BZ_BUILD_TYPE=\""..bzVersion.buildType.."\"",
    "BZ_PROTO_VERSION=\""..bzVersion.protocol.."\"",
    "BZ_CONFIG_DIR_VERSION=\""..bzVersion.major.."."..bzVersion.minor.."\"",
    "BZ_CONFIG_FILE_VERSION="..bzVersion.configFileVersion,
    "BUILD_DATE=\""..bzBuildDate.."\"",
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

  filter "action:gmake"
    defines {
      "INSTALL_LIB_DIR=\""..bzInstallPrefix.."/lib/bzflag/\"",
      "INSTALL_DATA_DIR=\""..bzInstallPrefix.."/share/bzflag\""
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
    frameworkdirs "/Library/Frameworks"
    xcodebuildsettings { ["CLANG_CXX_LIBRARY"] = "libc++",
			 ["MACOSX_DEPLOYMENT_TARGET"] = "10.7",
			 ["LD_RUNPATH_SEARCH_PATHS"] = "@executable_path/../Frameworks @executable_path/../PlugIns" }
    defines "INSTALL_DATA_DIR=\"\"" -- there's one place that has to have it
  filter { "system:macosx", "action:gmake" }
    buildoptions "-F/Library/Frameworks" -- frameworkdirs() isn't passed to gmake
    linkoptions "-F/Library/Frameworks" -- same

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

  -- set up files that need version substitution
  function substituteVersion(dataIn)
    local dataOut = dataIn

    dataOut = string.gsub(dataOut, "BZ_VERSION_MAJOR", bzVersion.major)
    dataOut = string.gsub(dataOut, "BZ_VERSION_MINOR", bzVersion.minor)
    dataOut = string.gsub(dataOut, "BZ_VERSION_REVISION", bzVersion.revision)
    dataOut = string.gsub(dataOut, "BZ_BUILD_TYPE", bzVersion.buildType)
    dataOut = string.gsub(dataOut, "BZ_VERSION_PROTOCOL", bzVersion.protocol)
    dataOut = string.gsub(dataOut, "BZ_VERSION_CONFIG_FILE_VERSION",
			  bzVersion.configFileVersion)
    dataOut = string.gsub(dataOut, "BZ_WIN_INSTALLER_TYPE",
			  bzVersion.winInstallerType)
    dataOut = string.gsub(dataOut, "BZ_WIN_INSTALLER_REVISION",
			  bzVersion.winInstallerRevision)
    dataOut = string.gsub(dataOut, "BZ_BUILD_DATE", bzBuildDate)

    return dataOut
  end

  if _ACTION then
    if string.find(_ACTION, "vs", 0) then
      io.writefile("build/BZFlag.nsi", substituteVersion(
		   io.readfile("buildsupport/windows/installer/BZFlag.nsi.in")))
      print("Generated build/BZFlag.nsi...")
      io.writefile("build/bzflag.rc", substituteVersion(
		   io.readfile("buildsupport/windows/bzflag.rc.in")))
      print("Generated build/bzflag.rc...")
    elseif string.find(_ACTION, "xcode", 0) then
      io.writefile("build/BZFlag-Info.plist", substituteVersion(
		   io.readfile("buildsupport/macos/BZFlag-Info.plist.in")))
      print("Generated build/BZFlag-Info.plist...")
    elseif _ACTION == "gmake" then
      if _OS == "windows" then
	os.execute("if not exist build\\man mkdir build\\man")
      else
	os.execute("mkdir -p build/man")
      end
      io.writefile("build/man/bzadmin.6", substituteVersion(
		   io.readfile("man/bzadmin.6.in")))
      print("Generated build/man/bzadmin.6...")
      io.writefile("build/man/bzflag.6", substituteVersion(
		   io.readfile("man/bzflag.6.in")))
      print("Generated build/man/bzflag.6...")
      io.writefile("build/man/bzfs.6", substituteVersion(
		   io.readfile("man/bzfs.6.in")))
      print("Generated build/man/bzfs.6...")
      io.writefile("build/man/bzw.5", substituteVersion(
		   io.readfile("man/bzw.5.in")))
      print("Generated build/man/bzw.5...")
    end
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
  if _OS == "windows" and
     not _OPTIONS["disable-client"] and
     not _OPTIONS["disable-bzadmin"] and
     not _OPTIONS["disable-plugins"] and
     not _OPTIONS["disable-installer"] then
    include "buildsupport/windows/installer"
  end
