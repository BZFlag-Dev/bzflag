project "platform"
  kind "StaticLib"

  -- generic files
  files {
    "BzfDisplay.cxx",
    "BzfJoystick.cxx",
    "BzfMedia.cxx",
    "BzfVisual.cxx",
    "BzfWindow.cxx",
    "PlatformFactory.cxx",
    "wave.cxx",
    "wave.h",
    "../../include/*.h"
  }
  vpaths {
    ["Header Files/include"] = "../../include",
    ["Header Files"] = "**.h",
    ["Source Files"] = "**.cxx"
  }

  -- SDL
  filter "options:not with-sdl=no"
    files {
      "SDLJoystick.cxx",
      "SDLJoystick.h",
      "SDLPlatformFactory.cxx",
      "SDLPlatformFactory.h",
      "SDLMedia.cxx",
      "SDLMedia.h"
    }
    removefiles { "wave.cxx", "wave.h" }
  filter { "options:not with-sdl=no", "options:not with-sdl=1" }
    files {
      "SDL2Display.cxx",
      "SDL2Display.h",
      "SDL2Visual.cxx",
      "SDL2Visual.h",
      "SDL2Window.cxx",
      "SDL2Window.h"
    }
  filter "options:with-sdl=1"
    files { "SDLDisplay.cxx", "SDLDisplay.h" }

  -- windows
  filter "system:windows"
    files {
      "WinPlatformFactory.cxx",
      "WinPlatformFactory.h",
      "WinDisplay.cxx",
      "WinDisplay.h",
      "DXJoystick.cxx",
      "DXJoystick.h",
      "WinJoystick.cxx",
      "WinJoystick.h",
      "WinVisual.cxx",
      "WinVisual.h",
      "WinWindow.cxx",
      "WinWindow.h",
      "WinMedia.cxx",
      "WinMedia.h"
    }
  filter { "system:windows", "options:not with-sdl=no" }
    files { "wave.cxx", "wave.h" } -- add them back
    removefiles { -- windows uses a combination of SDL2 and native stuff
      "SDLPlatformFactory.cxx",
      "SDLPlatformFactory.h",
      "SDLMedia.cxx",
      "SDLMedia.h",
      "WinDisplay.cxx",
      "WinDisplay.h",
      "WinJoystick.cxx",
      "WinJoystick.h",
      "WinVisual.cxx",
      "WinVisual.h",
      "WinWindow.cxx",
      "WinWindow.h"
    }

  -- macOS
  filter "system:macosx"
    files "MacDataPath.cxx"
  filter { "system:macosx", "options:with-sdl=1" }
    files { "SDLMain.h", "SDLMain.m" }

  -- linux
  filter "system:linux"
    files { "EvdevJoystick.cxx", "EvdevJoystick.h" }
  filter { "system:linux", "options:with-sdl=no" }
    files {
      "LinuxPlatformFactory.cxx",
      "LinuxPlatformFactory.h",
      "LinuxDisplay.cxx",
      "LinuxDisplay.h",
      "LinuxMedia.cxx",
      "LinuxMedia.h",
      "USBJoystick.cxx",
      "USBJoystick.h",
      "XIJoystick.cxx",
      "XIJoystick.h",
      "XDisplay.cxx",
      "XDisplay.h",
      "XVisual.cxx",
      "XVisual.h",
      "XWindow.cxx",
      "XWindow.h"
    }

  -- solaris
  filter "system:solaris"
    files {
      "SolarisPlatformFactory.cxx",
      "SolarisPlatformFactory.h",
      "SolarisMedia.cxx",
      "SolarisMedia.h",
      "XDisplay.cxx",
      "XDisplay.h",
      "XVisual.cxx",
      "XVisual.h",
      "XWindow.cxx",
      "XWindow.h"
    }

  -- no support for irix in premake

  -- no support for beos in premake (is "haiku" the same?)
