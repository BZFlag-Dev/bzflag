# Microsoft Developer Studio Project File - Name="runmakedb" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=runmakedb - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "runmakedb.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "runmakedb.mak" CFG="runmakedb - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "runmakedb - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "runmakedb - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
MTL=midl.exe

!IF  "$(CFG)" == "runmakedb - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "runmakedb___Win32_Release"
# PROP BASE Intermediate_Dir "runmakedb___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "runmakedb - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "runmakedb___Win32_Debug"
# PROP BASE Intermediate_Dir "runmakedb___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "runmakedb - Win32 Release"
# Name "runmakedb - Win32 Debug"
# Begin Source File

SOURCE=..\..\README.WIN32

!IF  "$(CFG)" == "runmakedb - Win32 Release"

# Begin Custom Build - bin\makedb.exe -i bzflag.spc -o lib\database.obj -b 1048576
InputPath=..\..\README.WIN32

"lib\database.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	bin\makedb.exe -i bzflag.spc -o lib\database.obj -b 1048576 
	dir /b lib\database.obj 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "runmakedb - Win32 Debug"

# Begin Custom Build - bin\makedb.exe -i bzflag.spc -o lib\database.obj -b 1048576
InputPath=..\..\README.WIN32

"lib\database.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	bin\makedb.exe -i bzflag.spc -o lib\database.obj -b 1048576 
	dir /b lib\database.obj 
	
# End Custom Build

!ENDIF 

# End Source File
# End Target
# End Project
