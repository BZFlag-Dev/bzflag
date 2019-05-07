# BZFlag README for macOS

BZFlag is supported on macOS version 10.7 and later. Official builds may be
downloaded from the project web site at https://www.bzflag.org/downloads.
Alternatively, you can set up your build environment and project dependencies,
and then build BZFlag from source.

## Using the BZFlag Application on macOS

To run the BZFlag client, double-click the BZFlag application in the Finder.
To access the bzadmin text client or the bzflag server bzfs from the BZFlag
application, right click on the application, click "Show Package Contents,"
and navigate to Contents/MacOS. Server plugins are located at
Contents/PlugIns. Note that plugin names are different from Windows and Linux
("<name>.dylib" instead of "<name>.dll or "<name>.so"). However, bzfs will
still load the plugins when you specify the full plugin path.

## Building BZFlag from Source

BZFlag builds on macOS are now done using premake5 and Xcode 7 (or later).
Please refer to the [README.premake5.md](README.premake5.md) file for build instructions.

Please note that at least through SDL 2.0.8, there are issues building BZFlag
on recent versions of macOS and Xcode. We have success building on Mac OS X
10.10 with Xcode 7. Here are known issues with SDL 2 on macOS that are
affecting us:

  https://bugzilla.libsdl.org/show_bug.cgi?id=4272
  https://bugzilla.libsdl.org/show_bug.cgi?id=4177