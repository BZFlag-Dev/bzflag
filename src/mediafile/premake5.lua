project "mediafile"
  kind "StaticLib"
  files { "*.cxx", "*.h", "../../include/*.h"  }
  removefiles { "OggAudioFile.cxx", "OggAudioFile.h" }
  vpaths {
    ["Header Files/include"] = "../../include",
    ["Header Files"] = "**.h",
    ["Source Files"] = "**.cxx"
  }
