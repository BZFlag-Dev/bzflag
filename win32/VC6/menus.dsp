# Microsoft Developer Studio Project File - Name="menus" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=menus - Win32 SDL_Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "menus.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "menus.mak" CFG="menus - Win32 SDL_Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "menus - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "menus - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "menus - Win32 SDL_Release" (based on "Win32 (x86) Static Library")
!MESSAGE "menus - Win32 SDL_Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "menus - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\src\bzflag"
# PROP Intermediate_Dir "..\..\src\bzflag\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "./" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "menus - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\src\bzflag\Debug"
# PROP Intermediate_Dir "..\..\src\bzflag\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "./" /I "../../include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "menus - Win32 SDL_Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "menus___Win32_SDL_Release"
# PROP BASE Intermediate_Dir "menus___Win32_SDL_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "menus___Win32_SDL_Release"
# PROP Intermediate_Dir "menus___Win32_SDL_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "./" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "./" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "menus - Win32 SDL_Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "menus___Win32_SDL_Debug"
# PROP BASE Intermediate_Dir "menus___Win32_SDL_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\src\menus\SDL_Debug"
# PROP Intermediate_Dir "..\..\src\menus\SDL_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "./" /I "../../include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "./" /I "../../include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_SDL" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "menus - Win32 Release"
# Name "menus - Win32 Debug"
# Name "menus - Win32 SDL_Release"
# Name "menus - Win32 SDL_Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\bzflag\FormatMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\GUIOptionsMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HelpMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDDialogStack.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\JoinMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\KeyboardMapMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\MainMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\MenuDefaultKey.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\OptionsMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\QuickKeysMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\QuitMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\SaveWorldMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerItem.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerStartMenu.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\common.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\FormatMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\GUIOptionsMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HelpMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDDialogStack.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\JoinMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\KeyboardMapMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ListServer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\MainMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\MenuDefaultKey.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\OptionsMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\QuickKeysMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\QuitMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\SaveWorldMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerItem.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerStartMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\include\win32.h
# End Source File
# End Group
# End Target
# End Project
