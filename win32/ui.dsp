# Microsoft Developer Studio Project File - Name="ui" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ui - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ui.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ui.mak" CFG="ui - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ui - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ui - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /vmg /GX /O2 /I "..\include" /D "NDEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /D VERSION=10801001 /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ui___Win32_Debug"
# PROP BASE Intermediate_Dir "ui___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /vmg /GX /ZI /Od /I "..\include" /D "_LIB" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D VERSION=10801001 /Fd"Debug/ui.pdb" /FD /GZ /c
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

# Name "ui - Win32 Release"
# Name "ui - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\ui\CommandManager.cxx
# End Source File
# Begin Source File

SOURCE=..\src\ui\CommandsStandard.cxx
# End Source File
# Begin Source File

SOURCE=..\src\ui\KeyManager.cxx
# End Source File
# Begin Source File

SOURCE=..\src\ui\Menu.cxx
# End Source File
# Begin Source File

SOURCE=..\src\ui\MenuControls.cxx
# End Source File
# Begin Source File

SOURCE=..\src\ui\MenuManager.cxx
# End Source File
# Begin Source File

SOURCE=..\src\ui\MenuReader.cxx
# End Source File
# Begin Source File

SOURCE=..\src\ui\MessageBuffer.cxx
# End Source File
# Begin Source File

SOURCE=..\src\ui\MessageManager.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\bzflag\CommandManager.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\CommandsStandard.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\KeyManager.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\Menu.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\MenuControls.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\MenuManager.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\MenuReader.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\MessageBuffer.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\MessageManager.h
# End Source File
# End Group
# End Target
# End Project
