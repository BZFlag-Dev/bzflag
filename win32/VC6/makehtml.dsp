# Microsoft Developer Studio Project File - Name="makehtml" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=makehtml - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "makehtml.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "makehtml.mak" CFG="makehtml - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "makehtml - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "makehtml - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
MTL=midl.exe

!IF  "$(CFG)" == "makehtml - Win32 Release"

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

!ELSEIF  "$(CFG)" == "makehtml - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "makehtml - Win32 Release"
# Name "makehtml - Win32 Debug"
# Begin Source File

SOURCE=..\..\man\bzadmin.6s

!IF  "$(CFG)" == "makehtml - Win32 Release"

# Begin Custom Build - Running $(OutDir)\man2html on $(InputPath)
OutDir=.\Release
ProjDir=.
InputPath=..\..\man\bzadmin.6s

"$(ProjDir)\..\..\doc\bzadmin.html" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(OutDir)\man2html < $(InputPath) > $(ProjDir)\..\..\doc\bzadmin.html

# End Custom Build

!ELSEIF  "$(CFG)" == "makehtml - Win32 Debug"

# Begin Custom Build - Running $(OutDir)\man2html on $(InputPath)
OutDir=.\Debug
ProjDir=.
InputPath=..\..\man\bzadmin.6s

"$(ProjDir)\..\..\doc\bzadmin.html" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(OutDir)\man2html < $(InputPath) > $(ProjDir)\..\..\doc\bzadmin.html

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\man\bzflag.6s

!IF  "$(CFG)" == "makehtml - Win32 Release"

# PROP Intermediate_Dir "Release"
# Begin Custom Build - Running $(OutDir)\man2html on $(InputPath)
OutDir=.\Release
ProjDir=.
InputPath=..\..\man\bzflag.6s

"$(ProjDir)\..\..\doc\bzflag.html" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(OutDir)\man2html < $(InputPath) > $(ProjDir)\..\..\doc\bzflag.html

# End Custom Build

!ELSEIF  "$(CFG)" == "makehtml - Win32 Debug"

# PROP Intermediate_Dir "Debug"
# Begin Custom Build - Running $(OutDir)\man2html on $(InputPath)
OutDir=.\Debug
ProjDir=.
InputPath=..\..\man\bzflag.6s

"$(ProjDir)\..\..\doc\bzflag.html" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(OutDir)\man2html < $(InputPath) > $(ProjDir)\..\..\doc\bzflag.html

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\man\bzfs.6s

!IF  "$(CFG)" == "makehtml - Win32 Release"

# Begin Custom Build - Running $(OutDir)\man2html on $(InputPath)
OutDir=.\Release
ProjDir=.
InputPath=..\..\man\bzfs.6s

"$(ProjDir)\..\..\doc\bzfs.html" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(OutDir)\man2html < $(InputPath) > $(ProjDir)\..\..\doc\bzfs.html

# End Custom Build

!ELSEIF  "$(CFG)" == "makehtml - Win32 Debug"

# Begin Custom Build - Running $(OutDir)\man2html on $(InputPath)
OutDir=.\Debug
ProjDir=.
InputPath=..\..\man\bzfs.6s

"$(ProjDir)\..\..\doc\bzfs.html" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(OutDir)\man2html < $(InputPath) > $(ProjDir)\..\..\doc\bzfs.html

# End Custom Build

!ENDIF 

# End Source File
# End Target
# End Project

