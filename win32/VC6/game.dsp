# Microsoft Developer Studio Project File - Name="game" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=game - Win32 SDL_Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "game.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "game.mak" CFG="game - Win32 SDL_Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "game - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "game - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "game - Win32 SDL_Release" (based on "Win32 (x86) Static Library")
!MESSAGE "game - Win32 SDL_Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "game - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\src\game"
# PROP Intermediate_Dir "..\..\src\game\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\include" /I "..\..\win32" /I ".\\" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\src\game\Debug"
# PROP Intermediate_Dir "..\..\src\game\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\include" /I "..\..\win32" /I ".\\" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "game - Win32 SDL_Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "game___Win32_SDL_Release"
# PROP BASE Intermediate_Dir "game___Win32_SDL_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\src\game\SDL_Release"
# PROP Intermediate_Dir "..\..\src\game\SDL_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\..\include" /I "..\..\win32" /I ".\\" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\include" /I "..\..\win32" /I ".\\" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_SDL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "game - Win32 SDL_Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "game___Win32_SDL_Debug"
# PROP BASE Intermediate_Dir "game___Win32_SDL_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\src\game\SDL_Debug"
# PROP Intermediate_Dir "..\..\src\game\SDL_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\include" /I "..\..\win32" /I ".\\" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\include" /I "..\..\win32" /I ".\\" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_SDL" /FD /GZ /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "game - Win32 Release"
# Name "game - Win32 Debug"
# Name "game - Win32 SDL_Release"
# Name "game - Win32 SDL_Debug"
# Begin Group "Source Files"

# PROP Default_Filter "c;cxx"
# Begin Source File

SOURCE=..\..\src\game\CommandsStandard.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\game\CollisionManager.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\game\DelayQueue.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\game\DirectoryNames.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\game\Frustum.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\game\Intersect.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\game\LagInfo.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\game\NetHandler.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\game\PlayerInfo.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\game\Ray.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\game\VotingArbiter.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\..\src\game\CommandsStandard.h
# End Source File
# Begin Source File

SOURCE=..\..\include\DelayQueue.h
# End Source File
# Begin Source File

SOURCE=..\..\include\DirectoryNames.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Frustum.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Intersect.h
# End Source File
# Begin Source File

SOURCE=..\..\include\LagInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\include\NetHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\include\PlayerInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\include\VotingArbiter.h
# End Source File
# End Group
# End Target
# End Project
