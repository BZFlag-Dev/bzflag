noinst_LTLIBRARIES = libfreetype.la

AM_CPPFLAGS = \
	-I${top_srcdir}/src/other/freetype/include \
	-I${top_srcdir}/src/other/freetype/include/freetype \
	-I${top_srcdir}/src/other/freetype/include/freetype/config \
	-DFT_CONFIG_OPTION_SYSTEM_ZLIB \
	-DDARWIN_NO_CARBON \
	-DFT_CONFIG_CONFIG_H="<ftconfig.h>" \
	-DFT2_BUILD_LIBRARY \
	-DFT_CONFIG_MODULES_H="<ftmodule.h>"

libfreetype_la_LDFLAGS = -static
libfreetype_la_SOURCES = \
	src/autofit/autofit.c \
	src/bdf/bdf.c \
	src/cff/cff.c \
	src/base/ftbase.c \
	src/base/ftbbox.c \
	src/base/ftbdf.c \
	src/base/ftbitmap.c \
	src/cache/ftcache.c \
	src/base/ftdebug.c \
	src/base/ftgasp.c \
	src/base/ftglyph.c \
	src/base/ftgxval.c \
	src/gzip/ftgzip.c \
	src/base/ftinit.c \
	src/base/ftlcdfil.c \
	src/lzw/ftlzw.c \
	src/base/ftmm.c \
	src/base/ftotval.c \
	src/base/ftpfr.c \
	src/base/ftstroke.c \
	src/base/ftsynth.c \
	src/base/ftsystem.c \
	src/base/fttype1.c \
	src/base/ftwinfnt.c \
	src/base/ftxf86.c \
	src/pcf/pcf.c \
	src/pfr/pfr.c \
	src/psaux/psaux.c \
	src/pshinter/pshinter.c \
	src/psnames/psnames.c \
	src/raster/raster.c \
	src/sfnt/sfnt.c \
	src/smooth/smooth.c \
	src/truetype/truetype.c \
	src/type1/type1.c \
	src/cid/type1cid.c \
	src/type42/type42.c \
	src/winfonts/winfnt.c

MAINTAINERCLEANFILES = \
	Makefile.in

EXTRA_DIST = \
	ChangeLog \
	ChangeLog.20 \
	ChangeLog.21 \
	ChangeLog.22 \
	Jamfile \
	Jamrules \
	README \
	README.CVS \
	autogen.sh \
	builds \
	devel \
	docs \
	include \
	modules.cfg \
	src \
	version.sed \
	vms_make.com
