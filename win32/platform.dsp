# Microsoft Developer Studio Project File - Name="platform" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=platform - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "platform.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "platform.mak" CFG="platform - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "platform - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "platform - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "platform - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib\Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\include" /I "..\win32" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"../src/platform/platform.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\src\platform\platform.lib"

!ELSEIF  "$(CFG)" == "platform - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib\Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\include" /I "..\win32" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"../src/platfform/Debug/platform.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\src\platform\Debug\platform.lib"

!ENDIF 

# Begin Target

# Name "platform - Win32 Release"
# Name "platform - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\platform\BzfDisplay.cxx
# End Source File
# Begin Source File

SOURCE=..\src\platform\BzfMedia.cxx
# End Source File
# Begin Source File

SOURCE=..\src\platform\BzfVisual.cxx
# End Source File
# Begin Source File

SOURCE=..\src\platform\BzfWindow.cxx
# End Source File
# Begin Source File

SOURCE=..\src\platform\PlatformFactory.cxx
# End Source File
# Begin Source File

SOURCE=..\src\platform\wave.cxx
# End Source File
# Begin Source File

SOURCE=..\src\platform\WinDisplay.cxx
# End Source File
# Begin Source File

SOURCE=..\src\platform\WinMedia.cxx
# End Source File
# Begin Source File

SOURCE=..\src\platform\WinPlatformFactory.cxx
# End Source File
# Begin Source File

SOURCE=..\src\platform\WinVisual.cxx
# End Source File
# Begin Source File

SOURCE=..\src\platform\WinWindow.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\BzfDisplay.h
# End Source File
# Begin Source File

SOURCE=..\include\BzfEvent.h
# End Source File
# Begin Source File

SOURCE=..\include\bzfgl.h
# End Source File
# Begin Source File

SOURCE=..\include\bzfio.h
# End Source File
# Begin Source File

SOURCE=..\include\BzfMedia.h
# End Source File
# Begin Source File

SOURCE=..\include\BzfString.h
# End Source File
# Begin Source File

SOURCE=..\include\BzfVisual.h
# End Source File
# Begin Source File

SOURCE=..\include\BzfWindow.h
# End Source File
# Begin Source File

SOURCE=..\include\common.h
# End Source File
# Begin Source File

SOURCE=..\include\ErrorHandler.h
# End Source File
# Begin Source File

SOURCE=..\include\OpenGLGState.h
# End Source File
# Begin Source File

SOURCE=..\include\Pack.h
# End Source File
# Begin Source File

SOURCE=..\include\PlatformFactory.h
# End Source File
# Begin Source File

SOURCE=..\include\resource.h
# End Source File
# Begin Source File

SOURCE=..\include\TimeKeeper.h
# End Source File
# Begin Source File

SOURCE=..\src\platform\wave.h
# End Source File
# Begin Source File

SOURCE=..\src\platform\WinDisplay.h
# End Source File
# Begin Source File

SOURCE=..\src\platform\WinMedia.h
# End Source File
# Begin Source File

SOURCE=..\src\platform\WinPlatformFactory.h
# End Source File
# Begin Source File

SOURCE=..\src\platform\WinVisual.h
# End Source File
# Begin Source File

SOURCE=..\src\platform\WinWindow.h
# End Source File
# End Group
# End Target
# End Project
