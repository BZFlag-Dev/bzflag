project "fairCTF"
  kind "SharedLib"
  targetprefix ""
  files {
    "../../include/bzfsApi.h",
    "*.cpp",
    "*.h",
    "*.txt",
    "*.cfg"
  }
  vpaths {
    ["*"] = { "**.txt", "**.cfg" },
    ["Header Files"] = { "**.h", "../../include/**.h" },
    ["Source Files"] = "**.cpp",
  }

  filter "system:windows"
    defines {
      "WIN32",
      "_USE_MATH_DEFINES",
      "_WINDOWS",
      "_USRDLL",
      "fairCTF_EXPORTS"
    }
    libdirs "$(OutDir)"
    links "bzfs.lib" -- ".lib" required to distinguish from the executable
    dependson "bzfs"
    postbuildcommands {
      "if not exist ..\\..\\bin_$(Configuration)_$(Platform) mkdir ..\\..\\bin_$(Configuration)_$(Platform)",
      "if not exist ..\\..\\bin_$(Configuration)_$(Platform)\\plugins mkdir ..\\..\\bin_$(Configuration)_$(Platform)\\plugins",
      "copy $(OutDir)fairCTF.dll ..\\..\\bin_$(Configuration)_$(Platform)\\plugins\\",
      "copy ..\\..\\plugins\\fairCTF\\*.txt ..\\..\\bin_$(Configuration)_$(Platform)\\plugins\\",
      "if exist ..\\..\\plugins\\fairCTF\\*.cfg ( copy ..\\..\\plugins\\fairCTF\\*.cfg ..\\..\\bin_$(configuration)_$(Platform)\\plugins\\ )"
    }

  filter "system:macosx"
    linkoptions "-undefined dynamic_lookup"

  filter "system:linux"
    symbols "On"
  filter { }
