;NSIS Modern User Interface version 1.69
;Original templates by Joost Verburg
;Redesigned for BZFlag by blast007

;--------------------------------
;BZFlag Version Variables

  !define VER_MAJOR 2.99
  !define VER_MINOR .19.20090321

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
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;Configuration

  ;General
  Name "BZFlag ${VER_MAJOR}${VER_MINOR}"
  OutFile "..\..\..\dist\bzflag-${VER_MAJOR}${VER_MINOR}.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\BZFlag${VER_MAJOR}${VER_MINOR}"

  ; Make it look pretty in XP
  XPStyle on

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
  !define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of BZFlag ${VER_MAJOR}${VER_MINOR}.\r\n\r\nBZFlag is a free multiplayer multiplatform 3D tank battle game. The name stands for Battle Zone capture Flag. It runs on Irix, Linux, *BSD, Windows, Mac OS X and other platforms. It's one of the most popular games ever on Silicon Graphics machines.\r\n\r\nPlease note that you must have the Visual C++ 2008 Redistributable Package to run BZFlag, it can be downloaded from Microsoft.\r\n\r\nClick Next to continue."

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "copying.rtf"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY

  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\BZFlag${VER_MAJOR}${VER_MINOR}" 
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
  File ..\..\..\bzflag.exe
  
  ; make the data dir
  SetOutPath $INSTDIR\data
  File ..\..\..\data\*.png

  ; make the fonts dir
  SetOutPath $INSTDIR\data\fonts
  File ..\..\..\data\fonts\*.ttf
  File ..\..\..\data\fonts\README

  ; make the l10n dir
  SetOutPath $INSTDIR\data\l10n
  File ..\..\..\data\l10n\*.po
  File ..\..\..\data\l10n\*.txt

  SetOutPath $INSTDIR\data\skins\blue
  File ..\..\..\data\skins\blue\*.png

  SetOutPath $INSTDIR\data\skins\red
  File ..\..\..\data\skins\red\*.png

  SetOutPath $INSTDIR\data\skins\green
  File ..\..\..\data\skins\green\*.png

  SetOutPath $INSTDIR\data\skins\purple
  File ..\..\..\data\skins\purple\*.png

  SetOutPath $INSTDIR\data\skins\hunter
  File ..\..\..\data\skins\hunter\*.png

  SetOutPath $INSTDIR\data\skins\observer
  File ..\..\..\data\skins\observer\*.png

  SetOutPath $INSTDIR\data\skins\rabbit
  File ..\..\..\data\skins\rabbit\*.png

  SetOutPath $INSTDIR\data\skins\rogue
  File ..\..\..\data\skins\rogue\*.png

  ; make the sounds dir
  SetOutPath $INSTDIR\data\sounds
  File ..\..\..\data\sounds\*.wav

  ; make the LuaUser dir 
  ;SetOutPath $INSTDIR\data\LuaUser
  ;File ..\..\..\data\LuaUser\*.*

  ; make the doc dir
  SetOutPath $INSTDIR\doc
  File ..\ReadMe.win32.html
  File ..\..\..\COPYING
  File ..\..\..\man\bzflag.html

  ; Add some DLL files
  SetOutPath $INSTDIR
  File ..\..\..\libcurl.dll


  ; need to change this to just install the MSVC9 runtimes
  ;File ..\..\..\msvcr80.dll
  ;File ..\..\..\msvcp80.dll

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\BZFlag${VER_MAJOR}${VER_MINOR} "Install_Dir" "$INSTDIR"

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BZFlag${VER_MAJOR}${VER_MINOR}" "DisplayName" "BZFlag ${VER_MAJOR}${VER_MINOR} (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BZFlag${VER_MAJOR}${VER_MINOR}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"


  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ;Main start menu shortcuts
    SetOutPath $INSTDIR
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\BZFlag ${VER_MAJOR}${VER_MINOR}.lnk" "$INSTDIR\bzflag.exe" "" "$INSTDIR\bzflag.exe" 0
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\BZFlag ${VER_MAJOR}${VER_MINOR} (Windowed).lnk" "$INSTDIR\bzflag.exe"  "-window 800x600" "$INSTDIR\bzflag.exe" 0

    SetOutPath $INSTDIR\doc
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER\Doc"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Doc\BZFlag [game] Manual Pages (HTML).lnk" "$INSTDIR\doc\bzflag.html" "" "" 0
  
  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

Section "BZAdmin" BZAdmin
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  ; Put file there
  File ..\..\..\bzadmin.exe

  ; Add some DLL files
  ;SetOutPath $INSTDIR\
  ;File ..\..\..\curses.dll

  ; Add to the doc dir
  SetOutPath $INSTDIR\doc
  File ..\..\..\man\bzadmin.html

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
    File ..\..\..\bzfs.exe
    File ..\..\..\misc\bzfs.conf

    ; add to the data dir
    SetOutPath $INSTDIR\misc
    File ..\..\..\misc\hix.bzw
    File ..\..\..\misc\bzfs.conf
    File ..\..\..\misc\bzfs_conf.html
    File ..\..\..\misc\groups.conf
    File ..\..\..\misc\vars.txt

    ; Add to the doc dir
    SetOutPath $INSTDIR\doc
    File ..\..\..\man\bzfs.html
    File ..\..\..\man\bzw.html

    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
      ;Main start menu shortcuts
      SetOutPath $INSTDIR\data
      CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER\Server"
      CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Server\Start Server (Simple Jump Teleport 1 shot).lnk" "$INSTDIR\bzfs.exe" "-p 5154 -j -t -s 32 +s 16 -h" "$INSTDIR\bzflag.exe" 0
      CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Server\Start Server (Simple Jump Teleport 3 shots).lnk" "$INSTDIR\bzfs.exe" "-p 5154 -j -t -ms 3 -s 32 +s 16 -h" "$INSTDIR\bzflag.exe" 0
      CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Server\Start Server (HIX [Public] FFA).lnk" "$INSTDIR\bzfs.exe" '-p 5154 -j -tkkr 80 -fb -ms 3 -s 32 +s 16 -world misc\HIX.bzw -public "Lazy Users HIX FFA Server"' "$INSTDIR\bzflag.exe" 0
      CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Server\Start Server (HIX [Public] CTF).lnk" "$INSTDIR\bzfs.exe" '-p 5154 -c -j -fb -world misc\HIX.bzw -public "Lazy Users HIX CTF Server"' "$INSTDIR\bzflag.exe" 0
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
    File ..\..\..\plugins\*.dll
  SectionEnd

  Section "Plugin API" BZFlagServer_PluginAPI
    ; Add the API library and header
    SetOutPath $INSTDIR\API
    File ..\..\..\bzfs.lib
    File ..\..\..\plugins\plugin_utils\Release\plugin_utils.lib
    File ..\..\..\include\bzfsAPI.h
    File ..\..\..\plugins\plugin_utils\*.h
  SectionEnd
SectionGroupEnd

Section "Quick Launch Shortcuts" QuickLaunch
  ;shortcut in the "quick launch bar"
  SetOutPath $INSTDIR
  CreateShortCut "$QUICKLAUNCH\BZFlag${VER_MAJOR}${VER_MINOR}.lnk" "$INSTDIR\bzflag.exe" "" "$INSTDIR\bzflag.exe" 0
SectionEnd

Section "Desktop Icon" Desktop
  ;shortcut on the "desktop"
  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\BZFlag${VER_MAJOR}${VER_MINOR}.lnk" "$INSTDIR\bzflag.exe" "" "$INSTDIR\bzflag.exe" 0
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
  LangString DESC_QuickLaunch ${LANG_ENGLISH} "Adds a shortcut in the Quick Launch toolbar."
  LangString DESC_Desktop ${LANG_ENGLISH} "Adds a shortcut on the desktop."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${BZFlag} $(DESC_BZFlag)
    !insertmacro MUI_DESCRIPTION_TEXT ${BZAdmin} $(DESC_BZAdmin)
    !insertmacro MUI_DESCRIPTION_TEXT ${BZFlagServer} $(DESC_BZFlagServer)
    !insertmacro MUI_DESCRIPTION_TEXT ${BZFlagServer_Application} $(DESC_BZFlagServer_Application)
    !insertmacro MUI_DESCRIPTION_TEXT ${BZFlagServer_Plugins} $(DESC_BZFlagServer_Plugins)
    !insertmacro MUI_DESCRIPTION_TEXT ${BZFlagServer_PluginAPI} $(DESC_BZFlagServer_PluginAPI)
    !insertmacro MUI_DESCRIPTION_TEXT ${QuickLaunch} $(DESC_QuickLaunch)
    !insertmacro MUI_DESCRIPTION_TEXT ${Desktop} $(DESC_Desktop)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"
  ;remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BZFlag${VER_MAJOR}${VER_MINOR}"
  DeleteRegKey HKLM "SOFTWARE\BZFlag${VER_MAJOR}${VER_MINOR}"
  DeleteRegKey HKCU "Software\BZFlag"

  ; remove files
  Delete $INSTDIR\*.*
  Delete $INSTDIR\doc\*.*
  Delete $INSTDIR\misc\*.*
  Delete $INSTDIR\data\*.*
  Delete $INSTDIR\data\fonts\*.*
  Delete $INSTDIR\data\l10n\*.*
  Delete $INSTDIR\data\skins\blue\*.*
  Delete $INSTDIR\data\skins\red\*.*
  Delete $INSTDIR\data\skins\green\*.*
  Delete $INSTDIR\data\skins\purple\*.*
  Delete $INSTDIR\data\skins\rabbit\*.*
  Delete $INSTDIR\data\skins\observer\*.*
  Delete $INSTDIR\data\skins\hunter\*.*
  Delete $INSTDIR\data\skins\rogue\*.*

  Delete $INSTDIR\API\*.*

  ; MUST REMOVE UNINSTALLER, too
  Delete $INSTDIR\uninstall.exe


  ; remove directories used.
  RMDir "$INSTDIR\API"
  RMDir "$INSTDIR\data\l10n"
  RMDir "$INSTDIR\data\fonts"
  RMDir "$INSTDIR\data\skins\blue"
  RMDir "$INSTDIR\data\skins\green"
  RMDir "$INSTDIR\data\skins\hunter"
  RMDir "$INSTDIR\data\skins\red"
  RMDir "$INSTDIR\data\skins\purple"
  RMDir "$INSTDIR\data\skins\rabbit"
  RMDir "$INSTDIR\data\skins\observer"
  RMDir "$INSTDIR\data\skins\rogue"
  RMDir "$INSTDIR\data\skins\"
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

  Delete "$QUICKLAUNCH\BZFlag${VER_MAJOR}${VER_MINOR}.lnk"
  Delete "$DESKTOP\BZFlag${VER_MAJOR}${VER_MINOR}.lnk"
  
  ;Delete empty start menu parent diretories
  StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"
 
  startMenuDeleteLoop:
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."
    
    IfErrors startMenuDeleteLoopDone
  
    StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
  startMenuDeleteLoopDone:

SectionEnd
