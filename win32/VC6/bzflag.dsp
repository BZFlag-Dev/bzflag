# Microsoft Developer Studio Project File - Name="bzflag" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=bzflag - Win32 SDL_Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bzflag.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bzflag.mak" CFG="bzflag - Win32 SDL_Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bzflag - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "bzflag - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "bzflag - Win32 SDL_Release" (based on "Win32 (x86) Application")
!MESSAGE "bzflag - Win32 SDL_Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\include" /I "..\..\win32" /I ".\\" /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /FD /c
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
# ADD LINK32 sdlmain.lib sdl.lib ws2_32.lib dsound.lib winmm.lib glu32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /pdb:"../src/bzflag/bzflag.pdb" /machine:I386 /nodefaultlib:"LIBCMT"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE=$(InputPath)
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
# ADD BASE CPP /nologo /W3 /GX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /Zi /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Od /I "..\..\include" /I "..\..\win32" /I ".\\" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /FD /Zi /GZ /c
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
# ADD LINK32 sdlmain.lib sdl.lib ws2_32.lib dsound.lib winmm.lib glu32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCMT" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=copy ..\..\src\bzflag\debug\*.exe ..\..\*.exe	copy\
  ..\..\src\bzflag\debug\*.pdb ..\..\*.pdb
# End Special Build Tool

!ELSEIF  "$(CFG)" == "bzflag - Win32 SDL_Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\..\src\bzflag\SDL_Release"
# PROP BASE Intermediate_Dir "..\..\src\bzflag\SDL_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\src\bzflag\SDL_Release"
# PROP Intermediate_Dir "..\..\src\bzflag\SDL_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\..\include" /I "..\..\win32" /I ".\\" /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\include" /I "..\..\win32" /I ".\\" /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "HAVE_SDL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /fo"..\..\src\bzflag\bzflag.res" /d "NDEBUG"
# ADD RSC /l 0x409 /fo"..\..\src\bzflag\bzflag.res" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 sdlmain.lib sdl.lib ws2_32.lib dsound.lib winmm.lib glu32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /pdb:"../src/bzflag/bzflag.pdb" /machine:I386 /nodefaultlib:"LIBCMT"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 sdlmain.lib sdl.lib ws2_32.lib dsound.lib winmm.lib glu32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /pdb:"../src/bzflag/bzflag.pdb" /machine:I386 /nodefaultlib:"LIBCMT"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=copy ..\..\src\bzflag\SDL_Release\*.exe ..\..\*.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "bzflag - Win32 SDL_Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "bzflag___Win32_SDL_Debug0"
# PROP BASE Intermediate_Dir "bzflag___Win32_SDL_Debug0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\src\bzflag\SDL_Debug"
# PROP Intermediate_Dir "..\..\src\bzflag\SDL_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GX /Od /I "..\..\include" /I "..\..\win32" /I ".\\" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /FD /Zi /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /GX /Od /I "..\..\include" /I "..\..\win32" /I ".\\" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "HAVE_SDL" /FD /Zi /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 sdlmain.lib sdl.lib ws2_32.lib dsound.lib winmm.lib glu32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCMT" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 sdlmain.lib sdl.lib ws2_32.lib dsound.lib winmm.lib glu32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCMT" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=copy ..\..\src\bzflag\SDL_debug\*.exe ..\..\*.exe	copy\
  ..\..\src\bzflag\SDL_debug\*.pdb ..\..\*.pdb
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "bzflag - Win32 Release"
# Name "bzflag - Win32 Debug"
# Name "bzflag - Win32 SDL_Release"
# Name "bzflag - Win32 SDL_Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "MenuSources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\bzflag\AudioMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\DisplayMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\FormatMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\GUIOptionsMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HelpMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDDialogStack.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\InputMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\JoinMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\KeyboardMapMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\MainMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\MenuDefaultKey.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\OptionsMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\QuickKeysMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\QuitMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\SaveWorldMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerItem.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerList.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerStartMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ShotStats.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ShotStatsDefaultKey.cxx
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\src\bzflag\ActionBinding.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\AutoPilot.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\BackgroundRenderer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\BaseLocalPlayer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\bzflag.cxx
# End Source File
# Begin Source File

SOURCE=..\bzflag.rc

!IF  "$(CFG)" == "bzflag - Win32 Release"

!ELSEIF  "$(CFG)" == "bzflag - Win32 Debug"

!ELSEIF  "$(CFG)" == "bzflag - Win32 SDL_Release"

!ELSEIF  "$(CFG)" == "bzflag - Win32 SDL_Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\callbacks.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\clientCommands.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ComposeDefaultKey.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ControlPanel.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\daylight.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\FlashClock.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\GuidedMissleStrategy.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDRenderer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDui.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDuiControl.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDuiDefaultKey.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDuiLabel.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDuiList.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDuiTextureLabel.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDuiTypeIn.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\LocalPlayer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\MainWindow.cxx
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

SOURCE=..\..\src\bzflag\RegionPriorityQueue.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\RemotePlayer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\RobotPlayer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\Roster.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\SceneBuilder.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\SceneRenderer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\SegmentedShotStrategy.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerCommandKey.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerLink.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerListCache.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ShockWaveStrategy.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ShotPath.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ShotPathSegment.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ShotStrategy.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ShotStatistics.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\SilenceDefaultKey.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\sound.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\stars.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\TargetingUtils.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\World.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\WorldBuilder.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\WorldPlayer.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Menu Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\bzflag\AudioMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\FormatMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\GUIOptionsMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HelpMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\InputMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\JoinMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\KeyboardMapMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\MainMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\MenuDefaultKey.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\OptionsMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\QuickKeysMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\QuitMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\SaveWorldMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerItem.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerList.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerStartMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ShotStats.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ShotStatsDefaultKey.h
# End Source File
# End Group
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

SOURCE=..\..\src\bzflag\BaseLocalPlayer.h
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

SOURCE=..\..\include\BzfJoystick.h
# End Source File
# Begin Source File

SOURCE=..\bzflag.ico
# End Source File
# Begin Source File

SOURCE=..\..\include\BzfMedia.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bzfSDL.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BzfVisual.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BzfWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bzsignal.h
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

SOURCE=..\..\src\bzflag\commands.h
# End Source File
# Begin Source File

SOURCE=..\..\include\CommandsStandard.h
# End Source File
# Begin Source File

SOURCE=..\..\include\common.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ComposeDefaultKey.h
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

SOURCE=..\..\include\DisplayListManager.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\DisplayMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\include\DrawablesManager.h
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

SOURCE=..\..\src\bzflag\EntryZone.h
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

SOURCE=..\..\include\FlagSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\FlagWarpSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\FlashClock.h
# End Source File
# Begin Source File

SOURCE=..\..\include\global.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\GuidedMissleStrategy.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDDialogStack.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDRenderer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDui.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDuiControl.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDuiDefaultKey.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDuiLabel.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDuiList.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDuiTextureLabel.hx
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\HUDuiTypeIn.h
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

SOURCE=..\..\src\bzflag\ListServer.h
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

SOURCE=..\..\include\multicast.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Obstacle.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ObstacleSceneNodeGenerator.h
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

SOURCE=..\..\src\bzflag\Region.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\RegionPriorityQueue.h
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

SOURCE=..\..\src\bzflag\Roster.h
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

SOURCE=..\..\src\bzflag\SegmentedShotStrategy.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerCommandKey.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerLink.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ServerListCache.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ShockWaveStrategy.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ShotPath.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\ShotPathSegment.h
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

SOURCE=..\..\src\bzflag\ShotStatistics.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\SilenceDefaultKey.h
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

SOURCE=..\..\src\bzflag\stars.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\StartupInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\include\StateDatabase.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TankSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\TargetingUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Team.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Teleporter.h
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

SOURCE=..\..\include\ViewFrustum.h
# End Source File
# Begin Source File

SOURCE=..\..\include\WallObstacle.h
# End Source File
# Begin Source File

SOURCE=..\..\include\WallSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\Weapon.h
# End Source File
# Begin Source File

SOURCE=..\..\include\win32.h
# End Source File
# Begin Source File

SOURCE=..\..\include\WordFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\World.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\WorldBuilder.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bzflag\WorldPlayer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\zlib\zconf.h
# End Source File
# Begin Source File

SOURCE=..\..\src\zlib\zlib.h
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
# End Target
# End Project
