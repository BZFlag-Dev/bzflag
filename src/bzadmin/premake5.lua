project "bzadmin"
  kind "ConsoleApp"
  files { "*.cxx", "*.h" }
  defines { "BUILDING_BZADMIN" }
  links { "date", "game", "net", "common", "curl", "ncurses" }
