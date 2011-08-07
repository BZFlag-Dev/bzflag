if BUILD_ZLIB
ZLIB_DIR = zlib
else
ZLIB_DIR =
endif

if BUILD_ARES
ARES_DIR = ares
else
ARES_DIR =
endif

if BUILD_REGEX
REGEX_DIR = regex
else
REGEX_DIR =
endif

if BUILD_CURL
CURL_DIR = curl
else
CURL_DIR =
endif

SUBDIRS = \
	${ZLIB_DIR} \
	${ARES_DIR} \
	${CURL_DIR} \
	${REGEX_DIR}

# include the files for subconfigured packages as needed left out of the dist
EXTRA_DIST = \
	PDCurses \
	glew

MAINTAINERCLEANFILES = \
	Makefile.in \
	ares/Makefile.in \
	ares/aclocal.m4 \
	ares/ares_config.h.in \
	ares/ares_config.h.in~ \
	ares/autom4te.cache \
	ares/compile \
	ares/config.guess \
	ares/config.sub \
	ares/configure \
	ares/depcomp \
	ares/install-sh \
	ares/ltmain.sh \
	ares/m4/libtool.m4 \
	ares/m4/ltoptions.m4 \
	ares/m4/ltsugar.m4 \
	ares/m4/ltversion.m4 \
	ares/m4/lt~obsolete.m4 \
	ares/missing \
	ares/mkinstalldirs \
	curl/Makefile.in \
	curl/aclocal.m4 \
	curl/autom4te.cache \
	curl/config.guess \
	curl/config.sub \
	curl/configure \
	curl/depcomp \
	curl/docs/Makefile.in \
	curl/docs/examples/Makefile.in \
	curl/docs/libcurl/Makefile.in \
	curl/include/Makefile.in \
	curl/include/curl/Makefile.in \
	curl/install-sh \
	curl/lib/Makefile.in \
	curl/lib/curl_config.h.in \
	curl/lib/curl_config.h.in~ \
	curl/ltmain.sh \
	curl/m4/libtool.m4 \
	curl/m4/ltoptions.m4 \
	curl/m4/ltsugar.m4 \
	curl/m4/ltversion.m4 \
	curl/m4/lt~obsolete.m4 \
	curl/missing \
	curl/mkinstalldirs \
	curl/src/Makefile.in 

maintainer-clean-local:
	-rm -rf \
		ares/autom4te.cache \
		curl/autom4te.cache
