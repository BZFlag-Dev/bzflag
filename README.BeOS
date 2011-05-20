README for BeOS platforms
-------------------------

(François Revol - 26/02/2004)

BZFlag should now compile on BeOS R5 BONE, using the software
renderer. It also works fine with MESA 6. It can also built for
the "new" OpenGL kit that was to be in R5.1d0, the configure
detects libGL2, though one should then have to replace
src/platform/BeOSWindow.cxx by src/platform/BeOSWindow2.cxx.
Note there are many issues with this beta GL stuff, and it's
totally unsupported. I keep this one in SVN because I don't
know which API will survive the other.

net_server isn't supported yet, and I don't think it will ever
be, though the configure script only links to BONE libs if they
are present.

I usually use:
configure --prefix=/boot/home/config --enable-debug

Current limitations include:
- windowed mode only, and fixed size window.
- buggy audio (samples get played in a loop, giving a headache
after some minutes.

SDL build (= without the native ui) under BeOS hasn't been
investigated yet.
