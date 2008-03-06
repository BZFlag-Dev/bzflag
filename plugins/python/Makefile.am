# change EXTRA to lib to enable this plugin
# lib_LTLIBRARIES = PyBZFlag.la
EXTRA_LTLIBRARIES = PyBZFlag.la

PyBZFlag_la_LDFLAGS = -module -avoid-version

# for Mac OS X, uncomment these flags:
# PyBZFlag_la_CPPFLAGS = -I/System/Library/Frameworks/Python.framework/Headers
# PyBZFlag_la_LDFLAGS += -framework Python

# PyBZFlag_la_CPPFLAGS = @CONF_CPPFLAGS@
# PyBZFlag_la_CXXFLAGS = @CONF_CXXFLAGS@

PyBZFlag_la_SOURCES = \
	PyBZDB.cpp \
	PyBZDB.h \
	PyBZFlag.cpp \
	PyBZFlag.h \
	PyEvent.cpp \
	PyEvent.h \
	PyEvents.cpp \
	PyEvents.h \
	PyPlayer.cpp \
	PyPlayer.h \
	PyTeam.cpp \
	PyTeam.h \
	python.cpp \
	PyWorldBox.cpp \
	PyWorldBox.h

EXTRA_SCRIPTS = \
	BZReactor.py

EXTRA_DIST = \
	$(EXTRA_SCRIPTS) \
	SConstruct \
	binding-thoughts \
	python.def \
	python.sln \
	python.vcproj \
	test-twisted.py \
	test.py

MAINTAINERCLEANFILES =	\
	Makefile.in
