# Microsoft Developer Studio Project File - Name="geometry" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=geometry - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "geometry.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "geometry.mak" CFG="geometry - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "geometry - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "geometry - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "geometry - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /O2 /I "..\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "geometry - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I "..\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
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

# Name "geometry - Win32 Release"
# Name "geometry - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "hitank"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\geometry\models\hitank\barrel.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\src\geometry\models\hitank\body.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\src\geometry\models\hitank\ltread.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\src\geometry\models\hitank\rtread.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\src\geometry\models\hitank\turret.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "medtank"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\geometry\models\medtank\barrel.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\src\geometry\models\medtank\body.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\src\geometry\models\medtank\ltread.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\src\geometry\models\medtank\rtread.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\src\geometry\models\medtank\turret.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "lowtank"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\geometry\models\lowtank\barrel.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\src\geometry\models\lowtank\body.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\src\geometry\models\lowtank\ltread.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\src\geometry\models\lowtank\rtread.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\src\geometry\models\lowtank\turret.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Source File

SOURCE=..\src\geometry\BillboardSceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\BoltSceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\EighthDBoxSceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\EighthDimSceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\EighthDPyrSceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\FlagSceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\FlagWarpSceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\LaserSceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\PolyWallSceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\PTSceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\QuadWallSceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\Ray.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\SceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\ShellSceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\SphereSceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\TankSceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\TracerSceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\TriWallSceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\ViewFrustum.cxx
# End Source File
# Begin Source File

SOURCE=..\src\geometry\WallSceneNode.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\BillboardSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\BoltSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\EighthDBoxSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\EighthDimSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\EighthDPyrSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\FlagSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\FlagWarpSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\LaserSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\PolyWallSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\PTSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\QuadWallSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\Ray.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\ShellSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\ShotSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\SphereSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\TankSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\TracerSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\TriWallSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\ViewFrustum.h
# End Source File
# Begin Source File

SOURCE=..\include\WallSceneNode.h
# End Source File
# End Group
# End Target
# End Project
