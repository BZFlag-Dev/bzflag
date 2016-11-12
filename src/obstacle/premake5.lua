project "obstacle"
  kind "StaticLib"
  files { "*.cxx", "*.h" }
  filter "options:disable-client"
    removefiles { "*SceneNodeGenerator.cxx" }