# Microsoft Developer Studio Project File - Name="ogl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ogl - Win32 SDL_Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ogl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ogl.mak" CFG="ogl - Win32 SDL_Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ogl - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ogl - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ogl - Win32 SDL_Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ogl - Win32 SDL_Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ogl - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\src\ogl"
# PROP Intermediate_Dir "..\..\src\ogl\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\include" /I "..\..\win32" /I ".\\" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\src\ogl\Release\ogl.lib"

!ELSEIF  "$(CFG)" == "ogl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\src\ogl\Debug"
# PROP Intermediate_Dir "..\..\src\ogl\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I "..\..\include" /I "..\..\win32" /I ".\\" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ogl - Win32 SDL_Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ogl___Win32_SDL_Release"
# PROP BASE Intermediate_Dir "ogl___Win32_SDL_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\src\ogl\SDL_Release"
# PROP Intermediate_Dir "..\..\src\ogl\SDL_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\..\include" /I "..\..\win32" /I ".\\" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\include" /I "..\..\win32" /I ".\\" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_SDL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\src\ogl\Release\ogl.lib"
# ADD LIB32 /nologo /out:"..\..\src\ogl\Release\ogl.lib"

!ELSEIF  "$(CFG)" == "ogl - Win32 SDL_Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ogl___Win32_SDL_Debug"
# PROP BASE Intermediate_Dir "ogl___Win32_SDL_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\src\ogl\SDL_Debug"
# PROP Intermediate_Dir "..\..\src\ogl\SDL_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GX /Zi /Od /I "..\..\include" /I "..\..\win32" /I ".\\" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I "..\..\include" /I "..\..\win32" /I ".\\" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_SDL" /FD /GZ /c
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

# Name "ogl - Win32 Release"
# Name "ogl - Win32 Debug"
# Name "ogl - Win32 SDL_Release"
# Name "ogl - Win32 SDL_Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\ogl\OpenGLDisplayList.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\ogl\OpenGLGState.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\ogl\OpenGLLight.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\ogl\OpenGLMaterial.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\ogl\OpenGLTexture.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\ogl\RenderNode.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\common.h
# End Source File
# Begin Source File

SOURCE=.\config.h
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

SOURCE=..\..\include\OpenGLTexture.h
# End Source File
# Begin Source File

SOURCE="C:\Program Files\Microsoft SDK\include\Reason.h"
# End Source File
# Begin Source File

SOURCE=..\..\include\RenderNode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TextureManager.h
# End Source File
# End Group
# End Target
# End Project
