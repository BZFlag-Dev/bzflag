lib_LTLIBRARIES = bzfscron.la

AM_CPPFLAGS = @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

bzfscron_la_SOURCES = \
	bzfscron.cpp \
	bzfscron.h \
	croncommand.cpp \
	croncommand.h \
	CronJob.cpp \
	CronJob.h 

bzfscron_la_LDFLAGS = -module -avoid-version
bzfscron_la_CXXFLAGS= -I$(top_srcdir)/plugin_utils
bzfscron_la_LIBADD  = $(top_builddir)/plugin_utils/libplugin_utils.la

EXTRA_DIST = \
	README.txt \
	crontab \
	bzfscron.def \
	bzfscron.vc8.sln \
	bzfscron.vc8.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
