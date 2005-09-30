; BZFlag.nsi
;
; This script is based on example1.nsi, but adds uninstall support
; and (optionally) start menu shortcuts.
;
; It will install BZFlag into a directory that the user selects.
;

!define VER_MAJOR 2.0
!define VER_MINOR .4Final

; Main Installer Options
Name "BZFlag"
;Icon ..\..\..\win32\bzflag.ico
WindowIcon On
;EnabledBitmap "EnableCheck.bmp"
;DisabledBitmap "DisableCheck.bmp"
Caption "BZFlag ${VER_MAJOR}${VER_MINOR}: - Setup"

; The file to write
OutFile "..\..\..\dist\bzflag-${VER_MAJOR}${VER_MINOR}.exe"

; The default installation directory
InstallDir $PROGRAMFILES\BZFlag${VER_MAJOR}${VER_MINOR}

; Show the lisense
LicenseText "Please read our following license before installing:"
LicenseData ..\..\..\COPYING

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\BZFlag${VER_MAJOR}${VER_MINOR} "Install_Dir"

; The text to prompt the user to enter a directory
ComponentText "This will install the BZFlag ${VER_MAJOR}${VER_MINOR} game and server files on your computer."

; The text to prompt the user to enter a directory
DirText "Please choose a directory to install into:"

CompletedText "Thank you for installing BZFlag ${VER_MAJOR}${VER_MINOR}."
; The stuff to install

Section "BZFlag (required)"
	; Set output path to the installation directory.
	SetOutPath $INSTDIR
	; Put file there
	File ..\..\..\src\bzflag\bzflag.exe
	File ..\..\..\src\bzadmin\bzadmin.exe
	File ..\..\..\src\bzfs\bzfs.exe
	File ..\..\..\libcurl.dll
	File ..\..\..\curses.dll
	File ..\..\..\zlibwapi.dll
	File ..\..\..\plugins\*.dll

	; See http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_crt_c_run.2d.time_libraries.asp
	; "An application should use and redistribute msvcr71.dll [and msvcp71.dll], and it should avoid placing a copy or using an existing copy of msvcr71.dll in the system directory. Instead, the application should keep a copy of msvcr71.dll [and msvcp71.dll] in its application directory with the program executable."
	File ..\..\..\msvcr71.dll
	File ..\..\..\msvcp71.dll


	SetOutPath $INSTDIR\API
	File ..\..\..\src\bzfs\bzfs.lib
	File ..\..\..\include\bzfsAPI.h

	; make the data dir
	SetOutPath $INSTDIR\data
	File ..\..\..\data\*.*
	File ..\..\..\misc\hix.bzw
	File ..\..\..\misc\bzfs.conf
	File ..\..\..\misc\bzfs_conf.html

	; make the l10n dir
	SetOutPath $INSTDIR\data\l10n
	File ..\..\..\data\l10n\*.*

	; make the fonts dir
	SetOutPath $INSTDIR\data\fonts
	File ..\..\..\data\fonts\*.*

	; make the doc dir
	SetOutPath $INSTDIR\doc
	File ..\..\..\doc\*.*
	File ..\ReadMe.win32.html
	File ..\..\..\COPYING

	; make the plugin docs dir
	SetOutPath $INSTDIR\doc\plugin
	File ..\..\..\plugins\doc\*.*

	; Write the installation path into the registry
	WriteRegStr HKLM SOFTWARE\BZFlag "Install_Dir" "$INSTDIR"

	; Write the uninstall keys for Windows
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BZFlag${VER_MAJOR}${VER_MINOR}" "DisplayName" "BZFlag${VER_MAJOR}${VER_MINOR} (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BZFlag${VER_MAJOR}${VER_MINOR}" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteUninstaller "uninstall.exe"
SectionEnd

; optional sections
Section "Start Menu Shortcuts"
	; Main start menu shortcuts
	SetOutPath $INSTDIR
	CreateDirectory "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}"
	CreateShortCut "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
	CreateShortCut "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\BZFlag ${VER_MAJOR}${VER_MINOR}.lnk" "$INSTDIR\bzflag.exe" "" "$INSTDIR\bzflag.exe" 0
	CreateShortCut "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\BZFlag ${VER_MAJOR}${VER_MINOR} (Windowed).lnk" "$INSTDIR\bzflag.exe"  "-window -geometry 800x600" "$INSTDIR\bzflag.exe" 0

	SetOutPath $INSTDIR\data
	CreateDirectory "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\Server"
	CreateShortCut "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\Server\Start Server (Simple Jump Teleport 1 shot).lnk" "$INSTDIR\bzfs.exe" "-p 5154 -j -t -s 32 +s 16 -h" "$INSTDIR\bzflag.exe" 0
	CreateShortCut "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\Server\Start Server (Simple Jump Teleport 3 shots).lnk" "$INSTDIR\bzfs.exe" "-p 5155 -j -t -ms 3 -s 32 +s 16 -h" "$INSTDIR\bzflag.exe" 0
	CreateShortCut "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\Server\Start Server (HIX [Public] FFA).lnk" "$INSTDIR\bzfs.exe" '-p 5156 -j -tkkr 80 -fb -ms 3 -s 32 +s 16 -world HIX.bzw -public "My HIX FFA Server"' "$INSTDIR\bzflag.exe" 0
	CreateShortCut "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\Server\Start Server (HIX [Public] CTF).lnk" "$INSTDIR\bzfs.exe" '-p 5157 -c -j -fb -world HIX.bzw -public "My HIX CTF Server"' "$INSTDIR\bzflag.exe" 0
	CreateShortCut "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\Server\Data Folder.lnk" "$INSTDIR\data" "" "" 0
	CreateShortCut "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\Server\BZFS Configuration Builder.lnk" "$INSTDIR\data\bzfs_conf.html" "" "" 0
	SetOutPath $INSTDIR\doc
	CreateDirectory "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\Doc"
	CreateShortCut "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\Doc\BZFlag [game] Manual Pages (HTML).lnk" "$INSTDIR\doc\bzflag.html" "" "" 0
	CreateShortCut "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\Doc\bzfs [server] Manual Pages (HTML).lnk" "$INSTDIR\doc\bzfs.html" "" "" 0
	CreateShortCut "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\Doc\bzadmin [admin] Manual Pages (HTML).lnk" "$INSTDIR\doc\bzadmin.html" "" "" 0
SectionEnd

Section "Quick Launch Shortcuts"
	; shortcut in the "quick launch bar"
	SetOutPath $INSTDIR
	CreateShortCut "$QUICKLAUNCH\BZFlag${VER_MAJOR}${VER_MINOR}.lnk" "$INSTDIR\bzflag.exe" "" "$INSTDIR\bzflag.exe" 0
SectionEnd

Function .onInstSuccess
	MessageBox MB_YESNO|MB_ICONQUESTION \
		"Setup has completed. Would you like to view readme file now?" \
		IDNO NoReadme
			ExecShell open '$INSTDIR\doc\ReadMe.win32.html'
		NoReadme:
FunctionEnd

; uninstall stuff
UninstallText "This will uninstall BZFlag. Please hit next to continue with the removal."

; special uninstall section.
Section "Uninstall"
	; remove registry keys

	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BZFlag${VER_MAJOR}${VER_MINOR}"
	DeleteRegKey HKLM "Software\BZFlag${VER_MAJOR}${VER_MINOR}"
	; remove files

	Delete $INSTDIR\bzflag.exe
	Delete $INSTDIR\bzfs.exe
	Delete $INSTDIR\bzadmin.exe
	Delete $INSTDIR\doc\plugin\*.*
	Delete $INSTDIR\doc\*.*
	Delete $INSTDIR\data\fonts\*.*
	Delete $INSTDIR\data\*.*
	Delete $INSTDIR\data\l10n\*.*
	Delete $INSTDIR\API\*.*

	; This nails libcurl, the msvcrts, and any plugins
	Delete $INSTDIR\*.dll

	; MUST REMOVE UNINSTALLER, too
	Delete $INSTDIR\uninstall.exe

	; remove shortcuts, if any.
	Delete "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\*.*"
	Delete "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\Server\*.*"
	Delete "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\Doc\*.*"
	Delete "$$QUICKLAUNCH\BZFlag${VER_MAJOR}${VER_MINOR}.lnk"

	; remove directories used.
	RMDir "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\Server"
	RMDir "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}\Doc"
	RMDir "$SMPROGRAMS\BZFlag${VER_MAJOR}${VER_MINOR}"
	RMDir "$INSTDIR\data\l10n"
	RMDir "$INSTDIR\data\fonts"
	RMDir "$INSTDIR\data"
	RMDir "$INSTDIR\doc"
	RMDir "$INSTDIR\API"
	RMDir "$INSTDIR"
SectionEnd

; eof
