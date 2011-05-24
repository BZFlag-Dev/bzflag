--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local common_files = {
  'BzfDisplay.cpp',
  'BzfJoystick.cpp',
  'BzfMedia.cpp',
  'BzfVisual.cpp',
  'BzfWindow.cpp',
  'Clipboard.cpp',
  'PlatformFactory.cpp',
  'VerticalSync.cpp',
  'wave.cpp', 'wave.h',
}


local sdl_files = {
  'SDLDisplay.cpp',  'SDLDisplay.h',
  'SDLJoystick.cpp', 'SDLJoystick.h',
  'SDLMedia.cpp',    'SDLMedia.h',
  'SDLVisual.cpp',   'SDLVisual.h',
  'SDLWindow.cpp',   'SDLWindow.h',
}


local x11_files = {
  'XDisplay.cpp', 'XDisplay.h',
  'XVisual.cpp',  'XVisual.h',
  'XWindow.cpp',  'XWindow.h',
}

local linux_sdl_files = table.join(sdl_files, {
  'SDLPlatformFactory.cpp', 'SDLPlatformFactory.h',
  'EvdevJoystick.cpp',      'EvdevJoystick.h',
})

local linux_native_files = table.join(x11_files, {
  'EvdevJoystick.cpp',
  'LinuxDisplay.cpp',
  'LinuxMedia.cpp',
  'LinuxPlatformFactory.cpp',
  'USBJoystick.cpp',
})

local solaris_files = table.join(x11_files, {
  'SolarisMedia.cpp',
  'SolarisPlatformFactory.cpp',
})

local irix_files = table.join(x11_files, {
  'SGIDisplay.cpp',
  'SGIMedia.cpp',
  'SGIPlatformFactory.cpp',
})

local macosx_files = table.join(sdl_files, {
  'SDLPlatformFactory.cpp',
})

local beos_files = {
  'BeOSDisplay.cpp',
  'BeOSMedia.cpp',
  'BeOSPlatformFactory.cpp',
  'BeOSVisual.cpp',
  'BeOSWindow.cpp',
}

local win32_files = table.join(sdl_files, {
  'DXJoystick.cpp',
  'WinDisplay.cpp',
  'WinJoystick.cpp',
  'WinMedia.cpp',
  'WinPlatformFactory.cpp',
  'WinVisual.cpp',
  'WinWindow.cpp',
})

local hpux_files = {
  'HPUXMedia.cpp',
  'HPUXPlatformFactory.cpp',
}

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


project   'libPlatform'
  targetname 'Platform'
  kind  'StaticLib'
  objdir '.obj'
  files(common_files)


if (os.is('windows')) then
  files(win32_files)
else
  files(linux_sdl_files)
--  include 'MacOSX'
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

