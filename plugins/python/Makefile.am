EXTRA_LTLIBRARIES = libPyBZFlag.la

libPyBZFlag_la_LDFLAGS = -rpath ${libdir}

libPyBZFlag_la_SOURCES = \
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
