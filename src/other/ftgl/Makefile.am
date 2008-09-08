
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src test demo docs
DIST_SUBDIRS = $(SUBDIRS) msvc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ftgl.pc

DISTCLEANFILES = ftgl.pc

EXTRA_DIST = \
	AUTHORS \
	BUGS \
	COPYING \
	ChangeLog \
	INSTALL \
	NEWS \
	README \
	TODO \
	autogen.sh \
	configure.ac \
	ftgl.pc.in \
	m4 \
	$(NULL)

# Print out an informative summary.
all-local:
	@$(ECHO) "Done."
	@$(ECHO)
	@if test "x$(MAKECMDGOALS)" = "xall-am" -o "x$(.TARGETS)" = "xall-am" -o "x$(MAKECMDGOALS)" = "x" -o "x$(.TARGETS)" = "x" ; then \
	  $(ECHO) "---" ;\
	  $(ECHO) "Run 'make install' to begin installation into $(prefix)" ;\
	fi
	@$(ECHO)

# Upload documentation
DOC = docs/html docs/latex/ftgl.pdf
HOST = ftgl.sf.net
DIR = /home/groups/f/ft/ftgl/htdocs/
upload-doc:
	tar cz $(DOC) | ssh $(HOST) "cd $(DIR) && rm -Rf $(DOC) && tar xvz"

NULL =
