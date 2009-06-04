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

if BUILD_FTGL
FTGL_DIR = ftgl
else
FTGL_DIR =
endif

if BUILD_FREETYPE
FREETYPE_DIR = freetype
else
FREETYPE_DIR =
endif

if BUILD_GCRYPT
GCRYPT_DIR = gcrypt
else
GCRYPT_DIR =
endif

if BUILD_GPG_ERROR
GPG_ERROR_DIR = gpg-error
else
GPG_ERROR_DIR =
endif

if BUILD_OPENLDAP
OPENLDAP_DIR = openldap
else
OPENLDAP_DIR =
endif

SUBDIRS = \
	${ZLIB_DIR} \
	${ARES_DIR} \
	${CURL_DIR} \
	${FREETYPE_DIR} \
	${FTGL_DIR} \
	${REGEX_DIR} \
	${GCRYPT_DIR} \
	${GPG_ERROR_DIR} \
	${OPENLDAP_DIR}

DIST_SUBDIRS = \
	regex \
	zlib

# include the files for subconfigured packages as needed left out of the dist
EXTRA_DIST = \
	ares \
	curl \
	freetype \
	ftgl \
	glew

MAINTAINERCLEANFILES = \
	Makefile.in
