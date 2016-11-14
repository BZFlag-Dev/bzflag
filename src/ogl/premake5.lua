project "ogl"
  kind "StaticLib"
  files { "*.cxx", "*.h" }
  removefiles { "GLCollect.cxx" }
