# Microsoft Developer Studio Project File - Name="installer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=installer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE
!MESSAGE NMAKE /f "installer.mak".
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "installer.mak" CFG="installer - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "installer - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "installer - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
MTL=midl.exe

!IF  "$(CFG)" == "installer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "installer___Win32_Release"
# PROP BASE Intermediate_Dir "installer___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "installer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "installer___Win32_Debug"
# PROP BASE Intermediate_Dir "installer___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""

!ENDIF

# Begin Target

# Name "installer - Win32 Release"
# Name "installer - Win32 Debug"
# Begin Source File

SOURCE=..\..\package\win32\nsis\BZFlag.nsi

!IF  "$(CFG)" == "installer - Win32 Release"

USERDEP__BZFLA="..\..\src\bzfs\bzfs.exe"	"..\..\src\bzflag\bzflag.exe"	"..\..\src\bzadmin\bzadmin.exe"
# Begin Custom Build - Building installer using $(InputPath)
InputDir=\bz\bz_cvs\bzflag\package\win32\nsis
InputPath=..\..\package\win32\nsis\BZFlag.nsi

"..\..\dist\bzflag1.11.0.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir)
	makensis $(InputDir)\BZFlag

# End Custom Build

!ELSEIF  "$(CFG)" == "installer - Win32 Debug"

USERDEP__BZFLA="..\..\src\bzfs\debug\bzfs.exe"	"..\..\src\bzflag\debug\bzflag.exe"	"..\..\src\bzadmin\debug\bzadmin.exe"
# Begin Custom Build - Building installer using $(InputPath)
InputDir=\bz\bz_cvs\bzflag\package\win32\nsis
InputPath=..\..\package\win32\nsis\BZFlag.nsi

"..\..\dist\bzflag1.11.0.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir)
	makensis $(InputDir)\BZFlag

# End Custom Build

!ENDIF

# End Source File
# Begin Source File

SOURCE=.\NudgeDate.txt
# End Source File
# End Target
# End Project
