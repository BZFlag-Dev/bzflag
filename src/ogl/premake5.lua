project "ogl"
  kind "StaticLib"
  files { "*.cxx", "*.h", "../../include/*.h" }
  removefiles { "GLCollect.cxx" }
  vpaths {
    ["Header Files/include"] = "../../include",
    ["Header Files"] = "**.h",
    ["Source Files"] = "**.cxx"
  }

  filter "system:windows"
    defines "GLEW_STATIC"
