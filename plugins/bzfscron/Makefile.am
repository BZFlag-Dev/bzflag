lib_LTLIBRARIES = bzfscron.la

bzfscron_la_SOURCES = \
	bzfscron.cpp \
	bzfscron.h \
	croncommand.cpp \
	croncommand.h \
	CronJob.cpp \
	CronJob.h \
	TextUtils.cxx \
	TextUtils.h

bzfscron_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = \
	README.txt \
	crontab \
	bzfscron.def \
	bzfscron.vc8.sln \
	bzfscron.vc8.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
