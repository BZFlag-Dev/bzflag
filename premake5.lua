#!lua

-- BZFlag
-- Copyright (c) 1993-2018 Tim Riker
--
-- This package is free software;  you can redistribute it and/or
-- modify it under the terms of the license found in the file
-- named COPYING that should have accompanied this file.
--
-- THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
-- IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
-- WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

-- game version
local bzVersion = {
  ["major"] = 2,
  ["minor"] = 5,
  ["revision"] = 3,
  ["buildType"] = "DEVEL" -- DEVEL | RC# | STABLE | MAINT
}

local bzBuildDate = os.date("%Y-%m-%d")

-- set up files for installation action
local bzInstallFiles = {
  -- programs
  { ["outDir"] = "bin", ["file"] = "bzadmin" },
  { ["outDir"] = "bin", ["file"] = "bzflag" },
  { ["outDir"] = "bin", ["file"] = "bzfs" },

  -- plugins
  { ["outDir"] = "lib/bzflag", ["outDirDelete"] = true,
    ["file"] = iif(_TARGET_OS == "macosx", "*.dylib", "*.so") },

  -- man files
  { ["inDir"] = "premake5/man", ["outDir"] = "share/man/man5",
    ["file"] = "bzw.5" },
  { ["inDir"] = "premake5/man", ["outDir"] = "share/man/man6",
    ["file"] = "bzadmin.6" },
  { ["inDir"] = "premake5/man", ["outDir"] = "share/man/man6",
    ["file"] = "bzflag.6" },
  { ["inDir"] = "premake5/man", ["outDir"] = "share/man/man6",
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
    print "Generated premake5/installPrefix.txt..."
    io.writefile("premake5/installPrefix.txt", bzInstallPrefix)
  else
    os.remove("premake5/installPrefix.txt")
  end
elseif _ACTION == "install" or _ACTION == "uninstall" then
  -- install/uninstall can't override the prefix, because the paths are
  -- already hard-coded into the binaries, but there can be a staging directory
  if _OPTIONS["destdir"] then
    bzInstallPrefix = _OPTIONS["destdir"]
  else
    local prefixFromFile = io.readfile("build/installPrefix.txt")
    if prefixFromFile then
      bzInstallPrefix = prefixFromFile
    end
  end
end

-- check for the dependencies directory (mandatory for Visual Studio and Xcode)
local depsDir = false
if os.isdir("dependencies") then
  if (_ACTION and string.find(_ACTION, "vs", 0) and
      os.isdir("dependencies/output-windows-release-x86") and
      os.isdir("dependencies/output-windows-debug-x86") and
      os.isdir("dependencies/licenses")) then
    depsDir = true
  elseif (_ACTION and string.find(_ACTION, "xcode", 0) and
      os.isdir("dependencies/output-macOS-release-x86_64") and
      os.isdir("dependencies/output-macOS-debug-x86_64") and
      os.isdir("dependencies/licenses")) then
    depsDir = true
  end
end
if not depsDir then
  if (_ACTION and string.find(_ACTION, "vs", 0)) then
    print "The dependencies package is required by Visual Studio actions, but was not found."
    os.exit(1)
  elseif (_ACTION and string.find(_ACTION, "xcode", 0)) then
    print "The dependencies package is required by Xcode actions, but was not found."
    os.exit(1)
  end
end

-- set up main workspace
workspace(iif(_ACTION and string.find(_ACTION, "vs", 0), "fullbuild", "BZFlag"))
  -- set up command line options
  newoption {
    ["trigger"] = "disable-client",
    ["description"] = "Do not build the BZFlag client"
  }
  newoption {
    ["trigger"] = "disable-server",
        ["description"] = "Do not build the BZFlag server"
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
    ["description"] = "Set a prefix for the BZFlag installation path; default is '/usr/local'"
  }
  newoption {
    ["trigger"] = "destdir",
    ["value"] = "PATH",
    ["description"] = "Set a staging location for installing/uninstalling BZFlag files"
  }

  -- Windows builds require SDL 2
  if _ACTION and string.find(_ACTION, "vs", 0) then
    if _OPTIONS["with-sdl"] == "1" or _OPTIONS["with-sdl"] == "no" then
      print "Error: Windows builds require SDL 2."
      return
    end
  end

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
        if _TARGET_OS == "windows" then
          -- work around an issue with RMDIR /S /Q sometimes giving an error
          -- "The directory is not empty" by running commands twice (allow
          -- error output on the second invokation in case of another issue)
          os.execute("IF EXIST premake5 ( RMDIR /S /Q premake5 2>NUL )")
          os.execute("IF EXIST premake5 ( RMDIR /S /Q premake5 )")
          os.execute("IF EXIST bin_Debug_Win32 ( RMDIR /S /Q bin_Debug_Win32 2>NUL )")
          os.execute("IF EXIST bin_Debug_Win32 ( RMDIR /S /Q bin_Debug_Win32 )")
          os.execute("IF EXIST bin_Release_Win32 ( RMDIR /S /Q bin_Release_Win32 2>NUL )")
          os.execute("IF EXIST bin_Release_Win32 ( RMDIR /S /Q bin_Release_Win32 )")
        else
          os.execute("rm -rf premake5")
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
        if _TARGET_OS == "windows" then
          print "Error: the install action is not supported on Windows."
          return
        end
        if _OPTIONS["prefix"] then
          print "Error: cannot override the --prefix setting for the 'install' action."
          return
        end

        for _, installFile in ipairs(bzInstallFiles) do
          if #os.matchfiles((installFile.inDir or "premake5/bin/Release").."/"..
                            installFile.file) > 0 then
            print("Installing "..bzInstallPrefix.."/"..installFile.outDir..
                  "/"..installFile.file)
            os.execute("mkdir -p "..bzInstallPrefix.."/"..installFile.outDir)
            os.execute("cp "..(installFile.inDir or "premake5/bin/Release")..
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
        if _TARGET_OS == "windows" then
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
  cppdialect "C++11"
  warnings "Default"
  location("premake5/"..iif(_ACTION, _ACTION, ""))
  if not _OPTIONS["disable-client"] then
    startproject "bzflag"
  elseif not _OPTIONS["disable-server"] then
    startproject "bzfs"
  elseif not _OPTIONS["disable-bzadmin"] then
    startproject "bzadmin"
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
    "HAVE_STD__ISNAN",
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
      "HAVE__VSNPRINTF",
      "_WINSOCK_DEPRECATED_NO_WARNINGS"
    }
    sysincludedirs "dependencies/output-windows-$(Configuration)-$(PlatformShortName)/include"
    libdirs "dependencies/output-windows-$(Configuration)-$(PlatformShortName)/lib"
    characterset "MBCS"
    systemversion "latest" -- Visual Studio 2017 defaults to the Windows 8.1 SDK
  filter { "system:windows", "configurations:Release" }
    defines "NDEBUG"

  filter "system:macosx"
    defines "HAVE_CGLGETCURRENTCONTEXT"
    sysincludedirs "dependencies/output-macOS-$CONFIGURATION-$CURRENT_ARCH/include"
    libdirs "dependencies/output-macOS-$CONFIGURATION-$CURRENT_ARCH/lib"
    xcodebuildsettings { ["CLANG_CXX_LIBRARY"] = "libc++",
                         ["MACOSX_DEPLOYMENT_TARGET"] = "10.7",
                         ["LD_RUNPATH_SEARCH_PATHS"] =" @executable_path/../PlugIns" }

  filter { "system:macosx", "action:xcode*" }
    defines "INSTALL_DATA_DIR=\"\"" -- there's one place that has to have it

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

  if _TARGET_OS == "windows" then
    filter "configurations:Release"
      defines { "BZ_BUILD_OS=\"premake-Windows\"" }
    filter "configurations:Debug"
      defines { "BZ_BUILD_OS=\"premake-Windows-dbg\"" }
  elseif _TARGET_OS == "macosx" then
    filter "configurations:Release"
      defines { "BZ_BUILD_OS=\"premake-macOS\"" }
    filter "configurations:Debug"
      defines { "BZ_BUILD_OS=\"premake-macOS-dbg\"" }
  elseif _TARGET_OS == "linux" then
    filter "configurations:Release"
      defines { "BZ_BUILD_OS=\"premake-Linux\"" }
    filter "configurations:Debug"
      defines { "BZ_BUILD_OS=\"premake-Linux-dbg\"" }
  elseif _TARGET_OS == "bsd" then
    filter "configurations:Release"
      defines { "BZ_BUILD_OS=\"premake-BSD\"" }
    filter "configurations:Debug"
      defines { "BZ_BUILD_OS=\"premake-BSD-dbg\"" }
  elseif _TARGET_OS == "solaris" then
    filter "configurations:Release"
      defines { "BZ_BUILD_OS=\"premake-Solaris\"" }
    filter "configurations:Debug"
      defines { "BZ_BUILD_OS=\"premake-Solaris-dbg\"" }
  else
    filter "configurations:Release"
      defines { "BZ_BUILD_OS=\"premake-Unknown\"" }
    filter "configurations:Debug"
      defines { "BZ_BUILD_OS=\"premake-Unknown-dbg\"" }
  end

  -- generate man files
  if _ACTION and _ACTION == "gmake" then
    if _TARGET_OS == "windows" then
      os.execute("if not exist premake5\\man mkdir premake5\\man")
    else
      os.execute("mkdir -p premake5/man")
    end

    local function substituteVersion(dataIn)
      local dataOut = dataIn

      dataOut = string.gsub(dataOut, "@BUILD_DATE@", bzBuildDate)
      dataOut = string.gsub(dataOut, "@PACKAGE_VERSION@", "BZFlag")
      dataOut = string.gsub(dataOut, "@INSTALL_DATA_DIR@", bzInstallPrefix.."/share/bzflag")

      return dataOut
    end

    io.writefile("premake5/man/bzadmin.6", substituteVersion(io.readfile("man/bzadmin.6.in")))
    print("Generated premake5/man/bzadmin.6...")
    io.writefile("premake5/man/bzflag.6", substituteVersion(io.readfile("man/bzflag.6.in")))
    print("Generated premake5/man/bzflag.6...")
    io.writefile("premake5/man/bzfs.6", substituteVersion(io.readfile("man/bzfs.6.in")))
    print("Generated premake5/man/bzfs.6...")
    io.writefile("premake5/man/bzw.5", substituteVersion(io.readfile("man/bzw.5.in")))
    print("Generated premake5/man/bzw.5...")
  end

  -- generate BZFlag-Info.plist
  if _ACTION and string.find(_ACTION, "xcode", 0) then
    io.writefile("premake5/".._ACTION.."/BZFlag-Info.plist",
                 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"..
                 "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">"..
                 "<plist version=\"1.0\">"..
                 "<dict>"..
                 "<key>CFBundleDevelopmentRegion</key><string>en</string>"..
                 "<key>CFBundleExecutable</key><string>${EXECUTABLE_NAME}</string>"..
                 "<key>CFBundleIconFile</key><string>BZFlag</string>"..
                 "<key>CFBundleIdentifier</key><string>org.BZFlag</string>"..
                 "<key>CFBundleInfoDictionaryVersion</key><string>6.0</string>"..
                 "<key>CFBundleName</key><string>BZFlag</string>"..
                 "<key>CFBundlePackageType</key><string>APPL</string>"..
                 "<key>CFBundleShortVersionString</key><string>"..bzVersion.major.."."..bzVersion.minor.."."..bzVersion.revision.."</string>"..
                 "<key>CFBundleSignature</key><string>????</string>"..
                 "<key>CFBundleVersion</key><string>"..bzVersion.major.."."..bzVersion.minor.."."..bzVersion.revision.."</string>"..
                 "<key>LSApplicationCategoryType</key><string>public.app-category.arcade-games</string>"..
                 "<key>LSMinimumSystemVersion</key><string>${MACOSX_DEPLOYMENT_TARGET}</string>"..
                 "<key>NSHumanReadableCopyright</key><string>Copyright (c) 1993-2018 Tim Riker</string>"..
                 "</dict>"..
                 "</plist>");
    print("Generated premake5/".._ACTION.."/BZFlag-Info.plist...")
  end

  -- set up the projects
  if not _OPTIONS["disable-client"] then include "src/3D" end
  if not _OPTIONS["disable-bzadmin"] then include "src/bzadmin" end
  if not _OPTIONS["disable-client"] then include "src/bzflag" end
  if not _OPTIONS["disable-server"] then include "src/bzfs" end
  include "src/common"
  include "src/date"
  include "src/game"
  if not _OPTIONS["disable-client"] then include "src/geometry" end
  if _TARGET_OS == "windows" and
     not _OPTIONS["disable-client"] and
     not _OPTIONS["disable-server"] and
     not _OPTIONS["disable-bzadmin"] and
     not _OPTIONS["disable-plugins"] and
     not _OPTIONS["disable-installer"] then
    include "package/win32/nsis" -- for installer, makehtml, and man2html
  end
  if not _OPTIONS["disable-client"] then include "src/mediafile" end
  include "src/net"
  include "src/obstacle"
  if not _OPTIONS["disable-client"] then include "src/ogl" end
  if not _OPTIONS["disable-client"] then include "src/platform" end
  if not _OPTIONS["disable-plugins"] and not _OPTIONS["disable-server"] then include "plugins" end
  if not _OPTIONS["disable-client"] then include "src/scene" end

-- set up secondary workspaces for Visual Studio (this creates the extra .sln files)
if _ACTION and string.find(_ACTION, "vs", 0) then
  if not _OPTIONS["disable-client"] then
    workspace "bzflag"
      configurations { "Release", "Debug" }
      location("premake5/".._ACTION)
      startproject "bzflag"

      includeexternal "src/bzflag"
      includeexternal "src/3D"
      includeexternal "src/common"
      includeexternal "src/date"
      includeexternal "src/game"
      includeexternal "src/geometry"
      includeexternal "src/mediafile"
      includeexternal "src/net"
      includeexternal "src/obstacle"
      includeexternal "src/ogl"
      includeexternal "src/platform"
      includeexternal "src/scene"
  end

  if not _OPTIONS["disable-server"] then
    workspace "bzfs"
      configurations { "Release", "Debug" }
      location("premake5/".._ACTION)
      startproject "bzfs"

      includeexternal "src/bzfs"
      includeexternal "src/common"
      includeexternal "src/date"
      includeexternal "src/game"
      includeexternal "src/net"
      includeexternal "src/obstacle"
  end

  if not _OPTIONS["disable-bzadmin"] then
    workspace "bzadmin"
      configurations { "Release", "Debug" }
      location("premake5/".._ACTION)
      startproject "bzadmin"

      includeexternal "src/bzadmin"
      includeexternal "src/common"
      includeexternal "src/date"
      includeexternal "src/game"
      includeexternal "src/net"
      includeexternal "src/obstacle"
      includeexternal "src/scene"
  end
end
