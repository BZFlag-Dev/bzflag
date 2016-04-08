BZFlag README for Linux
=======================

BZFlag is a network multiplayer 3D tank battle game.  You can play
against several other people anywhere on a TCP/IP based network,
including the Internet.  BZFlag is free and open source.  Please
read the file COPYING in the bzflag-<version> directory of the
standard doc directory (probably /usr/share/doc).

Both hardware and software rendering is supported.  Hardware
rendering is highly recommended, as software rendering is very
slow.  Hardware rendering is provided by the OpenGL drivers for
your video card.  ATI and NVIDIA are the primary chipsets for video
cards in use today.  Many Linux distributions do not ship with
video drivers that properly support hardware acceleration on
modern cards.  Both ATI and NVIDIA provide Linux drivers for many
architectures on their websites.  In general they provide greater
performance then the default drivers in many distributions.

More information about BZFlag is available in the man pages for
bzflag, bzfs, bzadmin, and bzw.  Also check:

  http://BZFlag.org/

Different Linux distributions use different naming conventions for the
packages that provide the dependencies needed to compile and run
BZFlag.  Known build requirements include:

  Debian & Ubuntu:

    apt-get install g++ libtool automake autoconf libgl1-mesa-dev \
    libglu1-mesa-dev libsdl1.2-dev libsdl-sound1.2-dev libcurl3-dev \
    libc-ares-dev zlib1g-dev libncurses-dev make

  Fedora:

    yum install SDL-devel c-ares-devel libSM-devel libcurl-devel \
    gcc-c++ libidn-devel libtool ncurses-devel zlib-devel

After any development packages are installed, execute the following
commands from the top-level BZFlag source directory:

  ./autogen.sh
  ./configure
  make

The autogen.sh step can be skipped if the source code was received
from the tarball package, but must be included if the source
came directly from a git clone.

This will build bzflag.  If you wish to install the executables to
the usr directory after the build, then execute:

  make install

as root.

Known bugs in the Linux version:

  * Screen redraw problems when using accelerated glx.  Normally
    only happens, if at all, when you change video format.  The
    workaround is to press ctrl+alt+'+' or ctrl+alt+'-' after exit
    to change the video format.  Restart the X server if that
    doesn't work.  This appears to be a problem in glx.

Known issues:

  * rpm may complain about missing libGL and/or libGLU.  just
    use --nodeps and make sure you've got Mesa installed.  Mesa
    3.1 provides these libraries while earlier versions provided
    libMesaGL and libMesaGLU.  The earlier versions (starting
    with version 2.6) will work as long as there is a symbolic
    link from libGL to libMesaGL and from libGLU to libMesaGLU.

  * If rendering is very slow on your hardware, try turning off
    some rendering options.  If you are using software rendering,
    turn off textures and shadows and lower your resolution, as
    they cause the largest load on the CPU.  If you are using
    hardware rendering, make sure you have current video drivers
    for your video card's chipset.

  * Some sound drivers don't support the SNDCTL_DSP_SETFORMAT
    ioctl.  In this case bzflag uses SNDCTL_DSP_POST to flush
    partial buffers which may not work well on some drivers.
    Use -mute to disable sound.

Having sound problems?  If you're using ALSA for sound, you may get
better performance using "hw:0" instead of "plughw:0" or "default"
for audio by specifying "hw:0" under the Options->Audio->Device menu
in the game.  This seems to be due to a bug in the SDL audio mixer.
If you're using Debian, you may also have positive results replacing
libsdl1.2debian-alsa with libsdl1.2debian-all via apt.

Submit bug reports and comments on the GitHub site
https://github.com/BZFlag-Dev/bzflag/issues
