project "bzfs"
  kind "ConsoleApp"
  files { "*.cxx", "*.h" }
  defines "INSIDE_BZ"
  links { "date",
	  "game",
	  "net",
	  "obstacle",
	  "common",
	  "cares",
	  "curl",
	  "z" }

  filter "options:not disable-plugins"
    defines { "BZ_PLUGINS" }
  filter "options:disable-plugins"
    removefiles { "bzfsPlugins.cxx", "bzfsPlugins.h" }

  filter "system:macosx"
    links "Cocoa.framework"

  filter "system:linux"
    linkoptions "-export-dynamic"
    links "dl"
