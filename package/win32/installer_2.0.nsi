;NSIS Modern User Interface version 1.70
;Basic Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

	!include "MUI.nsh"

;--------------------------------
;General

	!define VER_MAJOR 1.11
	!define VER_MINOR .14

	;Name and file
	Name "BZFlag ${VER_MAJOR}${VER_MINOR}: - Setup"
	Icon ..\..\win32\bzflag.ico

	OutFile "..\..\dist\bzflag_installer_${VER_MAJOR}${VER_MINOR}.exe"

	;Default installation folder
	InstallDir "$PROGRAMFILES\bzflag_2"

	;Get installation folder from registry if available
	InstallDirRegKey HKCU "Software\bzflag_2" ""

;--------------------------------
;Interface Settings

	!define MUI_HEADERIMAGE
	!define MUI_ABORTWARNING

;--------------------------------
;Pages

	!insertmacro MUI_PAGE_LICENSE "..\..\COPYING"
	!insertmacro MUI_PAGE_COMPONENTS
	!insertmacro MUI_PAGE_DIRECTORY
	!insertmacro MUI_PAGE_INSTFILES

	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
	!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "BZFlag Core (required)" SecCore

	SetOutPath "$INSTDIR"
  
	File ..\..\src\bzflag\bzflag.exe
	File ..\..\libcurl.dll
  
  ;Store installation folder
  WriteRegStr HKCU "Software\bzflag_2" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

Section "BZAdmin" SecBZAdmin

	SetOutPath "$INSTDIR"
  
	File ..\..\src\bzadmin\bzadmin.exe

SectionEnd

Section "BZFlag Server" SecBZFS

	SetOutPath "$INSTDIR"
  
	File ..\..\src\bzfs\bzfs.exe
SectionEnd


;--------------------------------
;Descriptions

	;Language strings
	LangString DESC_SecCore ${LANG_ENGLISH} "Files required to run BZFlag"
	LangString DESC_SecBZAdmin ${LANG_ENGLISH} "Administraton tool for BZFlag Servers"
	LangString DESC_SecBZFS ${LANG_ENGLISH} "The BZFlag server aplication ( BZFS )"

	;Assign language strings to sections
	!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
		!insertmacro MUI_DESCRIPTION_TEXT ${SecCore} $(DESC_SecCore)
		!insertmacro MUI_DESCRIPTION_TEXT ${SecBZAdmin} $(DESC_SecBZAdmin)
		!insertmacro MUI_DESCRIPTION_TEXT ${SecBZFS} $(DESC_SecBZFS)

	!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...
  Delete "$INSTDIR\*.*"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"

  DeleteRegKey /ifempty HKCU "Software\bzflag_2"

SectionEnd