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
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /vmg /GX /O2 /I "..\include" /I "..\src\game" /D "NDEBUG" /D "_WINDOWS" /D "WIN32" /D "_MBCS" /D VERSION=10801001 /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 vorbis_static.lib ogg_static.lib vorbisenc_static.lib vorbisfile_static.lib dsound.lib winmm.lib wsock32.lib glu32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /map /machine:I386

!ELSEIF  "$(CFG)" == "bzflag - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "bzflag___Win32_Debug"
# PROP BASE Intermediate_Dir "bzflag___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin\Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /vmg /GX /ZI /Od /I "..\include" /I "..\src\game" /D "_WINDOWS" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D VERSION=10801001 /Fd"Debug/bzflag.pdb" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 vorbis_static.lib ogg_static.lib vorbisenc_static.lib vorbisfile_static.lib dsound.lib winmm.lib wsock32.lib glu32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "bzflag - Win32 Release"
# Name "bzflag - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\bzflag\bzflag.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\bzflag.rc
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\CommandsSearch.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\DeadPlayer.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\HUDManager.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\LocalPlayer.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\Player.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\playing.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\RemotePlayer.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\SceneBuilder.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ServerLink.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ShotPath.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ShotStrategy.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\SoundManager.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ViewItemHUD.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ViewItemPlayerScene.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ViewItemPWR.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ViewItemRadar.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ViewItems.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ViewItemScoreboard.cxx
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\World.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\bzflag\CommandsSearch.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\daylight.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\DeadPlayer.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\HUDManager.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\LocalPlayer.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\Player.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\PlayerLink.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\playing.h
# End Source File
# Begin Source File

SOURCE=..\src\game\Protocol.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\Region.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\RemotePlayer.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\RobotPlayer.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\SceneBuilder.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\SceneManager.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ServerLink.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ShotPath.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ShotStrategy.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\sound.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\SoundManager.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ViewItemHUD.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ViewItemPlayerScene.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ViewItemPWR.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ViewItemRadar.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ViewItems.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\ViewItemScoreboard.h
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\World.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\src\bzflag\bzflag.bmp
# End Source File
# Begin Source File

SOURCE=..\src\bzflag\bzflag.ico
# End Source File
# End Group
# End Target
# End Project
