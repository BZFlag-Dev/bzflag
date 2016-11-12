project "mediafile"
  kind "StaticLib"
  files { "*.cxx", "*.h" }
  removefiles { "OggAudioFile.cxx", "OggAudioFile.h" }