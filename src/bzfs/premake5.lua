project "bzfs"
  kind "ConsoleApp"
  files { "*.cxx", "*.h" }
  defines "INSIDE_BZ"
  links {
    "date",
    "game",
    "net",
    "obstacle",
    "common",
    "cares",
    "curl",
    "z"
  }

  filter "options:not disable-plugins"
    defines "BZ_PLUGINS"
  filter "options:disable-plugins"
    removefiles { "bzfsPlugins.cxx", "bzfsPlugins.h" }

  filter "system:windows"
    removelinks "z"
    links { "regex", "winmm", "ws2_32", "zlib" }
    postbuildcommands {
      "if not exist ..\\bin_$(Configuration)_$(Platform) mkdir ..\\bin_$(Configuration)_$(Platform)",
      "copy \"$(OutDir)bzfs.exe\" ..\\bin_$(Configuration)_$(Platform)\\",
      "copy \"$(BZ_DEPS)\\output-$(Configuration)-$(PlatformShortName)\\bin\\*.dll\" ..\\bin_$(Configuration)_$(Platform)\\"
    }
  filter { "system:windows", "configurations:Release" }
    removelinks "curl"
    links "libcurl"
  filter { "system:windows", "configurations:Debug" }
    removelinks { "cares", "curl" }
    links { "caresd", "libcurl_debug" }
  filter { "system:windows", "options:not disable-plugins" }
    postbuildcommands "copy \"$(OutDir)bzfs.lib\" ..\\bin_$(Configuration)_$(Platform)\\"

  filter "system:macosx"
    links "Cocoa.framework"

  filter "system:linux"
    linkoptions "-export-dynamic"
    links "dl"
