# Microsoft Developer Studio Project File - Name="bzflag" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=bzflag - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bzflag.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bzflag.mak" CFG="bzflag - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bzflag - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "bzflag - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bzflag - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\src\bzflag"
# PROP Intermediate_Dir "..\..\src\bzflag\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\include" /I "..\..\win32" /I ".\\" /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /fo"..\..\src\bzflag\bzflag.res" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ws2_32.lib dsound.lib winmm.lib glu32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /pdb:"../src/bzflag/bzflag.pdb" /machine:I386
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\..\src\bzflag\*.exe ..\..\*.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "bzflag - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\src\bzflag\Debug"
# PROP Intermediate_Dir "..\..\src\bzflag\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\include" /I "..\..\win32" /I ".\\" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib dsound.lib winmm.lib glu32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\..\src\bzflag\debug\*.exe ..\..\*.exe	copy ..\..\src\bzflag\debug\*.pdb ..\..\*.pdb
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "bzflag - Win32 Release"
# Name "bzflag - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\bzflag\ActionBinding.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\BackgroundRenderer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\bzflag.cxx
# End Source File
# Begin Source File

SOURCE=..\bzflag.rc
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\callbacks.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\CommandsStandard.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ControlPanel.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\daylight.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDRenderer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDui.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\LocalPlayer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\MainWindow.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\menus.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\Player.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\playing.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\RadarRenderer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\Region.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\RemotePlayer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\RobotPlayer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\SceneBuilder.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\SceneRenderer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerLink.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerListCache.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ShotPath.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ShotStrategy.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\sound.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\stars.cxx
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\texture.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\TextureManager.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\World.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\WorldPlayer.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\bzflag\ActionBinding.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Address.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\BackgroundRenderer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BaseBuilding.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BillboardSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BoltSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BoxBuilding.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BSPSceneDatabase.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Bundle.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BundleMgr.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BZDBCache.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BzfDisplay.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BzfEvent.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bzfgl.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bzfio.h
# End Source File
# Begin Source File

SOURCE=..\bzflag.ico
# End Source File
# Begin Source File

SOURCE=..\..\include\BzfMedia.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BzfVisual.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BzfWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\include\CallbackList.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\callbacks.h
# End Source File
# Begin Source File

SOURCE=..\..\include\CommandManager.h
# End Source File
# Begin Source File

SOURCE=..\..\include\CommandsStandard.h
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

SOURCE=..\..\src\bzflag\ControlPanel.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\daylight.h
# End Source File
# Begin Source File

SOURCE=..\..\include\EighthDBaseSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\EighthDBoxSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\EighthDimSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\EighthDPyrSceneNode.h
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

SOURCE=..\..\src\bzflag\HUDDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDRenderer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDui.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Intersect.h
# End Source File
# Begin Source File

SOURCE=..\..\include\KeyManager.h
# End Source File
# Begin Source File

SOURCE=..\..\include\LaserSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\LocalPlayer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\MainWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\include\MediaFile.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\menus.h
# End Source File
# Begin Source File

SOURCE=..\..\include\multicast.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Obstacle.h
# End Source File
# Begin Source File

SOURCE=..\..\include\OpenGLDisplayList.h
# End Source File
# Begin Source File

SOURCE=..\..\include\OpenGLGState.h
# End Source File
# Begin Source File

SOURCE=..\..\include\OpenGLLight.h
# End Source File
# Begin Source File

SOURCE=..\..\include\OpenGLMaterial.h
# End Source File
# Begin Source File

SOURCE=..\..\include\OpenGLTexFont.h
# End Source File
# Begin Source File

SOURCE=..\..\include\OpenGLTexture.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Pack.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ping.h
# End Source File
# Begin Source File

SOURCE=..\..\include\PlatformFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\Player.h
# End Source File
# Begin Source File

SOURCE=..\..\include\PlayerState.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\playing.h
# End Source File
# Begin Source File

SOURCE="C:\Program Files\Microsoft SDK\include\PropIdl.h"
# End Source File
# Begin Source File

SOURCE=..\include\Protocol.h
# End Source File
# Begin Source File

SOURCE=..\..\include\PyramidBuilding.h
# End Source File
# Begin Source File

SOURCE=..\..\include\QuadWallSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\RadarRenderer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ray.h
# End Source File
# Begin Source File

SOURCE="C:\Program Files\Microsoft SDK\include\Reason.h"
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\Region.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\RemotePlayer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\RenderNode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\RobotPlayer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\SceneBuilder.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SceneDatabase.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SceneNode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SceneRenderer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerLink.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerListCache.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ShotPath.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ShotSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ShotStrategy.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ShotUpdate.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Singleton.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\sound.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SphereSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\StateDatabase.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TankSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Team.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Teleporter.h
# End Source File
# Begin Source File

SOURCE=..\..\include\texture.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TextureManager.h
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

SOURCE=..\include\version.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ViewFrustum.h
# End Source File
# Begin Source File

SOURCE=..\..\include\WallObstacle.h
# End Source File
# Begin Source File

SOURCE=..\..\include\WallSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\WordFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\World.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\WorldPlayer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ZSceneDatabase.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\bzflag.ico
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\include\bzsignal.h
# End Source File
# Begin Source File

SOURCE=..\config.h
# End Source File
# Begin Source File

SOURCE=..\..\include\FlagSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\FlagWarpSceneNode.h
# End Source File
# End Target
# End Project
