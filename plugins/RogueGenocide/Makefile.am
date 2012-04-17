lib_LTLIBRARIES = RogueGenocide.la

RogueGenocide_la_SOURCES = RogueGenocide.cpp
RogueGenocide_la_CPPFLAGS = -I$(top_srcdir)/include
RogueGenocide_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.txt \
	RogueGenocide.def \
	RogueGenocide.sln \
	RogueGenocide.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
