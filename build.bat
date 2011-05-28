
@rem  Script to generate a premake4 executable, and then run it from the
@rem  current directory to generate the MSVC project files for BZFlag.

@set COMPILER=MSBuild.exe /nologo

%COMPILER% "other_src\premake\build\vs2008\Premake4.sln"

"other_src\premake\bin\Release\premake4.exe vs2008"

%COMPILER% "build_vs2008\bz.sln"
