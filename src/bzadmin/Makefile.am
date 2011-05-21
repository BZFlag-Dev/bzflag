#
# Build BZAdmin
#

bin_PROGRAMS = bzadmin

AM_CPPFLAGS = \
	-DBUILDING_BZADMIN \
	-I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/bzflag \
	-I$(top_srcdir)/src/clientbase \
	$(LIBCURL_CPPFLAGS) \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

if HAVE_CURSES
CURSES_SRC = CursesUI.h CursesUI.cpp CursesMenu.h CursesMenu.cpp
else
CURSES_SRC =
endif

bzadmin_LDFLAGS = -static
bzadmin_SOURCES = \
	BZAdminClient.h \
	BZAdminClient.cpp \
	BZAdminUI.h \
	BZAdminUI.cpp \
	colors.h \
	$(CURSES_SRC) \
	OptionParser.h \
	OptionParser.cpp \
	PlayerInfo.h \
	ServerLink.cpp \
	StdBothUI.h \
	StdBothUI.cpp \
	StdInUI.h \
	StdInUI.cpp \
	StdOutUI.h \
	StdOutUI.cpp \
	UIMap.h \
	UIMap.cpp \
	bzadmin.cpp \
	curses_wrapper.h

EXTRA_bzadmin_SOURCES = \
	CursesMenu.h \
	CursesMenu.cpp \
	CursesUI.h \
	CursesUI.cpp

ServerLink.cpp: @top_srcdir@/src/clientbase/ServerLink.cpp
	cp -pR @top_srcdir@/src/clientbase/ServerLink.cpp .

LDADD = \
	../date/libDate.la	\
	../game/libGame.la	\
	../net/libNet.la	\
	../common/libCommon.la	\
	$(CURSES_LIB)		\
	$(LIBCURL)		\
	$(LIBCARES)		\
	$(LIBREGEX)

MAINTAINERCLEANFILES = \
	ServerLink.cpp \
	Makefile.in
