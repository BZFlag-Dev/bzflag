project "geometry"
  kind "StaticLib"
  files { "*.cxx", "*.h", "models/tank/*.cxx", "../../include/*.h" }
  vpaths {
    ["Header Files/include"] = "../../include",
    ["Header Files"] = "**.h",
    ["Source Files"] = "**.cxx"
  }
