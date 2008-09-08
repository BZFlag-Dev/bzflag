FTGL Version 2.0

FTGL on windows can be built a ether a dynamic link library (DLL) with export lib (lib)
or a static library (lib). All files will be built in the build directory that will be created in this
directory.

FTGL requires the Freetype2 library (version 2.3.5 r).
You will need to define the environment variable
FREETYPE to contain the full path to your freetype2 sources.

the VC8 dir contains projects for use with Visual C++ 2005 and 2008, and can build
both the dynamic and static libs.

the VC71 dir contains projects for use with Visual C++ 2003 and can only build a dynamic lib.

To use FTGL in your own projects you will need to link against ether the static lib, or the DLL export lib
All builds use the multithreaded runtimes.
Your project will also need to include freetype2 and OpenGL.

For instructions on using Freetype go to www.freetype.org
For instructions on using OpenGL go to www.opengl.org
