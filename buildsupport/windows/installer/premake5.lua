project "man2html"
  kind "ConsoleApp"
  language "C"
  files "../../../misc/man2html.c"

project "makehtml"
  kind "Utility"
  files "../../../man/*.in"
  dependson "man2html"
  postbuildcommands {
    "if not exist \"$(SolutionDir)..\\bin_$(Configuration)_$(Platform)\\docs\" mkdir \"$(SolutionDir)..\\bin_$(Configuration)_$(Platform)\\docs\"",
    "$(OutDir)man2html.exe < ..\\man\\bzadmin.6.in > \"$(SolutionDir)..\\bin_$(Configuration)_$(Platform)\\docs\\bzadmin.html\"",
    "$(OutDir)man2html.exe < ..\\man\\bzflag.6.in > \"$(SolutionDir)..\\bin_$(Configuration)_$(Platform)\\docs\\bzflag.html\"",
    "$(OutDir)man2html.exe < ..\\man\\bzfs.6.in > \"$(SolutionDir)..\\bin_$(Configuration)_$(Platform)\\docs\\bzfs.html\"",
    "$(OutDir)man2html.exe < ..\\man\\bzw.5.in > \"$(SolutionDir)..\\bin_$(Configuration)_$(Platform)\\docs\\bzw.html\""
  }

project "installer"
  kind "Utility"
  files "../../../buildsupport/windows/BZFlag.nsi"
  dependson { "bzflag", "bzfs", "bzadmin", "makehtml" }
  pluginDirNames = os.matchdirs("../../../plugins/*")
  for _, pluginDirName in ipairs(pluginDirNames) do
    local pluginName = string.sub(pluginDirName, 18, -1)
    if pluginName ~= "plugin_utils" then
      dependson(pluginName)
    end
  end
  filter "configurations:Release"
    postbuildcommands "\"$(ProgramFiles)\\nsis\\makensis.exe\" \"..\\build\\BZFlag.nsi\""
  filter { }
