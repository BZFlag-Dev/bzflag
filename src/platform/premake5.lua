project "platform"
  kind "StaticLib"

  -- generic files
  files {
    "BzfDisplay.cxx",
    "BzfJoystick.cxx",
    "BzfVisual.cxx",
    "BzfMedia.cxx",
    "BzfWindow.cxx",
    "PlatformFactory.cxx",
    "wave.cxx",
    "wave.h"
  }
  filter { "system:not windows", "options:not with-sdl=no" }
    removefiles { "wave.cxx", "wave.h" }

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

  -- macOS
  filter "system:macosx"
    files { "MacDataPath.cxx", "MacGL.h", "MacGL.mm" }
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

  -- no support for beos in premake (although there is "haiku"... is that the
  -- same?)
