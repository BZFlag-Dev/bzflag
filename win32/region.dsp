# Microsoft Developer Studio Project File - Name="region" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=region - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "region.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "region.mak" CFG="region - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "region - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "region - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "region - Win32 Release"

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
# ADD CPP /nologo /W3 /vmg /GX /O2 /I "..\include" /I "..\src\game" /D "NDEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /D VERSION=10801001 /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "region - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "region___Win32_Debug"
# PROP BASE Intermediate_Dir "region___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /vmg /GX /ZI /Od /I "..\include" /I "..\src\game" /D "_LIB" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D VERSION=10801001 /Fd"Debug/region.pdb" /FD /GZ /c
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

# Name "region - Win32 Release"
# Name "region - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\region\Region.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzfs\RegionManagerBase.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzfs\RegionManagerFlagSpawn.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzfs\RegionManagerObstacle.cxx
# End Source File
# Begin Source File

SOURCE=..\src\region\RegionReader.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzfs\RegionReaderBase.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzfs\RegionReaderFlagSpawn.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzfs\RegionReaderObstacle.cxx
# End Source File
# Begin Source File

SOURCE=..\src\region\RegionShape.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\Address.h
# End Source File
# Begin Source File

SOURCE=..\include\bzfio.h
# End Source File
# Begin Source File

SOURCE=..\include\CallbackList.h
# End Source File
# Begin Source File

SOURCE=..\include\CollisionDetector.h
# End Source File
# Begin Source File

SOURCE=..\include\CollisionDetectorGJK.h
# End Source File
# Begin Source File

SOURCE=..\include\common.h
# End Source File
# Begin Source File

SOURCE=..\include\ConfigFileReader.h
# End Source File
# Begin Source File

SOURCE=..\src\game\Flag.h
# End Source File
# Begin Source File

SOURCE=..\src\game\global.h
# End Source File
# Begin Source File

SOURCE=..\include\math3D.h
# End Source File
# Begin Source File

SOURCE=..\include\mathr.h
# End Source File
# Begin Source File

SOURCE=..\include\Pack.h
# End Source File
# Begin Source File

SOURCE=..\include\Region.h
# End Source File
# Begin Source File

SOURCE=..\src\bzfs\RegionManagerBase.h
# End Source File
# Begin Source File

SOURCE=..\src\bzfs\RegionManagerFlagSpawn.h
# End Source File
# Begin Source File

SOURCE=..\src\bzfs\RegionManagerObstacle.h
# End Source File
# Begin Source File

SOURCE=..\include\RegionReader.h
# End Source File
# Begin Source File

SOURCE=..\src\bzfs\RegionReaderBase.h
# End Source File
# Begin Source File

SOURCE=..\src\bzfs\RegionReaderFlagSpawn.h
# End Source File
# Begin Source File

SOURCE=..\src\bzfs\RegionReaderObstacle.h
# End Source File
# Begin Source File

SOURCE=..\include\RegionShape.h
# End Source File
# Begin Source File

SOURCE=..\include\Shape.h
# End Source File
# Begin Source File

SOURCE=..\include\ShapeBox.h
# End Source File
# Begin Source File

SOURCE=..\include\ShapePoint.h
# End Source File
# Begin Source File

SOURCE=..\include\ShapePyramid.h
# End Source File
# Begin Source File

SOURCE=..\include\StateDatabase.h
# End Source File
# Begin Source File

SOURCE=..\src\game\Team.h
# End Source File
# Begin Source File

SOURCE=..\include\TransformableShape.h
# End Source File
# Begin Source File

SOURCE=..\include\TransformedShape.h
# End Source File
# Begin Source File

SOURCE=..\include\XMLTree.h
# End Source File
# End Group
# End Target
# End Project
