lib_LTLIBRARIES = RogueGenocide.la

RogueGenocide_la_SOURCES = RogueGenocide.cpp
RogueGenocide_la_CPPFLAGS = -I$(top_srcdir)/include @CONF_CPPFLAGS
RogueGenocide_la_CXXFLAGS = @CONF_CXXFLAGS@
RogueGenocide_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	README.txt \
	RogueGenocide.def \
	RogueGenocide.sln \
	RogueGenocide.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
