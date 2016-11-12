project "bzfs"
  kind "ConsoleApp"
  files { "*.cxx", "*.h" }
  links { "common",
	  "date",
	  "game",
	  "net",
	  "obstacle",
	  "cares",
	  "curl",
	  "z" }
