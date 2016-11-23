project "bzadmin"
  kind "ConsoleApp"
  files { "*.cxx", "*.h" }
  defines { "BUILDING_BZADMIN" }
  links { "date", "game", "net", "common", "curl", "ncurses" }

  filter "system:windows"
    removelinks { "ncurses" }
    links { "pdcurses", "regex", "winmm", "ws2_32" }
    postbuildcommands {
      "if not exist ..\\bin_$(Configuration)_$(Platform) mkdir ..\\bin_$(Configuration)_$(Platform)",
      "copy \"$(OutDir)bzadmin.exe\" ..\\bin_$(Configuration)_$(Platform)\\",
      "copy \"$(BZ_DEPS)\\output-$(Configuration)-$(PlatformShortName)\\bin\\*.dll\" ..\\bin_$(Configuration)_$(Platform)\\"
    }
  filter { "system:windows", "configurations:Release" }
    removelinks "curl"
    links "libcurl"
  filter { "system:windows", "configurations:Debug" }
    removelinks { "cares", "curl" }
    links { "caresd", "libcurl_debug" }

  filter "system:macosx"
    links "Cocoa.framework"
