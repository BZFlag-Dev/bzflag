project "bzadmin"
  kind "ConsoleApp"
  files { "*.cxx", "*.h" }
  defines { "BUILDING_BZADMIN" }
  links { "common", "date", "game", "net", "curl", "ncurses" }
