# Microsoft Developer Studio Project File - Name="collision" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=collision - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "collision.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "collision.mak" CFG="collision - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "collision - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "collision - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "collision - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "collision___Win32_Release"
# PROP BASE Intermediate_Dir "collision___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "collision - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "collision___Win32_Debug"
# PROP BASE Intermediate_Dir "collision___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
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

# Name "collision - Win32 Release"
# Name "collision - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\collision\Body.cxx
# End Source File
# Begin Source File

SOURCE=..\src\collision\BodyManager.cxx
# End Source File
# Begin Source File

SOURCE=..\src\collision\BodyODEAssistant.cxx
# End Source File
# Begin Source File

SOURCE=..\src\collision\CollisionDetectorGJK.cxx
# End Source File
# Begin Source File

SOURCE=..\src\collision\ContactPoint.cxx
# End Source File
# Begin Source File

SOURCE=..\src\collision\ContactSolverBaraffNoFriction.cxx
# End Source File
# Begin Source File

SOURCE=..\src\collision\ContactSurfaceIntersector.cxx
# End Source File
# Begin Source File

SOURCE=..\src\collision\Trackball.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\collision\Body.h
# End Source File
# Begin Source File

SOURCE=..\src\collision\BodyManager.h
# End Source File
# Begin Source File

SOURCE=..\src\collision\BodyODEAssistant.h
# End Source File
# Begin Source File

SOURCE=..\src\collision\CollisionDetector.h
# End Source File
# Begin Source File

SOURCE=..\src\collision\CollisionDetectorGJK.h
# End Source File
# Begin Source File

SOURCE=..\src\collision\ContactPoint.h
# End Source File
# Begin Source File

SOURCE=..\src\collision\ContactSolver.h
# End Source File
# Begin Source File

SOURCE=..\src\collision\ContactSolverBaraffNoFriction.h
# End Source File
# Begin Source File

SOURCE=..\src\collision\ContactSurfaceIntersector.h
# End Source File
# Begin Source File

SOURCE=..\src\collision\ContactSurfaceIntersectorPolygon.h
# End Source File
# Begin Source File

SOURCE=..\include\ms_minmax.h
# End Source File
# Begin Source File

SOURCE=..\src\collision\Trackball.h
# End Source File
# End Group
# End Target
# End Project
