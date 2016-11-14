project "plugin_utils"
  kind "StaticLib"
  files { "*.cpp", "*.h" }

  filter "system:linux"
    buildoptions "-fPIC"
