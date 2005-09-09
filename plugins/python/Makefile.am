# change EXTRA to lib to enable this plugin
# lib_LTLIBRARIES = PyBZFlag.la
EXTRA_LTLIBRARIES = PyBZFlag.la

PyBZFlag_la_LDFLAGS = -module -avoid-version ${top_builddir}/src/bzfs/libBZFSplugin.la

# for Mac OS X, uncomment these flags:
# PyBZFlag_la_CPPFLAGS = -I/System/Library/Frameworks/Python.framework/Headers
# PyBZFlag_la_LDFLAGS += -framework Python

PyBZFlag_la_SOURCES = \
	PyBZDB.cpp \
	PyBZFlag.cpp \
	PyEvent.cpp \
	PyEvents.cpp \
	PyPlayer.cpp \
	PyTeam.cpp \
	PyWorldBox.cpp \
	python.cpp

include_HEADERS = \
	PyBZDB.h \
	PyBZFlag.h \
	PyEvent.h \
	PyEvents.h \
	PyPlayer.h \
	PyTeam.h \
	PyWorldBox.h

EXTRA_SCRIPTS = \
	BZReactor.py

EXTRA_DIST = \
	SConstruct \
	binding-thoughts \
	python.def \
	python.sln \
	python.vcproj \
	test-twisted.py \
	test.py

MAINTAINERCLEANFILES =	\
	Makefile.in
