lib_LTLIBRARIES = RogueGenocide.la

RogueGenocide_la_SOURCES = RogueGenocide.cpp
RogueGenocide_la_CPPFLAGS = -I../../include
RogueGenocide_la_LDFLAGS = -module -avoid-version ${top_builddir}/src/bzfs/libBZFSplugin.la

EXTRA_DIST = \
	ReadMe.txt \
	RogueGenocide.def \
	RogueGenocide.sln \
	RogueGenocide.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
