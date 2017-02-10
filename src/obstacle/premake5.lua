project "obstacle"
  kind "StaticLib"
  files { "*.cxx", "*.h", "../../include/*.h" }
  filter "options:disable-client"
    removefiles { "*SceneNodeGenerator.cxx" }
  filter { }
  vpaths {
    ["Header Files/include"] = "../../include",
    ["Header Files"] = "**.h",
    ["Source Files"] = "**.cxx"
   }
