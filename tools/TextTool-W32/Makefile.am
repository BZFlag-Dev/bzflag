SUBDIRS = res

bin_PROGRAMS = TextTool

AM_CPPFLAGS = @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

TextTool_SOURCES = \
	MainFrm.cpp \
	MainFrm.h \
	StdAfx.cpp \
	StdAfx.h \
	TextTool.cpp \
	TextTool.h \
	TextToolBatch.cpp \
	TextToolBatch.h \
	TextToolDoc.cpp \
	TextToolDoc.h \
	TextToolView.cpp
	TextToolView.h \
	resource.h

EXTRA_DIST = \
	bzflag.ttb \
	README.txt \
	TextTool.clw \
	TextTool.dsp \
	TextTool.dsw \
	TextTool.rc \
	TextTool.sln \
	TextTool.vcproj

MAINTAINERCLEANFILES = \
	Makefile.in
