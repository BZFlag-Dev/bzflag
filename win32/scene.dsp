# Microsoft Developer Studio Project File - Name="scene" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=scene - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "scene.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "scene.mak" CFG="scene - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "scene - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "scene - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "scene - Win32 Release"

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

!ELSEIF  "$(CFG)" == "scene - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "scene___Win32_Debug"
# PROP BASE Intermediate_Dir "scene___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /vmg /GX /ZI /Od /I "..\include" /D "_LIB" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D VERSION=10801001 /Fd"Debug/scene.pdb" /FD /GZ /c
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

# Name "scene - Win32 Release"
# Name "scene - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\scene\SceneNode.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneNodeAnimate.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneNodeBaseTransform.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneNodeBillboard.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneNodeChoice.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneNodeGeometry.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneNodeGroup.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneNodeGState.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneNodeLight.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneNodeLOD.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneNodeMatrixTransform.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneNodeMetadata.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneNodeParameters.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneNodePrimitive.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneNodeSelector.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneNodeSphereTransform.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneNodeTransform.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneReader.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneVisitor.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneVisitorFind.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneVisitorFindAll.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneVisitorParams.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneVisitorRender.cxx
# End Source File
# Begin Source File

SOURCE=..\src\scene\SceneVisitorSimpleRender.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\SceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNodeAnimate.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNodeBaseTransform.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNodeBillboard.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNodeChoice.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNodeGeometry.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNodeGroup.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNodeGState.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNodeLight.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNodeLOD.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNodeMatrixTransform.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNodeMetadata.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNodeParameters.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNodePrimitive.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNodes.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNodeSelector.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNodeSphereTransform.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneNodeTransform.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneReader.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneVisitor.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneVisitorFind.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneVisitorFindAll.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneVisitorParams.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneVisitorRender.h
# End Source File
# Begin Source File

SOURCE=..\include\SceneVisitorSimpleRender.h
# End Source File
# End Group
# End Target
# End Project
