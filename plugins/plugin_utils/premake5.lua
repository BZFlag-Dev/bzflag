project "plugin_utils"
  kind "StaticLib"
  files { "*.cpp", "*.h" }

  filter "system:windows"
    postbuildcommands {
      "if not exist ..\\bin_$(Configuration)_$(Platform) mkdir ..\\bin_$(Configuration)_$(Platform)",
      "copy \"$(OutDir)plugin_utils.lib\" ..\\bin_$(Configuration)_$(Platform)\\"
  }

  filter "system:linux"
    buildoptions "-fPIC"
