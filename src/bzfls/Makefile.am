bin_PROGRAMS = bzfls

MAINTAINERCLEANFILES = \
	Makefile.in

bzfls_SOURCES = bzfls.cxx

noinst_HEADERS = \
	buildDate.cxx

LDADD =					\
	../net/libNet.a			\
	../common/libCommon.a

bzfls$(EXEEXT): $(bzfls_OBJECTS) $(bzfls_DEPENDENCIES) 
	@rm -f bzfls$(EXEEXT)
	$(CXX) -o buildDate.o -c $(srcdir)/buildDate.cxx
	$(CXXLINK) $(bzfls_LDFLAGS) $(bzfls_OBJECTS) buildDate.o \
	$(bzfls_LDADD) $(LIBS)
