;NSIS Modern User Interface version 1.69
;Original templates by Joost Verburg
;Redesigned for BZFlag by blast007

;--------------------------------
;Includes

  ; Modern UI
  !include "MUI2.nsh"

  ; Windows Version Detection
  !include "WinVer.nsh"

;--------------------------------
;BZFlag Version Variables

  !define VER_MAJOR 2.4
  !define VER_MINOR .2
  
  !ifdef BUILD_64
    !define PLATFORM x64
    !define RUNTIME_PLATFORM x64
    !define BITNESS 64Bit
  !else
    !define PLATFORM Win32
    !define RUNTIME_PLATFORM x86
    !define BITNESS 32Bit
  !endif
  
;--------------------------------
;Compression options

  ;If you want to comment these
  ;out while testing, it speeds
  ;up the installer compile time
  ;Uncomment to reduce installer
  ;size by ~35%
  SetCompress auto
  SetCompressor /SOLID lzma

;--------------------------------
;Configuration

  ; Installer output file and default installation folder
  Name "BZFlag ${VER_MAJOR}${VER_MINOR} ${BITNESS}"
  !ifdef BUILD_64
    OutFile "..\..\..\bin_x64\bzflag-${VER_MAJOR}${VER_MINOR}_${BITNESS}.exe"
    InstallDir "$PROGRAMFILES64\BZFlag${VER_MAJOR}${VER_MINOR}_${BITNESS}"
  !else
    OutFile "..\..\..\bin_Win32\bzflag-${VER_MAJOR}${VER_MINOR}_${BITNESS}.exe"
    InstallDir "$PROGRAMFILES32\BZFlag${VER_MAJOR}${VER_MINOR}_${BITNESS}"
  !endif

  ; Make it look pretty in XP
  XPStyle on
  
  ; The installer needs administrative rights
  RequestExecutionLevel admin

;--------------------------------
;Variables

  Var MUI_TEMP
  Var STARTMENU_FOLDER

;--------------------------------
;Interface Settings

  ;Icons
  !define MUI_ICON ..\..\..\MSVC\bzflag.ico
  !define MUI_UNICON uninstall.ico

  ;Bitmaps
  !define MUI_WELCOMEFINISHPAGE_BITMAP "side.bmp"
  !define MUI_UNWELCOMEFINISHPAGE_BITMAP "side.bmp"

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "header.bmp"
  !define MUI_COMPONENTSPAGE_CHECKBITMAP "${NSISDIR}\Contrib\Graphics\Checks\simple-round2.bmp"

  !define MUI_COMPONENTSPAGE_SMALLDESC

  ;Show a warning before aborting install
  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  ;Welcome page configuration
  !define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of BZFlag ${VER_MAJOR}${VER_MINOR} ${__DATE__} ${BITNESS}.$\r$\n$\r$\nBZFlag is a free multiplayer multiplatform 3D tank battle game. The name stands for Battle Zone capture Flag. It runs on Irix, Linux, *BSD, Windows, Mac OS X and other platforms. It's one of the most popular games ever on Silicon Graphics machines.$\r$\n$\r$\nClick Next to continue."

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "copying.rtf"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY

  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\BZFlag${VER_MAJOR}${VER_MINOR}${BITNESS}" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

  !insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER


  !insertmacro MUI_PAGE_INSTFILES

  ;Finished page configuration
  !define MUI_FINISHPAGE_NOAUTOCLOSE
  !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\doc\ReadMe.win32.html"
  !define MUI_FINISHPAGE_SHOWREADME_TEXT "View Readme"
  !define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED

  !define MUI_FINISHPAGE_LINK "BZFlag Home Page"
  !define MUI_FINISHPAGE_LINK_LOCATION "http://www.bzflag.org"

  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

  !define MUI_UNFINISHPAGE_NOAUTOCLOSE
  !insertmacro MUI_UNPAGE_FINISH
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "!BZFlag (Required)" BZFlag
  ;Make it required
  SectionIn RO

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  ; Put file there
  File ..\..\..\bin_${PLATFORM}\bzflag.exe
  
  ; make the data dir
  SetOutPath $INSTDIR\data
  File ..\..\..\data\*.png

  ; make the fonts dir
  SetOutPath $INSTDIR\data\fonts
  File ..\..\..\data\fonts\*.png
  File ..\..\..\data\fonts\*.fmt
  File ..\..\..\data\fonts\*.License
  File ..\..\..\data\fonts\README

  ; make the l10n dir
  SetOutPath $INSTDIR\data\l10n
  File ..\..\..\data\l10n\*.po
  File ..\..\..\data\l10n\*.txt

  SetOutPath $INSTDIR\data
  File ..\..\..\data\*.png

  ; make the sounds dir
  SetOutPath $INSTDIR\data
  File ..\..\..\data\*.wav

  ; make the doc dir
  SetOutPath $INSTDIR\doc
  File ..\ReadMe.win32.html
  File ..\..\..\COPYING
  File ..\..\..\bin_${PLATFORM}\docs\bzflag.html

  ; Add some DLL files
  SetOutPath $INSTDIR
  File ..\..\..\bin_${PLATFORM}\libcurl.dll

  ; This requires the Visual C++ runtime file to be located in
  ; the same directory as the NSIS script
  ; http://www.microsoft.com/downloads/details.aspx?familyid=2051A0C1-C9B5-4B0A-A8F5-770A549FD78C
  SetOutPath $TEMP
  DetailPrint "Installing Visual C++ ${BITNESS} runtime"         
  File vcredist_${RUNTIME_PLATFORM}.exe  
  ExecWait "$TEMP\vcredist_${RUNTIME_PLATFORM}.exe /q"         
  DetailPrint "Cleaning up"         
  Delete $TEMP\vcredist_${RUNTIME_PLATFORM}.exe

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\BZFlag${VER_MAJOR}${VER_MINOR} "Install_Dir" "$INSTDIR"

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BZFlag${VER_MAJOR}${VER_MINOR}_${BITNESS}" "DisplayName" "BZFlag ${VER_MAJOR}${VER_MINOR} ${BITNESS} (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BZFlag${VER_MAJOR}${VER_MINOR}_${BITNESS}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"


  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ;Main start menu shortcuts
    SetOutPath $INSTDIR
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\BZFlag ${VER_MAJOR}${VER_MINOR}.lnk" "$INSTDIR\bzflag.exe" "" "$INSTDIR\bzflag.exe" 0
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\BZFlag ${VER_MAJOR}${VER_MINOR} (Windowed).lnk" "$INSTDIR\bzflag.exe"  "-window 800x600" "$INSTDIR\bzflag.exe" 0

	; Local User Data
    Var /GLOBAL UserData
    ${If} ${AtMostWinXP}
      StrCpy $UserData "%USERPROFILE%\Local Settings\Application Data\BZFlag"
    ${Else}
      StrCpy $UserData "%LOCALAPPDATA%\BZFlag"
    ${EndIf}

    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Browse User Data.lnk" "$UserData"

    SetOutPath $INSTDIR\doc
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER\Doc"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Doc\BZFlag [game] Manual Pages (HTML).lnk" "$INSTDIR\doc\bzflag.html" "" "" 0
  
  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

Section "BZAdmin" BZAdmin
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  ; Put file there
  File ..\..\..\bin_${PLATFORM}\bzadmin.exe

  ; Add to the doc dir
  SetOutPath $INSTDIR\doc
  File ..\..\..\bin_${PLATFORM}\docs\bzadmin.html

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ;Main start menu shortcuts
    SetOutPath $INSTDIR
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\BZAdmin ${VER_MAJOR}${VER_MINOR}.lnk" "$INSTDIR\bzadmin.exe" "" "$INSTDIR\bzadmin.exe" 0

    SetOutPath $INSTDIR\doc
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER\Doc"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Doc\bzadmin [admin] Manual Pages (HTML).lnk" "$INSTDIR\doc\bzadmin.html" "" "" 0
  
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

SectionGroup "BZFlag Server" BZFlagServer
  Section "Server Application" BZFlagServer_Application
    ; Set output path to the installation directory.
    SetOutPath $INSTDIR
    ; Put file there
    File ..\..\..\bin_${PLATFORM}\bzfs.exe

    ; add to the data dir
    SetOutPath $INSTDIR\misc
    File ..\..\..\misc\maps\hix.bzw
    File ..\..\..\misc\bzfs.conf
    File ..\..\..\misc\bzfs_conf.html
    File ..\..\..\misc\groups.conf
    File ..\..\..\misc\vars.txt

    ; Add to the doc dir
    SetOutPath $INSTDIR\doc
    File ..\..\..\bin_${PLATFORM}\docs\bzfs.html
    File ..\..\..\bin_${PLATFORM}\docs\bzw.html

    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
      ;Main start menu shortcuts
      SetOutPath $INSTDIR
      CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER\Server"
      CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Server\Start Server (Simple Jump Teleport 1 shot).lnk" "$INSTDIR\bzfs.exe" "-p 5154 -j -t -s 32 +s 16 -h" "$INSTDIR\bzflag.exe" 0
      CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Server\Start Server (Simple Jump Teleport 3 shots).lnk" "$INSTDIR\bzfs.exe" "-p 5154 -j -t -ms 3 -s 32 +s 16 -h" "$INSTDIR\bzflag.exe" 0
      CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Server\Start Server (HIX [Public] FFA).lnk" "$INSTDIR\bzfs.exe" '-p 5154 -j -tkkr 80 -fb -ms 3 -s 32 +s 16 -world misc\hix.bzw' "$INSTDIR\bzflag.exe" 0
      CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Server\Start Server (HIX [Public] CTF).lnk" "$INSTDIR\bzfs.exe" '-p 5154 -c -j -fb -world misc\hix.bzw' "$INSTDIR\bzflag.exe" 0
      CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Server\BZFS Configuration Builder.lnk" "$INSTDIR\misc\bzfs_conf.html" "" "" 0

      SetOutPath $INSTDIR\doc
      CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER\Doc"
      CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Doc\bzfs [server] Manual Pages (HTML).lnk" "$INSTDIR\doc\bzfs.html" "" "" 0
      CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Doc\bzw [maps] Manual Pages (HTML).lnk" "$INSTDIR\doc\bzw.html" "" "" 0
  
    !insertmacro MUI_STARTMENU_WRITE_END
  SectionEnd

  Section "Plugins" BZFlagServer_Plugins
    ; Include the plugins
    SetOutPath $INSTDIR
    File ..\..\..\bin_${PLATFORM}\plugins\*.dll
  SectionEnd

  Section "Plugin API" BZFlagServer_PluginAPI
    ; Add the API library and header
    SetOutPath $INSTDIR\API
    File ..\..\..\bin_${PLATFORM}\bzfs.lib
    File ..\..\..\bin_${PLATFORM}\plugin_utils.lib
    File ..\..\..\include\bzfsAPI.h
    File ..\..\..\plugins\plugin_utils\*.h
  SectionEnd
SectionGroupEnd

Section "Desktop Icon" Desktop
  ; Install for all users
  SetShellVarContext all
  
  ;shortcut on the "desktop"
  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\BZFlag${VER_MAJOR}${VER_MINOR} ${BITNESS}.lnk" "$INSTDIR\bzflag.exe" "" "$INSTDIR\bzflag.exe" 0
SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_BZFlag ${LANG_ENGLISH} "Installs the main 3D application and the associated data files."
  LangString DESC_BZAdmin ${LANG_ENGLISH} "Installs the command line administration tool."
  LangString DESC_BZFlagServer ${LANG_ENGLISH} "Installs the server application, associated plugins, and/or the plugin API."
  LangString DESC_BZFlagServer_Application ${LANG_ENGLISH} "This can be used to run private and public servers, but is not required to play the game."
  LangString DESC_BZFlagServer_Plugins ${LANG_ENGLISH} "Plugins can be used to modify the way a bzflag server runs, and add functionality."
  LangString DESC_BZFlagServer_PluginAPI ${LANG_ENGLISH} "The plugin API is used to compile plugins, and is only need for plugin developers."
  LangString DESC_Desktop ${LANG_ENGLISH} "Adds a shortcut on the desktop."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${BZFlag} $(DESC_BZFlag)
    !insertmacro MUI_DESCRIPTION_TEXT ${BZAdmin} $(DESC_BZAdmin)
    !insertmacro MUI_DESCRIPTION_TEXT ${BZFlagServer} $(DESC_BZFlagServer)
    !insertmacro MUI_DESCRIPTION_TEXT ${BZFlagServer_Application} $(DESC_BZFlagServer_Application)
    !insertmacro MUI_DESCRIPTION_TEXT ${BZFlagServer_Plugins} $(DESC_BZFlagServer_Plugins)
    !insertmacro MUI_DESCRIPTION_TEXT ${BZFlagServer_PluginAPI} $(DESC_BZFlagServer_PluginAPI)
    !insertmacro MUI_DESCRIPTION_TEXT ${Desktop} $(DESC_Desktop)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"
  ; remove files
  Delete $INSTDIR\*.*
  Delete $INSTDIR\doc\*.*
  Delete $INSTDIR\misc\*.*
  Delete $INSTDIR\data\*.*
  Delete $INSTDIR\data\fonts\*.*
  Delete $INSTDIR\data\l10n\*.*
  Delete $INSTDIR\data\*.*

  Delete $INSTDIR\API\*.*

  ; MUST REMOVE UNINSTALLER, too
  Delete $INSTDIR\uninstall.exe


  ; remove directories used.
  RMDir "$INSTDIR\API"
  RMDir "$INSTDIR\data\l10n"
  RMDir "$INSTDIR\data\fonts"
  RMDir "$INSTDIR\data"
  RMDir "$INSTDIR\misc"
  RMDir "$INSTDIR\doc"
  RMDir "$INSTDIR"
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP

  ;remove shortcuts, if any.    
  Delete "$SMPROGRAMS\$MUI_TEMP\*.*"
  Delete "$SMPROGRAMS\$MUI_TEMP\Server\*.*"
  Delete "$SMPROGRAMS\$MUI_TEMP\Doc\*.*"
  RMDir "$SMPROGRAMS\$MUI_TEMP\Server"
  RMDir "$SMPROGRAMS\$MUI_TEMP\Doc"
  RMDir "$SMPROGRAMS\$MUI_TEMP"
  
  ;Delete empty start menu parent diretories
  StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"
 
  startMenuDeleteLoop:
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."
    
    IfErrors startMenuDeleteLoopDone
  
    StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
  startMenuDeleteLoopDone:
  
  ; Remove desktop shortcut for all users
  SetShellVarContext all
  Delete "$DESKTOP\BZFlag${VER_MAJOR}${VER_MINOR} ${BITNESS}.lnk"
  
  ;remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BZFlag${VER_MAJOR}${VER_MINOR}${BITNESS}"
  DeleteRegKey HKLM "SOFTWARE\BZFlag${VER_MAJOR}${VER_MINOR}${BITNESS}"
  DeleteRegKey HKCU "Software\BZFlag"

SectionEnd
