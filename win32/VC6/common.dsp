# Microsoft Developer Studio Project File - Name="common" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=common - Win32 SDL_Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "common.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "common.mak" CFG="common - Win32 SDL_Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "common - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "common - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "common - Win32 SDL_Release" (based on "Win32 (x86) Static Library")
!MESSAGE "common - Win32 SDL_Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "common - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\src\common"
# PROP Intermediate_Dir "..\..\src\common\Release"
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

!ELSEIF  "$(CFG)" == "common - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\src\common\Debug"
# PROP Intermediate_Dir "..\..\src\common\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\include" /I "..\..\win32" /I ".\\" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "common - Win32 SDL_Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "common___Win32_SDL_Release"
# PROP BASE Intermediate_Dir "common___Win32_SDL_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\src\common\SDL_Release"
# PROP Intermediate_Dir "..\..\src\common\SDL_Release"
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

!ELSEIF  "$(CFG)" == "common - Win32 SDL_Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "common___Win32_SDL_Debug"
# PROP BASE Intermediate_Dir "common___Win32_SDL_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\src\common\SDL_Debug"
# PROP Intermediate_Dir "..\..\src\common\SDL_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\include" /I "..\..\win32" /I ".\\" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\include" /I "..\..\win32" /I ".\\" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_SDL" /FD /GZ /c
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

# Name "common - Win32 Release"
# Name "common - Win32 Debug"
# Name "common - Win32 SDL_Release"
# Name "common - Win32 SDL_Debug"
# Begin Group "Source Files"

# PROP Default_Filter "c;cxx"
# Begin Source File

SOURCE=..\..\src\common\Bundle.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\BundleMgr.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\bz_Locale.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\BZDBCache.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\bzfio.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\bzsignal.c
# End Source File
# Begin Source File

SOURCE=..\..\src\common\CommandManager.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\ConfigFileManager.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\Country.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\ErrorHandler.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\FileManager.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\Flag.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\global.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\KeyManager.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\Language.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\md5.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\PlayerState.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\ShotUpdate.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\StateDatabase.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\Team.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\TimeBomb.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\TimeKeeper.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\VotingBooth.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\common\WordFilter.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\..\include\Address.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Bundle.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BundleMgr.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bz_Locale.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BZDBCache.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BzfEvent.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bzfio.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bzsignal.h
# End Source File
# Begin Source File

SOURCE=..\..\include\CallbackList.h
# End Source File
# Begin Source File

SOURCE=..\..\include\CommandManager.h
# End Source File
# Begin Source File

SOURCE=..\..\include\common.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ConfigFileManager.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Country.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ErrorHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\include\FileManager.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Flag.h
# End Source File
# Begin Source File

SOURCE=..\..\include\global.h
# End Source File
# Begin Source File

SOURCE=..\..\include\KeyManager.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Language.h
# End Source File
# Begin Source File

SOURCE=..\..\include\md5.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Pack.h
# End Source File
# Begin Source File

SOURCE=..\..\include\PlayerState.h
# End Source File
# Begin Source File

SOURCE=..\..\include\regex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SceneDatabase.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ShotUpdate.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Singleton.h
# End Source File
# Begin Source File

SOURCE=..\..\include\StateDatabase.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Team.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TextUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TimeBomb.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TimeKeeper.h
# End Source File
# Begin Source File

SOURCE=..\..\include\version.h
# End Source File
# Begin Source File

SOURCE=..\..\include\VotingBooth.h
# End Source File
# Begin Source File

SOURCE=..\..\include\win32.h
# End Source File
# Begin Source File

SOURCE=..\..\include\WordFilter.h
# End Source File
# End Group
# End Target
# End Project
