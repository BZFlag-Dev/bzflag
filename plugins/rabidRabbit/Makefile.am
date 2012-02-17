lib_LTLIBRARIES = rabidRabbit.la

rabidRabbit_la_SOURCES = rabidRabbit.cpp
rabidRabbit_la_CPPFLAGS = -I$(top_srcdir)/include
rabidRabbit_la_LDFLAGS = -module -avoid-version -shared

EXTRA_DIST = \
	README.txt \
	rabidRabbit.def \
	rabidRabbit.sln \
	rabidRabbit.txt \
	rabidRabbit.vcproj

MAINTAINERCLEANFILES =	\
	Makefile.in
