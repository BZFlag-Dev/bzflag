SUBDIRS = TextTool2.xcodeproj

bin_PROGRAMS = TextTool2

TextTool2_SOURCES = \
	TextTool2.cpp
TextTool2_CPPFLAGS = -I/usr/include/freetype2
TextTool2_LDFLAGS = -lfreetype -lpng

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.txt

MAINTAINERCLEANFILES = \
	Makefile.in
