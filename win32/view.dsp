# Microsoft Developer Studio Project File - Name="view" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=view - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "view.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "view.mak" CFG="view - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "view - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "view - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "view - Win32 Release"

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

!ELSEIF  "$(CFG)" == "view - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "view___Win32_Debug"
# PROP BASE Intermediate_Dir "view___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /vmg /GX /ZI /Od /I "..\include" /D "_LIB" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D VERSION=10801001 /Fd"Debug/view.pdb" /FD /GZ /c
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

# Name "view - Win32 Release"
# Name "view - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\view\View.cxx
# End Source File
# Begin Source File

SOURCE=..\src\view\ViewItemBuffer.cxx
# End Source File
# Begin Source File

SOURCE=..\src\view\ViewItemIf.cxx
# End Source File
# Begin Source File

SOURCE=..\src\view\ViewItemMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\src\view\ViewItemMessages.cxx
# End Source File
# Begin Source File

SOURCE=..\src\view\ViewItemModel.cxx
# End Source File
# Begin Source File

SOURCE=..\src\view\ViewItemScene.cxx
# End Source File
# Begin Source File

SOURCE=..\src\view\ViewItemScissor.cxx
# End Source File
# Begin Source File

SOURCE=..\src\view\ViewItemText.cxx
# End Source File
# Begin Source File

SOURCE=..\src\view\ViewItemViewport.cxx
# End Source File
# Begin Source File

SOURCE=..\src\view\ViewManager.cxx
# End Source File
# Begin Source File

SOURCE=..\src\view\ViewReader.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\BoundingBox.h
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

SOURCE=..\include\CallbackList.h
# End Source File
# Begin Source File

SOURCE=..\include\common.h
# End Source File
# Begin Source File

SOURCE=..\include\ConfigFileReader.h
# End Source File
# Begin Source File

SOURCE=..\include\ErrorHandler.h
# End Source File
# Begin Source File

SOURCE=..\include\FileManager.h
# End Source File
# Begin Source File

SOURCE=..\include\math3D.h
# End Source File
# Begin Source File

SOURCE=..\include\mathr.h
# End Source File
# Begin Source File

SOURCE=..\include\Menu.h
# End Source File
# Begin Source File

SOURCE=..\include\MenuManager.h
# End Source File
# Begin Source File

SOURCE=..\include\MessageBuffer.h
# End Source File
# Begin Source File

SOURCE=..\include\MessageManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OpenGLGState.h
# End Source File
# Begin Source File

SOURCE=..\include\OpenGLTexFont.h
# End Source File
# Begin Source File

SOURCE=..\include\OpenGLTexture.h
# End Source File
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

SOURCE=..\include\SceneNodeParticleSystem.h
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

SOURCE=..\include\SceneVisitorBaseRender.h
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
# Begin Source File

SOURCE=..\include\StateDatabase.h
# End Source File
# Begin Source File

SOURCE=..\include\TimeKeeper.h
# End Source File
# Begin Source File

SOURCE=..\include\View.h
# End Source File
# Begin Source File

SOURCE=..\include\ViewFrustum.h
# End Source File
# Begin Source File

SOURCE=..\include\ViewItemBuffer.h
# End Source File
# Begin Source File

SOURCE=..\include\ViewItemIf.h
# End Source File
# Begin Source File

SOURCE=..\include\ViewItemMenu.h
# End Source File
# Begin Source File

SOURCE=..\include\ViewItemMessages.h
# End Source File
# Begin Source File

SOURCE=..\include\ViewItemModel.h
# End Source File
# Begin Source File

SOURCE=..\include\ViewItemScene.h
# End Source File
# Begin Source File

SOURCE=..\include\ViewItemScissor.h
# End Source File
# Begin Source File

SOURCE=..\include\ViewItemText.h
# End Source File
# Begin Source File

SOURCE=..\include\ViewItemViewport.h
# End Source File
# Begin Source File

SOURCE=..\include\ViewManager.h
# End Source File
# Begin Source File

SOURCE=..\include\ViewReader.h
# End Source File
# Begin Source File

SOURCE=..\include\XMLTree.h
# End Source File
# End Group
# End Target
# End Project
